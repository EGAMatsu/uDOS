#ifndef DATASET_H
#define DATASET_H

#include <stddef.h>

typedef struct {
    void *handle;
}Dataset;

Dataset *RtlOpenDataset(const char *name, const char *recfm);
int RtlReadDatasetLine(Dataset *ds, char *buffer, size_t n);
int RtlReadDataset(Dataset *ds, char *buffer, size_t n);

#endif