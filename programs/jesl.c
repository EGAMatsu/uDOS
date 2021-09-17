/*
 * Small JCL interpreter
 */

#include <alloc.h>
#include <panic.h>
#include <string.h>
#include <user.h>

static const char *identifier_accept = "QWERTYUIOPASDFGHJKLZXCVBNM1234567890-_";
static const char *param_key_accept =
    "&QWERTYUIOPASDFGHJKLZXCVBNM1234567890-_* ";
static const char *multi_param_accept =
    "&QWERTYUIOPASDFGHJKLZXCVBNM1234567890-_,(* ";

struct jcl_param {
    char *key;
    char *value;
};

struct jcl_variable {
    char *key;
    char *value;
};

struct jcl_procedure {
    char *name;
    char *body;
    struct jcl_param *params;
    size_t n_params;
};

struct jcl_job {
    user_t account;
    char *owner;
    char class;
    int priority;
    user_t notify;
    char msg_class;
    char stat_type;
    char msg_type;
    char exec_type;
    int t_minutes;
    int t_seconds;
    unsigned long long region_size;
    size_t log_size;
    user_t submit_user;
    char *hash_password;
};

enum {
    JCL_DUMMY,
    JCL_INSTREAM,
    JCL_SYSOUT,
    JCL_VOLUME,
};
struct jcl_dataset {
    int type;
    char *value;
};

struct jcl_parse_info {
    const char *start;
    const char *data;
    size_t c_line;

    struct jcl_variable *variables;
    size_t n_variables;

    struct jcl_procedure *procedures;
    size_t n_procedures;

    struct jcl_dataset *datasets;
    size_t n_datasets;

    struct jcl_procedure *curr_proc_ctx;
    struct jcl_job *job;
};

static struct jcl_variable *jcl_create_variable(
    struct jcl_parse_info *info, const char *key, const char *value) {
    info->variables = krealloc_array(
        info->variables, info->n_variables + 1, sizeof(struct jcl_variable));
    if (info->variables == NULL) {
        kpanic("Out of memory");
    }
    info->variables[info->n_variables].key = kmalloc(strlen(key) + 1);
    strcpy(info->variables[info->n_variables].key, key);
    info->variables[info->n_variables].value = kmalloc(strlen(value) + 1);
    strcpy(info->variables[info->n_variables].value, value);
    ++info->n_variables;
    return &info->variables[info->n_variables - 1];
}

static struct jcl_dataset *jcl_create_dataset(
    struct jcl_parse_info *info, const char *value) {
    struct jcl_dataset *dataset;
    info->datasets = krealloc_array(
        info->datasets, info->n_datasets + 1, sizeof(struct jcl_dataset));
    if (info->datasets == NULL) {
        kpanic("Out of memory");
    }

    dataset = &info->datasets[info->n_datasets];
    if (value != NULL) {
        dataset->value = kmalloc(strlen(value) + 1);
        if (dataset->value == NULL) {
            kpanic("Out of memory");
        }
        strcpy(dataset->value, value);
    } else {
        dataset->value = NULL;
    }

    ++info->n_datasets;
    return &info->datasets[info->n_datasets - 1];
}

static const char *jcl_resolve_variable_value(
    struct jcl_parse_info *info, const char *var_name) {
    size_t i;
    if (*var_name == '&') {
        ++var_name;
    }

    for (i = 0; i < info->n_variables; i++) {
        if (strcmp(info->variables[i].key, var_name)) {
            continue;
        }
        return info->variables[i].value;
    }
    return NULL;
}

static struct jcl_procedure *jcl_create_procedure(
    struct jcl_parse_info *info, const char *name) {
    info->procedures = krealloc_array(
        info->procedures, info->n_procedures + 1, sizeof(struct jcl_procedure));
    if (info->procedures == NULL) {
        kpanic("Out of memory");
    }
    info->procedures[info->n_procedures].name = kmalloc(strlen(name) + 1);
    strcpy(info->procedures[info->n_procedures].name, name);
    ++info->n_procedures;
    return &info->procedures[info->n_procedures - 1];
}

