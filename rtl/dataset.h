#ifndef DATASET_H
#define DATASET_H

#include <stddef.h>

#define EOF ((int)-1)

typedef struct {
    void *handle;
}Dataset;

#define RtlOpenDataset _Zrdsop
Dataset *RtlOpenDataset(const char *name, const char *recfm);
#define RtlCloseDataset _Zrdscl
void RtlCloseDataset(Dataset *ds);

#define RtlReadDatasetLine _Zrdsrl
int RtlReadDatasetLine(Dataset *ds, char *buffer, size_t n);
#define RtlReadDatasetChar _Zrdsrc
int RtlReadDatasetChar(Dataset *ds);
#define RtlReadDataset _Zrdsrd
int RtlReadDataset(Dataset *ds, char *buffer, size_t n);
#define RtlWriteDataset _Zrdswrd
int RtlWriteDataset(Dataset *ds, const char *buffer, size_t n);

#endif
