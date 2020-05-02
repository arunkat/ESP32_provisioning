#include "device.h"

#include <stdio.h>




//callbacks
void reportedStateCallback(int status_code, void* userContextCallback)
{
    (void)userContextCallback;
    printf("Device Twin reported properties update completed with result: %d\r\n", status_code);
}

//functions

void device_sendReportState(device_t *device, const unsigned char * reportedProperties, int len){
    
    IoTHubDeviceClient_LL_SendReportedState(*device->_handle, reportedProperties, len, reportedStateCallback, NULL);
}