#ifndef DEVICE_H
#define DEVICE_H

#include "device.h"

#include "iothub_device_client.h"

typedef struct{
    IOTHUB_DEVICE_CLIENT_LL_HANDLE *_handle;
    void *properties;
} device_t;

//void device_init(IOTHUB_DEVICE_CLIENT_HANDLE *h);

void device_sendReportState(device_t *device,const unsigned char *reportedProperties, int len);

void reportedStateCallback(int status_code, void* userContextCallback);

#endif