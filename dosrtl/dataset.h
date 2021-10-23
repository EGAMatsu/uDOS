#ifndef DATASET_H
#define DATASET_H

#include <stddef.h>

#define EOF ((int)-1)

typedef struct {
    void *handle;
}Dataset;

Dataset *RtlOpenDataset(const char *name, const char *recfm);
void RtlCloseDataset(Dataset *ds);

int RtlReadDatasetLine(Dataset *ds, char *buffer, size_t n);
int RtlReadDatasetChar(Dataset *ds);
int RtlReadDataset(Dataset *ds, char *buffer, size_t n);

#endif