static void jcl_add_param_to_procedure(struct jcl_parse_info *info,
    struct jcl_procedure *proc, struct jcl_param param) {
    proc->params = krealloc_array(
        proc->params, proc->n_params + 1, sizeof(struct jcl_param));
    if (proc->params == NULL) {
        kpanic("Out of memory");
    }
    proc->params[proc->n_params].key = kmalloc(strlen(param.key) + 1);
    strcpy(proc->params[proc->n_params].key, param.key);
    proc->params[proc->n_params].value = kmalloc(strlen(param.value) + 1);
    strcpy(proc->params[proc->n_params].value, param.value);
    ++proc->n_params;
    return;
}

static struct jcl_job *jcl_new_job(
    struct jcl_parse_info *info, user_t account, const char *owner) {
    struct jcl_job *job;
    job = kzalloc(sizeof(struct jcl_job));
    if (job == NULL) {
        kpanic("Out of memory");
    }
    job->account = account;
    job->owner   = kmalloc(strlen(owner) + 1);
    strcpy(job->owner, owner);
    return job;
}

static void jcl_error(struct jcl_parse_info *info, const char *fmt, ...) {
    char tmpbuf[80], *tmpbuf_ptr = (char *)&tmpbuf;
    const char *data = info->data;
    size_t c_row     = 0, i;
    va_list args;

    while (*data != '\n' && data != info->start) {
        ++c_row;
        --data;
    }
    ++data;

    while (*data != '\n' && *data != '\0') {
        *(tmpbuf_ptr++) = *(data++);
    }
    *(tmpbuf_ptr++) = '\0';

    va_start(args, fmt);
    kprintf("\x1b[31;1m[JCL Error]\x1b[0m %zu:%zu: ", info->c_line, c_row);
    kvprintf(fmt, args);

    /* Display the faulting line */
    kprintf("\n%s\n", &tmpbuf);

    /* Provide visual aid for tracking error */
    tmpbuf_ptr = (char *)&tmpbuf;
    for (i = 0; i < c_row; ++i) {
        *(tmpbuf_ptr++) = ' ';
    }
    *(tmpbuf_ptr++) = '^';
    *(tmpbuf_ptr++) = '\0';
    kprintf("%s\n", &tmpbuf);

    va_end(args);
    return;
}

static struct jcl_param jcl_get_positional(
    struct jcl_parse_info *info, const char *key, const char *fall_value) {
    struct jcl_param param;
    char is_multi = 0;
    size_t len;

    info->data += strspn(info->data, " ");

    len       = strlen(key);
    param.key = kmalloc(len + 1);
    if (param.key == NULL) {
        kpanic("Out of memory");
    }
    strcpy(param.key, key);

    /* Obtain the value of the parameter */
    if (*info->data == ',') {
        /* When the stream is just a comma we apply a default */
        len         = strlen(fall_value);
        param.value = kmalloc(len + 1);
        if (param.value == NULL) {
            kpanic("Out of memory");
        }
        strcpy(param.value, fall_value);
        ++info->data;
    } else {
        if (*info->data == '(') {
            is_multi = 1;
        }

        if (*info->data == '\'' || *info->data == '"') {
            ++info->data;
        }

        if (is_multi == 0) {
            len = strspn(info->data, param_key_accept);
        } else {
            len = strspn(info->data, multi_param_accept);
            if (info->data[len] != ')') {
                jcl_error(info, "Expected closing )");
                return param;
            }
            len++;
        }
        if (!len) {
            jcl_error(info, "Parameter %s has no value", param.key);
        }
        param.value = kmalloc(len + 1);
        if (param.value == NULL) {
            kpanic("Out of memory");
        }
        strncpy(param.value, info->data, len);
        info->data += len;

        if (*info->data == '\'' || *info->data == '"') {
            ++info->data;
        }
        if (*info->data == ',') {
            ++info->data;
        }
    }
    return param;
}

