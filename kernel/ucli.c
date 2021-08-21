/* uDOS console command line
 *
 * Supports a small JCL runtime to help execute jobs
 */

#include <malloc.h>
#include <panic.h>
#include <user.h>
#include <string.h>

static const char *identifier_accept = "QWERTYUIOPASDFGHJKLZXCVBNM1234567890-_";
static const char *param_key_accept = "&QWERTYUIOPASDFGHJKLZXCVBNM1234567890-_* ";
static const char *multi_param_accept = "&QWERTYUIOPASDFGHJKLZXCVBNM1234567890-_,(* ";

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

static struct jcl_variable *jcl_create_variable(struct jcl_parse_info *info, const char *key, const char *value) {
    info->variables = krealloc_array(info->variables, info->n_variables + 1, sizeof(struct jcl_variable));
    if(info->variables == NULL) {
        kpanic("Out of memory");
    }
    info->variables[info->n_variables].key = kmalloc(strlen(key) + 1);
    strcpy(info->variables[info->n_variables].key, key);
    info->variables[info->n_variables].value = kmalloc(strlen(value) + 1);
    strcpy(info->variables[info->n_variables].value, value);
    ++info->n_variables;
    return &info->variables[info->n_variables - 1];
}

static struct jcl_dataset *jcl_create_dataset(struct jcl_parse_info *info, const char *value) {
    struct jcl_dataset *dataset;
    info->datasets = krealloc_array(info->datasets, info->n_datasets + 1, sizeof(struct jcl_dataset));
    if(info->datasets == NULL) {
        kpanic("Out of memory");
    }

    dataset = &info->datasets[info->n_datasets];
    if(value != NULL) {
        dataset->value = kmalloc(strlen(value) + 1);
        if(dataset->value == NULL) {
            kpanic("Out of memory");
        }
        strcpy(dataset->value, value);
    } else {
        dataset->value = NULL;
    }

    ++info->n_datasets;
    return &info->datasets[info->n_datasets - 1];
}

static const char *jcl_resolve_variable_value(struct jcl_parse_info *info, const char *var_name) {
    size_t i;
    if(*var_name == '&') {
        ++var_name;
    }

    for(i = 0; i < info->n_variables; i++) {
        if(strcmp(info->variables[i].key, var_name)) {
            continue;
        }
        return info->variables[i].value;
    }
    return NULL;
}

static struct jcl_procedure *jcl_create_procedure(struct jcl_parse_info *info, const char *name) {
    info->procedures = krealloc_array(info->procedures, info->n_procedures + 1, sizeof(struct jcl_procedure));
    if(info->procedures == NULL) {
        kpanic("Out of memory");
    }
    info->procedures[info->n_procedures].name = kmalloc(strlen(name) + 1);
    strcpy(info->procedures[info->n_procedures].name, name);
    ++info->n_procedures;
    return &info->procedures[info->n_procedures - 1];
}

static void jcl_add_param_to_procedure(struct jcl_parse_info *info, struct jcl_procedure *proc, struct jcl_param param) {
    proc->params = krealloc_array(proc->params, proc->n_params + 1, sizeof(struct jcl_param));
    if(proc->params == NULL) {
        kpanic("Out of memory");
    }
    proc->params[proc->n_params].key = kmalloc(strlen(param.key) + 1);
    strcpy(proc->params[proc->n_params].key, param.key);
    proc->params[proc->n_params].value = kmalloc(strlen(param.value) + 1);
    strcpy(proc->params[proc->n_params].value, param.value);
    ++proc->n_params;
    return;
}

static struct jcl_job *jcl_new_job(struct jcl_parse_info *info, user_t account, const char *owner) {
    struct jcl_job *job;
    job = kzalloc(sizeof(struct jcl_job));
    if(job == NULL) {
        kpanic("Out of memory");
    }
    job->account = account;
    job->owner = kmalloc(strlen(owner) + 1);
    strcpy(job->owner, owner);
    return job;
}

