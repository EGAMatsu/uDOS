#ifndef RTL_PARAMS_H
#define RTL_PARAMS_H

#include <dataset.h>
#include <terminal.h>

typedef struct {
    const char *key;
    const char *value;
}Param;

typedef struct {
    Param *params;
    size_t n_params;

    /* Terminals allocated to this task */
    Terminal *terms;
    size_t n_terms;

    /* Allocated datasets to this task (They are not locked to this task upon
     * initialization, the task should obtain the needed locking of said
     * datasets) - So we will only pass the name of said datasets and ONLY
     * lock them when the task requires it */
    const char **dsnames;
    size_t n_dsnames;
}ExecParams;

#endif