static void jcl_next_line(struct jcl_parse_info *info) {
    while (*info->data != '\0') {
        if (!strncmp(info->data, "//", 2)) {
            info->data += 2;

            /* If it is a comment, then skip it and repeat loop for next valid
             * line */
            if (*info->data == '*') {
            skip_comment:
                while (*info->data != '\n') {
                    ++info->data;
                }
                continue;
            }
            return;
        } else if (!strncmp(info->data, "/*", 2)) {
            goto skip_comment;
        }
        ++info->data;
    }
    return;
}

static struct jcl_param jcl_get_optional(struct jcl_parse_info *info) {
    struct jcl_param param;
    size_t len;

    if (*info->data == ',') {
        ++info->data;
    }
    info->data += strspn(info->data, " ");

    len = strspn(info->data, param_key_accept);
    if (!len) {
        param.key   = NULL;
        param.value = NULL;
        return param;
    }
    param.key = kmalloc(len + 1);
    strncpy(param.key, info->data, len);
    info->data += len;

    if (*info->data != '=') {
        jcl_error(info, "Expected = after %s", param.key);
        return param;
    }
    ++info->data;

    len = strspn(info->data, param_key_accept);
    if (!len) {
        jcl_error(info, "Zero-length value on param %s", param.key);
        return param;
    }
    param.value = kmalloc(len + 1);
    strncpy(param.value, info->data, len);

    /* Perform a substitution when a reference to a variable is encountered */
    if (*info->data == '&') {
        const char *value;
        value = jcl_resolve_variable_value(info, param.value);
        if (value == NULL) {
            jcl_error(info, "Variable %s not found", param.value);
        }
        kfree(param.value);
        param.value = kmalloc(strlen(value) + 1);
        strcpy(param.value, value);
    }
    info->data += len;

    /* This code will skip until the next parameter - if there is a comma but
     * then
     * a line break occurs this code will position the reading cursor to prepare
     * it for the next reading of another optional parameter
     */
    if (*info->data == ',') {
        /* Skip the comma */
        ++info->data;

        /* Skip whitespace */
        info->data += strspn(info->data, " \t");

        /* If there is a newline it means we can have more parameters
         * due to a line break */
        if (*info->data == '\n') {
            ++info->data;
            ++info->c_line;

            /* Skip comments that may appear */
            jcl_next_line(info);
        }
    }
    return param;
}

static void jcl_destroy_param(struct jcl_param param) {
    kfree(param.key);
    kfree(param.value);
    return;
}