static void jcl_error(struct jcl_parse_info *info, const char *fmt, ...) {
    char tmpbuf[80], *tmpbuf_ptr = (char *)&tmpbuf;
    const char *data = info->data;
    size_t c_row = 0, i;
    va_list args;

    while(*data != '\n' && data != info->start) {
        ++c_row;
        --data;
    }
    ++data;

    while(*data != '\n' && *data != '\0') {
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
    for(i = 0; i < c_row; ++i) {
        *(tmpbuf_ptr++) = ' ';
    }
    *(tmpbuf_ptr++) = '^';
    *(tmpbuf_ptr++) = '\0';
    kprintf("%s\n", &tmpbuf);

    va_end(args);
    return;
}

static struct jcl_param jcl_get_positional(struct jcl_parse_info *info, const char *key, const char *fall_value) {
    struct jcl_param param;
    char is_multi = 0;
    size_t len;

    info->data += strspn(info->data, " ");

    len = strlen(key);
    param.key = kmalloc(len + 1);
    if(param.key == NULL) {
        kpanic("Out of memory");
    }
    strcpy(param.key, key);

    /* Obtain the value of the parameter */
    if(*info->data == ',') {
        /* When the stream is just a comma we apply a default */
        len = strlen(fall_value);
        param.value = kmalloc(len + 1);
        if(param.value == NULL) {
            kpanic("Out of memory");
        }
        strcpy(param.value, fall_value);
        ++info->data;
    } else {
        if(*info->data == '(') {
            is_multi = 1;
        }

        if(*info->data == '\'' || *info->data == '"') {
            ++info->data;
        }

        if(is_multi == 0) {
            len = strspn(info->data, param_key_accept);
        } else {
            len = strspn(info->data, multi_param_accept);
            if(info->data[len] != ')') {
                jcl_error(info, "Expected closing )");
                return param;
            }
            len++;
        }
        if(!len) {
            jcl_error(info, "Parameter %s has no value", param.key);
        }
        param.value = kmalloc(len + 1);
        if(param.value == NULL) {
            kpanic("Out of memory");
        }
        strncpy(param.value, info->data, len);
        info->data += len;

        if(*info->data == '\'' || *info->data == '"') {
            ++info->data;
        } if(*info->data == ',') {
            ++info->data;
        }
    }
    return param;
}

static void jcl_next_line(struct jcl_parse_info *info) {
    while(*info->data != '\0') {
        if(!strncmp(info->data, "//", 2)) {
            info->data += 2;

            /* If it is a comment, then skip it and repeat loop for next valid line */
            if(*info->data == '*') {
            skip_comment:
                while(*info->data != '\n') {
                    ++info->data;
                }
                continue;
            }
            return;
        } else if(!strncmp(info->data, "/*", 2)) {
            goto skip_comment;
        }
        ++info->data;
    }
    return;
}

static struct jcl_param jcl_get_optional(struct jcl_parse_info *info) {
    struct jcl_param param;
    size_t len;

    if(*info->data == ',') {
        ++info->data;
    }
    info->data += strspn(info->data, " ");

    len = strspn(info->data, param_key_accept);
    if(!len) {
        param.key = NULL;
        param.value = NULL;
        return param;
    }
    param.key = kmalloc(len + 1);
    strncpy(param.key, info->data, len);
    info->data += len;

    if(*info->data != '=') {
        jcl_error(info, "Expected = after %s", param.key);
        return param;
    }
    ++info->data;

    len = strspn(info->data, param_key_accept);
    if(!len) {
        jcl_error(info, "Zero-length value on param %s", param.key);
        return param;
    }
    param.value = kmalloc(len + 1);
    strncpy(param.value, info->data, len);

    /* Perform a substitution when a reference to a variable is encountered */
    if(*info->data == '&') {
        const char *value;
        value = jcl_resolve_variable_value(info, param.value);
        if(value == NULL) {
            jcl_error(info, "Variable %s not found", param.value);
        }
        kfree(param.value);
        param.value = kmalloc(strlen(value) + 1);
        strcpy(param.value, value);
    }
    info->data += len;

    /* This code will skip until the next parameter - if there is a comma but then a line
     * break occurs this code will position the reading cursor to prepare it for the next
     * reading of another optional parameter
     */
    if(*info->data == ',') {
        /* Skip the comma */
        ++info->data;

        /* Skip whitespace */
        info->data += strspn(info->data, " \t");

        /* If there is a newline it means we can have more parameters
         * due to a line break */
        if(*info->data == '\n') {
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

    jcl_create_variable(info, "SYSUID", user_from_uid(user_get_current())->name);

    ident = kmalloc(40 + 1);
    if(ident == NULL) {
        jcl_error(info, "Out of memory");
        goto end;
    }

    statement = kmalloc(40 + 1);
    if(statement == NULL) {
        jcl_error(info, "Out of memory");
        goto end;
    }

    while(*info->data != '\0') {
        jcl_next_line(info);
        if(*info->data == '\0') {
            break;
        }

        /* Obtain the identifier */
        info->data += strspn(info->data, " \t");
        len = strspn(info->data, identifier_accept);
        if(len >= 40) {
            jcl_error(info, "Identifier is too lengthy");
            goto end;
        }
        memcpy(ident, info->data, len);
        ident[len] = '\0';
        info->data += len;

        /* Obtain the statement */
        info->data += strspn(info->data, " \t");
        len = strspn(info->data, identifier_accept);
        if(len >= 40) {
            jcl_error(info, "Statement is too lengthy");
            goto end;
        }
        memcpy(statement, info->data, len);
        statement[len] = '\0';
        info->data += len;

        if(!strcmp(statement, "JOB")) {
            struct jcl_param param;
            user_t account;

            param = jcl_get_positional(info, "ACCNT", "(*)");
            if(!strcmp(param.value, "*")) {
                account = user_get_current();
            } else {
                /* Specified an ID */
                if(*param.value >= '0' && *param.value <= '9') {
                    account = user_get_by_name(user_from_uid(*param.value - '0')->name);
                } else {
                    account = user_get_by_name(param.value);
                }
            }
            jcl_destroy_param(param);

            param = jcl_get_positional(info, "PRGID", user_from_uid(user_get_current())->name);
            info->job = jcl_new_job(info, account, param.value);
            jcl_destroy_param(param);

            param = jcl_get_optional(info);
            while(param.key != NULL) {
                if(!strcmp(param.key, "CLASS")) {
                    if(*param.key >= '0' && *param.key <= '9') {
                        info->job->class = *param.key - '0';
                    } else if(*param.key >= 'A' && *param.key <= 'Z') {
                        info->job->class = *param.key - 'A';
                    } else {
                        jcl_error(info, "Unknown class %c", *param.key);
                        goto end;
                    }
                } else if(!strcmp(param.key, "PRTY")) {
                    info->job->priority = *param.key - '0';
                    if(info->job->priority >= 16) {
                        jcl_error(info, "Priority is higher than 16 (%u)", (unsigned)info->job->priority);
                        goto end;
                    }
                } else if(!strcmp(param.key, "NOTIFY")) {
                    
                } else if(!strcmp(param.key, "MSGCLASS")) {
                    if(*param.key >= '0' && *param.key <= '9') {
                        info->job->msg_class = *param.key - '0';
                    } else if(*param.key >= 'A' && *param.key <= 'Z') {
                        info->job->msg_class = *param.key - 'A';
                    } else {
                        jcl_error(info, "Unknown message class %c", *param.key);
                        goto end;
                    }
                } else if(!strcmp(param.key, "REGION")) {
                    const char *unit_spec;
                    info->job->region_size = (long unsigned int)atoi(param.key);
                    unit_spec = param.key + strspn(param.key, "0123456789");
                    switch(*unit_spec) {
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
                    if(info->job->region_size == 0) {
                        info->job->region_size = 1000000 * 16;
                    }
                } else if(!strcmp(param.key, "BYTES")) {
                    info->job->log_size = (long unsigned int)atoi(param.key);
                } else if(!strcmp(param.key, "USER")) {
                    info->job->submit_user = user_get_by_name(param.key);
                }

                kprintf("%s=%s\n", param.key, param.value);
                jcl_destroy_param(param);
                param = jcl_get_optional(info);
            }
        } else if(!strcmp(statement, "PROC")) {
            struct jcl_param param;
            struct jcl_procedure *proc;
            const char *body_end;

            if(info->curr_proc_ctx != NULL) {
                jcl_error(info, "Unimplemented nested procedures");
                goto end;
            }

            proc = jcl_create_procedure(info, ident);

            /* Obtain "default" parameters - procedures can have any number
             * of optional parameters */
            param = jcl_get_optional(info);
            while(param.key != NULL) {
                jcl_add_param_to_procedure(info, proc, param);

                jcl_destroy_param(param);
                param = jcl_get_optional(info);
            }
            info->curr_proc_ctx = proc;
            jcl_next_line(info);

            body_end = strstr(info->data, "PEND");
            if(body_end == NULL) {
                jcl_error(info, "Procedure without end");
                goto end;
            }

            /* Copy body of procedure */
            len = (size_t)((ptrdiff_t)body_end - (ptrdiff_t)info->data);
            proc->body = kmalloc(len + 1);
            if(proc->body == NULL) {
                jcl_error(info, "Out of memory");
                goto end;
            }
            memcpy(proc->body, info->data, len);
            info->data += len;
        } else if(!strcmp(statement, "DD")) {
            struct jcl_dataset *dataset;

            info->data += strspn(info->data, " ");
            if(*info->data == '*') {
                const char *data_start;
                char *tmpbuf;
                ++info->data;
                
                /* Go to the next line where the data is located on-stream */
                info->data = strchr(info->data, '\n');
                if(info->data == NULL) {
                    jcl_error(info, "Expected newline");
                    goto end;
                }
                ++info->data;

                data_start = info->data;
                info->data = strstr(info->data, "\n/*");
                if(info->data == NULL) {
                    jcl_error(info, "No end for in-stream dataset");
                    goto end;
                }

                len = (ptrdiff_t)info->data - (ptrdiff_t)data_start;
                tmpbuf = kmalloc(len + 1);
                if(tmpbuf == NULL) {
                    kpanic("Out of memory");
                }
                strncpy(tmpbuf, data_start, len);
                dataset = jcl_create_dataset(info, tmpbuf);
                dataset->type = JCL_INSTREAM;
                kfree(tmpbuf);

                kprintf("%s: %s\n", info->job->owner, dataset->value);

                /* Skip the newline and the stream end marker */
                info->data += 3;
                if(*info->data != '\n') {
                    jcl_error(info, "In-stream should end with newlines - not trash");
                    goto end;
                }
            } else {
                /* Get the key */
                struct jcl_param param;

                param = jcl_get_optional(info);
                kprintf("%s: %s=%s\n", info->job->owner, param.key, param.value);

                if(!strcmp(param.key, "SYSOUT")) {
                    dataset = jcl_create_dataset(info, param.value);
                    dataset->type = JCL_INSTREAM;
                } else if(!strcmp(param.key, "DUMMY")) {
                    dataset = jcl_create_dataset(info, NULL);
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

#include <s390/css.h>
static int ucli_cmd_info(int argc, char **argv) {
    kprintf("THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n");
    kprintf("EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n");
    kprintf("MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n");
    kprintf("IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY\n");
    kprintf("CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,\n");
    kprintf("TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE\n");
    kprintf("SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n");
    return 0;
}

static int ucli_cmd_lscss(int argc, char **argv) {
    struct css_schid schid = { 1, 0x0000 };
    struct css_schib schib = { 0 };
    int r;

    if(argc <= 1) {
        kprintf("Usage: lscss [id]\n");
        return -1;
    }

    schid.num = (unsigned)atoi(argv[1]);

    r = css_store_channel(schid, &schib);
    if(r != 0) {
        kprintf("Invalid channel\n");
        return -2;
    }

    schib.pmcw.flags |= CSS_PMCW_ENABLED(1);
    r = css_modify_channel(schid, &schib);
    if(r != 0) {
        kprintf("Modify channel error\n");
        return -1;
    }

    unsigned char data[255] = { 0 };
    struct css_ccw1 sense = { 0 };
    sense.cmd = 0x04;
    sense.count = sizeof(data);
    sense.data_addr = (uint32_t)&data;

    struct css_orb orb = { 0 };
    orb.flags |= CSS_ORB_FORMAT_2_IDAW_CTRL(1);
    orb.flags |= CSS_ORB_PREFETCH_CTRL(1);
    //orb.flags |= CSS_ORB_LPM_CTRL(0xff);
    orb.flags |= CSS_ORB_FORMAT_CTRL(1);
    orb.prog_addr = (uint32_t)&sense;
    r = css_start_channel(schid, &orb);
    if(r != 0) {
        kprintf("Start channel error\n");
        return -1;
    }

    struct css_irb irb = { 0 };
    irb.scsw.ccw_addr = (uint32_t)&sense;
    css_test_channel(schid, &irb);
    if(r != 0) {
        kprintf("Test channel error\n");
        return -1;
    }

    kprintf("*** Subchannel %x ***\n", (unsigned)atoi(argv[1]));
    kprintf("pmcw.int_param         %x\n", schib.pmcw.int_param);
    kprintf("pmcw.dev_num           %x\n", schib.pmcw.dev_num);
    kprintf("pmcw.lpm               %x\n", schib.pmcw.lpm);
    kprintf("pmcw.pnom              %x\n", schib.pmcw.pnom);
    kprintf("pmcw.lpum              %x\n", schib.pmcw.lpum);
    kprintf("pmcw.pim               %x\n", schib.pmcw.pim);
    kprintf("pmcw.mbi               %x\n", schib.pmcw.mbi);
    kprintf("pmcw.pom               %x\n", schib.pmcw.pom);
    kprintf("pmcw.pam               %x\n", schib.pmcw.pam);
    kprintf("pmcw.flags             %s, %s, ISC: %x, %s, %s, %s, %s, %x\n",
        schib.pmcw.flags & CSS_PMCW_DNV(1) ? "DeviceNumberValid" : "InvalidDeviceNumber",
        schib.pmcw.flags & CSS_PMCW_ENABLED(1) ? "Enabled" : "Disabled",
        schib.pmcw.flags & CSS_PMCW_ISC(2),
        schib.pmcw.flags & CSS_PMCW_LIMIT(1) ? "Limited" : "Unlimited",
        schib.pmcw.flags & CSS_PMCW_MM_ENABLE(1) ? "MeasureModeEnable" : "MeasureModeDisable",
        schib.pmcw.flags & CSS_PMCW_MULTIPATH_MODE(1) ? "MultiPath" : "NoMultiPath",
        schib.pmcw.flags & CSS_PMCW_TIMING(1) ? "TimingFacility" : "NoTimingFacility",
        (unsigned)schib.pmcw.flags
    );
    kprintf("pmcw.chpid             %x %x %x %x %x %x %x\n",
        (unsigned)schib.pmcw.chpid[0],
        (unsigned)schib.pmcw.chpid[1],
        (unsigned)schib.pmcw.chpid[2],
        (unsigned)schib.pmcw.chpid[3],
        (unsigned)schib.pmcw.chpid[4],
        (unsigned)schib.pmcw.chpid[5],
        (unsigned)schib.pmcw.chpid[6],
        (unsigned)schib.pmcw.chpid[7]
    );
    kprintf("pmcw.zero              %x %x %x %x\n",
        (unsigned)schib.pmcw.zero[0],
        (unsigned)schib.pmcw.zero[1],
        (unsigned)schib.pmcw.zero[2],
        (unsigned)schib.pmcw.last_flags
    );
    kprintf("scsw.flags             %x\n", schib.scsw.flags);
    kprintf("scsw.ccw_addr          %x\n", schib.scsw.ccw_addr);
    kprintf("scsw.device_status     %x\n", schib.scsw.device_status);
    kprintf("scsw.subchannel_status %x\n", schib.scsw.subchannel_status);
    kprintf("scsw.count             %x\n", schib.scsw.count);
    kprintf("mb_addr                %x\n", (unsigned)schib.mb_addr);
    kprintf("md_data                %x %x %x\n",
        (unsigned)schib.md_data[0],
        (unsigned)schib.md_data[1],
        (unsigned)schib.md_data[2]
    );

    for(size_t i = 0; i < sizeof(data); i++) {
        kprintf("%zu ", (size_t)data[i]);
    }
    kprintf("\n");
    return 0;
}

static int ucli_cmd_user(int argc, char **argv) {
    if(argc <= 1) {
        const struct user *c_user;
        c_user = user_from_uid(user_get_current());
        kprintf("%s\n", c_user->name);
    } else {
        user_t uid = (user_t)-1;
        if(argv[1][0] >= '0' && argv[1][0] <= '9') {
            uid = user_get_by_name(user_from_uid(atoi(argv[1]))->name);
        } else {
            uid = user_get_by_name(argv[1]);
        }

        if(uid == (user_t)-1) {
            kprintf("User %s does not exist\n", argv[1]);
            return -1;
        } else {
            user_set_current(uid);
        }
    }
    return 0;
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

static char tmpbuf[80];
int ucli_init(void) {
    struct jcl_parse_info info;
    char *filebuf = NULL, *linebuf;

    kprintf("\e[37;1m");
    kprintf("      $$$$$. .$$$$$. .$$$   \n");
    kprintf("\e[0m\e[37m");
    kprintf("      $    $ $     $ $      \n");
    kprintf("\e[36;1m");
    kprintf(" .  . $    $ $     $ `$$`.  \n");
    kprintf("\e[0m\e[36m");
    kprintf(" $  $ $    $ $     $     $  \n");
    kprintf("\e[34;1m");
    kprintf(" $  $ $    $ $     $     $  \n");
    kprintf("\e[0m\e[34m");
    kprintf(" `$$` $$$$$` `$$$$$` .$$.`  \n");
    kprintf("\e[0m");
    kprintf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    kprintf("JCL scripting is supported. \n");

    kprintf("An Operating System for mainframes and computers.\n");
    kprintf("Copyright (C) 2021 superleaf1995. All rights reserved. Type `info` for more info.\n");
    kprintf("Use `help` command to see more commands");

    info.start = test;
    info.data = info.start;
    info.variables = NULL;
    info.n_variables = 0;
    info.procedures = NULL;
    info.n_procedures = 0;
    info.datasets = NULL;
    info.n_datasets = 0;
    info.curr_proc_ctx = NULL;
    info.c_line = 0;
    info.job = NULL;
    jcl_parse(&info);

    for(size_t i = 0; i < info.n_procedures; i++) {
        struct jcl_procedure *proc = &info.procedures[i];

        kprintf("PROCEDURE %s\n[%s\n]\n", proc->name, proc->body);
        for(size_t j = 0; j < proc->n_params; j++) {
            kprintf("\t%s=%s\n", proc->params[i].key, proc->params[i].value);
        }
    }

    linebuf = kmalloc(70 + 1);
    if(linebuf == NULL) {
        kpanic("Ouf of memory");
    }

    while(1) {
        char *ptr = linebuf;
        size_t len = 0;

        kprintf("%s@> ", user_from_uid(user_get_current())->name);
        kflush();

        if(filebuf != NULL) {
            len = strlen(filebuf);
        }
        while(1) {
            int ch = kgetc();
            if((uintptr_t)ptr >= (uintptr_t)&linebuf[70] || ch == '\r') {
                kprintf("\n");
                *(ptr++) = '\n';
                *ptr = '\0';
                break;
            }
            *(ptr++) = (char)ch;

            switch(ch) {
            case 127:
                kputc('\b');
                kputc(' ');
                kputc('\b');
                *(--ptr) = '\0';
                ptr--;
                if((uintptr_t)ptr < (uintptr_t)&linebuf[0]) {
                    ptr = &linebuf[0];
                }
                break;
            default:
                kputc(ch);
                break;
            }
        }
        len += strlen(linebuf);

        filebuf = krealloc(filebuf, len + 1);
        if(filebuf == NULL) {
            kpanic("Ouf of memory");
        }
        strcat(filebuf, linebuf);

        *(strchr(linebuf, '\n')) = '\0';

        char *tok = linebuf, *start = linebuf;
        char **argv = NULL;
        int argc = 0;
        while(*start != '\0') {
            size_t len;

            argv = krealloc_array(argv, argc + 1, sizeof(char *));
            if(argv == NULL) {
                kpanic("Out of memory");
            }

            tok = strchr(start + 1, ' ');
            if(tok == NULL) {
                tok = start + strlen(start);
            }
            tok += strspn(tok, " ");

            len = strlen(start) - strlen(tok);
            argv[argc] = kmalloc(len + 1);
            if(argv[argc] == NULL) {
                kpanic("Out of memory");
            }
            strncpy(argv[argc], start, len);
            start = tok;
            ++argc;
        }

        if(!strncmp(argv[0], "run", 3)) {
            kprintf("\n*--- EXECUTING JCL ---*\n");
            info.start = filebuf;
            info.data = info.start;
            info.variables = NULL;
            info.n_variables = 0;
            info.procedures = NULL;
            info.n_procedures = 0;
            info.curr_proc_ctx = NULL;
            info.c_line = 0;
            jcl_parse(&info);
            kprintf("\n*---------------------*\n");
            kfree(filebuf);
            filebuf = NULL;
        } else if(!strncmp(argv[0], "info", 4)) {
            ucli_cmd_info(argc, argv);
        } else if(!strncmp(argv[0], "lscss", 5)) {
            ucli_cmd_lscss(argc, argv);
        } else if(!strncmp(argv[0], "help", 4)) {
            kprintf("run/jcl - execute JCL from stream buffer\n");
            kprintf("info - Information\n");
            kprintf("lscss [id] - List channel subsystem information\n");
            kprintf("cpu_reset [cpuid] - Reset a cpu");
            kprintf("cpu_on [cpuid] - Start a cpu");
            kprintf("cpu_off [cpuid] - Stop a cpu");
            kprintf("user [uid] - Get/set the current user\n");
        } else if(!strncmp(argv[0], "user", 4)) {
            ucli_cmd_user(argc, argv);
        } else if(!strncmp(argv[0], "cobc", 4)) {

        } else {
            kprintf("What?\n");
        }

        for(int i = 0; i < argc; i++) {
            kfree(argv[i]);
        }
        kfree(argv);
    }
    kfree(linebuf);
    //sci_init();
}