static void jcl_parse(struct jcl_parse_info *info) {
    char *ident, *statement;
    size_t len;
    user_t uid;

    jcl_create_variable(
        info, "SYSUID", user_from_uid(user_get_current())->name);

    ident = kmalloc(40 + 1);
    if (ident == NULL) {
        jcl_error(info, "Out of memory");
        goto end;
    }

    statement = kmalloc(40 + 1);
    if (statement == NULL) {
        jcl_error(info, "Out of memory");
        goto end;
    }

    while (*info->data != '\0') {
        jcl_next_line(info);
        if (*info->data == '\0') {
            break;
        }

        /* Obtain the identifier */
        info->data += strspn(info->data, " \t");
        len = strspn(info->data, identifier_accept);
        if (len >= 40) {
            jcl_error(info, "Identifier is too lengthy");
            goto end;
        }
        memcpy(ident, info->data, len);
        ident[len] = '\0';
        info->data += len;

        /* Obtain the statement */
        info->data += strspn(info->data, " \t");
        len = strspn(info->data, identifier_accept);
        if (len >= 40) {
            jcl_error(info, "Statement is too lengthy");
            goto end;
        }
        memcpy(statement, info->data, len);
        statement[len] = '\0';
        info->data += len;

        if (!strcmp(statement, "JOB")) {
            struct jcl_param param;
            user_t account;

            param = jcl_get_positional(info, "ACCNT", "(*)");
            if (!strcmp(param.value, "*")) {
                account = user_get_current();
            } else {
                /* Specified an ID */
                if (*param.value >= '0' && *param.value <= '9') {
                    account = user_get_by_name(
                        user_from_uid(*param.value - '0')->name);
                } else {
                    account = user_get_by_name(param.value);
                }
            }
            jcl_destroy_param(param);

            param = jcl_get_positional(
                info, "PRGID", user_from_uid(user_get_current())->name);
            info->job = jcl_new_job(info, account, param.value);
            jcl_destroy_param(param);

            param = jcl_get_optional(info);
            while (param.key != NULL) {
                if (!strcmp(param.key, "CLASS")) {
                    if (*param.key >= '0' && *param.key <= '9') {
                        info->job->class = *param.key - '0';
                    } else if (*param.key >= 'A' && *param.key <= 'Z') {
                        info->job->class = *param.key - 'A';
                    } else {
                        jcl_error(info, "Unknown class %c", *param.key);
                        goto end;
                    }
                } else if (!strcmp(param.key, "PRTY")) {
                    info->job->priority = *param.key - '0';
                    if (info->job->priority >= 16) {
                        jcl_error(info, "Priority is higher than 16 (%u)",
                            (unsigned)info->job->priority);
                        goto end;
                    }
                } else if (!strcmp(param.key, "NOTIFY")) {
                } else if (!strcmp(param.key, "MSGCLASS")) {
                    if (*param.key >= '0' && *param.key <= '9') {
                        info->job->msg_class = *param.key - '0';
                    } else if (*param.key >= 'A' && *param.key <= 'Z') {
                        info->job->msg_class = *param.key - 'A';
                    } else {
                        jcl_error(info, "Unknown message class %c", *param.key);
                        goto end;
                    }
                } else if (!strcmp(param.key, "REGION")) {
                    const char *unit_spec;
                    info->job->region_size = (long unsigned int)atoi(param.key);
                    unit_spec = param.key + strspn(param.key, "0123456789");
                    switch (*unit_spec) {
                    case 'B': /* Non-standard */
                        info->job->region_size *= 1;
                        break;
                    case 'K':
                        info->job->region_size *= 1000;
                        break;
                    case 'M':
                        info->job->region_size *= 1000000;
                        break;
                    case 'G': /* Non-standard */
                        info->job->region_size *= 1000000000;
                        break;
                    case 'T': /* Non-standard */
                        info->job->region_size *= 1000000000000000;
                        break;
                    case 'P':
                    case 'Y':
                    case 'Z':
                    case 'E':
                        jcl_error(info, "Too large units to handle");
                        goto end;
                    default:
                        break;
                    }

                    /* 0K/M tells us that we are free to allocate the largest
                     * address space for this job - we are going to use 16M */
                    if (info->job->region_size == 0) {
                        info->job->region_size = 1000000 * 16;
                    }
                } else if (!strcmp(param.key, "BYTES")) {
                    info->job->log_size = (long unsigned int)atoi(param.key);
                } else if (!strcmp(param.key, "USER")) {
                    info->job->submit_user = user_get_by_name(param.key);
                }

                kprintf("%s=%s\n", param.key, param.value);
                jcl_destroy_param(param);
                param = jcl_get_optional(info);
            }
        } else if (!strcmp(statement, "PROC")) {
            struct jcl_param param;
            struct jcl_procedure *proc;
            const char *body_end;

            if (info->curr_proc_ctx != NULL) {
                jcl_error(info, "Unimplemented nested procedures");
                goto end;
            }

            proc = jcl_create_procedure(info, ident);

            /* Obtain "default" parameters - procedures can have any number
             * of optional parameters */
            param = jcl_get_optional(info);
            while (param.key != NULL) {
                jcl_add_param_to_procedure(info, proc, param);

                jcl_destroy_param(param);
                param = jcl_get_optional(info);
            }
            info->curr_proc_ctx = proc;
            jcl_next_line(info);

            body_end = strstr(info->data, "PEND");
            if (body_end == NULL) {
                jcl_error(info, "Procedure without end");
                goto end;
            }

            /* Copy body of procedure */
            len        = (size_t)((ptrdiff_t)body_end - (ptrdiff_t)info->data);
            proc->body = kmalloc(len + 1);
            if (proc->body == NULL) {
                jcl_error(info, "Out of memory");
                goto end;
            }
            memcpy(proc->body, info->data, len);
            info->data += len;
        } else if (!strcmp(statement, "DD")) {
            struct jcl_dataset *dataset;

            info->data += strspn(info->data, " ");
            if (*info->data == '*') {
                const char *data_start;
                char *tmpbuf;
                ++info->data;

                /* Go to the next line where the data is located on-stream */
                info->data = strchr(info->data, '\n');
                if (info->data == NULL) {
                    jcl_error(info, "Expected newline");
                    goto end;
                }
                ++info->data;

                data_start = info->data;
                info->data = strstr(info->data, "\n/*");
                if (info->data == NULL) {
                    jcl_error(info, "No end for in-stream dataset");
                    goto end;
                }

                len    = (ptrdiff_t)info->data - (ptrdiff_t)data_start;
                tmpbuf = kmalloc(len + 1);
                if (tmpbuf == NULL) {
                    kpanic("Out of memory");
                }
                strncpy(tmpbuf, data_start, len);
                dataset       = jcl_create_dataset(info, tmpbuf);
                dataset->type = JCL_INSTREAM;
                kfree(tmpbuf);

                kprintf("%s: %s\n", info->job->owner, dataset->value);

                /* Skip the newline and the stream end marker */
                info->data += 3;
                if (*info->data != '\n') {
                    jcl_error(
                        info, "In-stream should end with newlines - not trash");
                    goto end;
                }
            } else {
                /* Get the key */
                struct jcl_param param;

                param = jcl_get_optional(info);
                kprintf(
                    "%s: %s=%s\n", info->job->owner, param.key, param.value);

                if (!strcmp(param.key, "SYSOUT")) {
                    dataset       = jcl_create_dataset(info, param.value);
                    dataset->type = JCL_INSTREAM;
                } else if (!strcmp(param.key, "DUMMY")) {
                    dataset       = jcl_create_dataset(info, NULL);
                    dataset->type = JCL_DUMMY;
                } else {
                    jcl_error(info, "Unknown stream type for dataset");
                    goto end;
                }
                jcl_destroy_param(param);
            }
        } else {
            jcl_error(info, "Unknown statment %s", statement);
        }
    }

end:
    kfree(statement);
    kfree(ident);
    return;
}

void jcl_dump_procedures(struct jcl_parse_info *info) {
    size_t i, j;
    for (i = 0; i < info->n_procedures; i++) {
        struct jcl_procedure *proc = &info->procedures[i];

        kprintf("PROCEDURE %s\n[%s\n]\n", proc->name, proc->body);
        for (j = 0; j < proc->n_params; j++) {
            kprintf("\t%s=%s\n", proc->params[i].key, proc->params[i].value);
        }
    }
    return;
}

static const char *test =
    "//HELLO    JOB (T043JM,JM00,1,0,0,0),'HELLO WORLD - JRM',CLASS=R,\n"
    "//             MSGCLASS=X,MSGLEVEL=1,NOTIFY=S0JM\n"
    "//*\n"
    "//* PRINT \"HELLO WORLD\" ON JOB OUTPUT\n"
    "//*\n"
    "//* NOTE THAT THE EXCLAMATION POINT IS INVALID EBCDIC FOR JCL\n"
    "//*   AND WILL CAUSE A JCL ERROR\n"
    "//*\n"
    "//STEP0001 EXEC PGM=IEBGENER\n"
    "//SYSIN    DD DUMMY\n"
    "//SYSPRINT DD SYSOUT=*\n"
    "//SYSUT1   DD *\n"
    "HELLO, WORLD\n"
    "/*\n"
    "//SYSUT2   DD SYSOUT=*\n"
    "//\n";

int main(int argc, char **argv) {
    struct jcl_parse_info info;

    info.start         = test;
    info.data          = info.start;
    info.variables     = NULL;
    info.n_variables   = 0;
    info.procedures    = NULL;
    info.n_procedures  = 0;
    info.datasets      = NULL;
    info.n_datasets    = 0;
    info.curr_proc_ctx = NULL;
    info.c_line        = 0;
    info.job           = NULL;
    jcl_parse(&info);
    return 0;
}