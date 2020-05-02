// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// This sample shows how to translate the Device Twin json received from Azure IoT Hub into meaningful data for your application.
// It uses the parson library, a very lightweight json parser.

// There is an analogous sample using the serializer - which is a library provided by this SDK to help parse json - in devicetwin_simplesample.
// Most applications should use this sample, not the serializer.

// WARNING: Check the return of all API calls when developing your solution. Return checks ommited for sample simplification.

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "azure_c_shared_utility/macro_utils.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/platform.h"
#include "iothub_device_client.h"
#include "iothub_client_options.h"
#include "iothub.h"
#include "iothub_message.h"
#include "parson.h"
#include "sdkconfig.h"

#include "lightABCdevice.h"
#include "device.h"

// The protocol you wish to use should be uncommented
//
#define SAMPLE_MQTT
//#define SAMPLE_MQTT_OVER_WEBSOCKETS
//#define SAMPLE_AMQP
//#define SAMPLE_AMQP_OVER_WEBSOCKETS
//#define SAMPLE_HTTP

#ifdef SAMPLE_MQTT
    #include "iothubtransportmqtt.h"
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
    #include "iothubtransportmqtt_websockets.h"
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
    #include "iothubtransportamqp.h"
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
    #include "iothubtransportamqp_websockets.h"
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
    #include "iothubtransporthttp.h"
#endif // SAMPLE_HTTP

/* Paste in the your iothub device connection string  */

IOTHUB_DEVICE_CLIENT_LL_HANDLE *handle;

static const char* connectionString = "HostName=eraniothub1.azure-devices.net;DeviceId=hwLightABC01;x509=true";
/*
{
  "registrationId": "hwLightABC01",
  "deviceType": "LightABC",
  "provisioned": true,
  "cert": "-----BEGIN CERTIFICATE-----\nMIICwjCCAaqgAwIBAgIEOq0T2TANBgkqhkiG9w0BAQsFADATMREwDwYDVQQDDAhM\naWdodEFCQzAeFw0yMDA0MTcxMjA2MzBaFw0yMTA0MTcxMjA2MzBaMBcxFTATBgNV\nBAMMDGh3TGlnaHRBQkMwMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\nAOYte6l7IAKjC61HNhwfbyd+wRp6z1JoU2cVD8QIUo1la+FODidT1fX2IoDyd2WF\nIEPEYPRhmOh5U2tv52HE8sfsDS5KRi314PyjUUOrP9kIz2fRXPA9Kmj2HAqIapMu\n+sqKl2iue4C9FGiKR64GGonarMmQX1CaB0PRwGbPXN9PFaIasn67iomE+F13Ujp/\nLsNQTVpFM/g+QwdfhP3rEfUQLIZIN6Bf4WkojQhLWNz5PznT9irIh1AB303mbJK4\nGrK5i3nfqq+Iwne2kaGcAUD9UtQaO6NB9zM2kFzdWDbZaa6OiUGPu9Tex+81/pps\nITcV+QFjjMmZFJVYwLWxdWcCAwEAAaMaMBgwFgYDVR0lAQH/BAwwCgYIKwYBBQUH\nAwIwDQYJKoZIhvcNAQELBQADggEBADOnKPcm48473noxTbzMkfQKDWtKK91k1E4N\ndiRbVLSGYU8AuxyPqV4/dgzJElrMVH5YzKw6Ar3K5lpoHprsjkhD8Jsr2WAfWTiw\ndyUH6yxJvSiaUJhEKy6X5rHa8gJ5NqRQZZOvaFikVJh3uMBK7a9OJ3yq+yBPyZC0\nb6dfAQSsjuFHu8jasbOr6zzA1s3TYluauFWoZsyAJJHISefVKc7N3PqfFru0sc/2\nnYi5fD9OxPOHbrBLda81Nh339/EblgbwnWnGEZOmQZgSRgLkZloYBcwlTnzbpnAM\npLEf5Ju3g2zwjBACrRfaIjz0d8+ZkNcnK9RYbqc8N2+vFYCsDEA=\n-----END CERTIFICATE-----",
  "key": "-----BEGIN RSA PRIVATE KEY-----\nMIIEpAIBAAKCAQEA5i17qXsgAqMLrUc2HB9vJ37BGnrPUmhTZxUPxAhSjWVr4U4O\nJ1PV9fYigPJ3ZYUgQ8Rg9GGY6HlTa2/nYcTyx+wNLkpGLfXg/KNRQ6s/2QjPZ9Fc\n8D0qaPYcCohqky76yoqXaK57gL0UaIpHrgYaidqsyZBfUJoHQ9HAZs9c308Vohqy\nfruKiYT4XXdSOn8uw1BNWkUz+D5DB1+E/esR9RAshkg3oF/haSiNCEtY3Pk/OdP2\nKsiHUAHfTeZskrgasrmLed+qr4jCd7aRoZwBQP1S1Bo7o0H3MzaQXN1YNtlpro6J\nQY+71N7H7zX+mmwhNxX5AWOMyZkUlVjAtbF1ZwIDAQABAoIBAQC87HIGjn+cinTI\nGZ3pAUf7k8ctU8Wc7vIdtqTFEsunMKqWN7nYP7Bq/EYfrmOfWOA9nw6xJvYZQZPd\np/CzR7K5sx6yctYdXSX4VpgZwZJbMicCIE53BM0tb2tenc9T1QiVe6GAk03dQdRh\nZbYluO7JXUna+vuwrWvvF1cjS2oAAltfr8BJoFIOAFbhd/wOXesPLgHwI2LJvC61\nvfGtNrGlNqeUE0Q/muIs8eXzBHQQtpd9GLmTIh6gaFSzUVXaI2I7nn8uVSegraPK\n+kmhHp3fgpEDHP70SdsE4C8EgRTSWadbzhMOuQcMalFIhqrCPbqaAUpUTsog07R0\niN40s69BAoGBAP6hdA3KhUAFGZ/xhKDw/RQUXzdGx/jDVk727Jce5wU1OViC9BSv\nqKGsqxeIGmNLX2oSAE+0feczLNPVh/KuCeNqa2gs5EaE6cRQk+bjAvliNytVLBpL\nzorxAp2Rc0jIlYm+dtwidvacxAruORCBr0wMDo/RANjz07GzcoFi8miHAoGBAOdq\nXZcjeAV4Rt3zSidkv/FXDwtqN/UQyffytemRcJLYCvc8oTM1LPtskuslyMkc1idu\n9MaLVBVBwQSK4wNg39ybLEqsjwGfxXMGgGBwFHlQT61glfPDA3jAr6iWI8jzRGKn\n0Vw9kbr00ByAfaxrr/5Dw0gr/z5UFHfHXFyHCSQhAoGARJFVnyEaINM+w0NWY8CB\nZhbWTRxSXTq80ybLLyazL0PV3W/mKmvjDSZiLEQKVxLE7ttKGiyQeuHdAG5P3Zng\nL81IfxUXo6XHDYZlTZd0BZPdJ14YMjyXsfKUsbmpQcBCBIW1nDHrtx0f7ZGY7Ej/\n24qjoTa287U1HHUmMJFklaECgYEAv4dQGIP5lQVcGdx/FiWTmwpD4F20HHcdwcI2\nfy6pbk+ym7epbzlmllzhKA+oo5LjR9XUbvLnz4QRXVIZ2zT1cp9XRCKXZW+3uqC5\n5Zc9yr4Gg+d5lDtmBy3q9Gv3CB0XD1P3uhEXKRXvnHdYDDlAev/Yg0YuxYZPPmdY\n8ReuICECgYBqZb+9VrA5S58KJ23n7YftLULySLDygkqkbAE6pP4yjaUJB7MM42BP\nwJGvlImwQH3ibEqapXdt7y73R8zVLvFB2HfrVI4IOpuOdHxsv5EtWk63otQwhSE0\nNmw8jRDTqHu044pj+3s5FtjGPS48o4BxJMbaZ3eQ7xVQU3Loa/WfbA==\n-----END RSA PRIVATE KEY-----",
  "deviceId": "hwLightABC01",
  "hubName": "eraniothub1.azure-devices.net"
}*/

static const char* x509certificate =
"-----BEGIN CERTIFICATE-----\n" 
"MIICwjCCAaqgAwIBAgIEOq0T2TANBgkqhkiG9w0BAQsFADATMREwDwYDVQQDDAhM\n"
"aWdodEFCQzAeFw0yMDA0MTcxMjA2MzBaFw0yMTA0MTcxMjA2MzBaMBcxFTATBgNV\n"
"BAMMDGh3TGlnaHRBQkMwMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n"
"AOYte6l7IAKjC61HNhwfbyd+wRp6z1JoU2cVD8QIUo1la+FODidT1fX2IoDyd2WF\n"
"IEPEYPRhmOh5U2tv52HE8sfsDS5KRi314PyjUUOrP9kIz2fRXPA9Kmj2HAqIapMu\n"
"+sqKl2iue4C9FGiKR64GGonarMmQX1CaB0PRwGbPXN9PFaIasn67iomE+F13Ujp/\n"
"LsNQTVpFM/g+QwdfhP3rEfUQLIZIN6Bf4WkojQhLWNz5PznT9irIh1AB303mbJK4\n"
"GrK5i3nfqq+Iwne2kaGcAUD9UtQaO6NB9zM2kFzdWDbZaa6OiUGPu9Tex+81/pps\n"
"ITcV+QFjjMmZFJVYwLWxdWcCAwEAAaMaMBgwFgYDVR0lAQH/BAwwCgYIKwYBBQUH\n"
"AwIwDQYJKoZIhvcNAQELBQADggEBADOnKPcm48473noxTbzMkfQKDWtKK91k1E4N\n"
"diRbVLSGYU8AuxyPqV4/dgzJElrMVH5YzKw6Ar3K5lpoHprsjkhD8Jsr2WAfWTiw\n"
"dyUH6yxJvSiaUJhEKy6X5rHa8gJ5NqRQZZOvaFikVJh3uMBK7a9OJ3yq+yBPyZC0\n"
"b6dfAQSsjuFHu8jasbOr6zzA1s3TYluauFWoZsyAJJHISefVKc7N3PqfFru0sc/2\n"
"nYi5fD9OxPOHbrBLda81Nh339/EblgbwnWnGEZOmQZgSRgLkZloYBcwlTnzbpnAM\n"
"pLEf5Ju3g2zwjBACrRfaIjz0d8+ZkNcnK9RYbqc8N2+vFYCsDEA=\n"
"-----END CERTIFICATE-----";
/*
"-----BEGIN CERTIFICATE-----""\n"
"MIICpDCCAYwCCQCfIjBnPxs5TzANBgkqhkiG9w0BAQsFADAUMRIwEAYDVQQDDAls""\n"
"b2NhbGhvc3QwHhcNMTYwNjIyMjM0MzI3WhcNMTYwNjIzMjM0MzI3WjAUMRIwEAYD""\n"
"...""\n"
"+s88wBF907s1dcY45vsG0ldE3f7Y6anGF60nUwYao/fN/eb5FT5EHANVMmnK8zZ2""\n"
"tjWUt5TFnAveFoQWIoIbtzlTbOxUFwMrQFzFXOrZoDJmHNWc2u6FmVAkowoOSHiE""\n"
"dkyVdoGPCXc=""\n"
"-----END CERTIFICATE-----";
*/
static const char* x509privatekey =
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpAIBAAKCAQEA5i17qXsgAqMLrUc2HB9vJ37BGnrPUmhTZxUPxAhSjWVr4U4O\n"
"J1PV9fYigPJ3ZYUgQ8Rg9GGY6HlTa2/nYcTyx+wNLkpGLfXg/KNRQ6s/2QjPZ9Fc\n"
"8D0qaPYcCohqky76yoqXaK57gL0UaIpHrgYaidqsyZBfUJoHQ9HAZs9c308Vohqy\n"
"fruKiYT4XXdSOn8uw1BNWkUz+D5DB1+E/esR9RAshkg3oF/haSiNCEtY3Pk/OdP2\n"
"KsiHUAHfTeZskrgasrmLed+qr4jCd7aRoZwBQP1S1Bo7o0H3MzaQXN1YNtlpro6J\n"
"QY+71N7H7zX+mmwhNxX5AWOMyZkUlVjAtbF1ZwIDAQABAoIBAQC87HIGjn+cinTI\n"
"GZ3pAUf7k8ctU8Wc7vIdtqTFEsunMKqWN7nYP7Bq/EYfrmOfWOA9nw6xJvYZQZPd\n"
"p/CzR7K5sx6yctYdXSX4VpgZwZJbMicCIE53BM0tb2tenc9T1QiVe6GAk03dQdRh\n"
"ZbYluO7JXUna+vuwrWvvF1cjS2oAAltfr8BJoFIOAFbhd/wOXesPLgHwI2LJvC61\n"
"vfGtNrGlNqeUE0Q/muIs8eXzBHQQtpd9GLmTIh6gaFSzUVXaI2I7nn8uVSegraPK\n"
"+kmhHp3fgpEDHP70SdsE4C8EgRTSWadbzhMOuQcMalFIhqrCPbqaAUpUTsog07R0\n"
"iN40s69BAoGBAP6hdA3KhUAFGZ/xhKDw/RQUXzdGx/jDVk727Jce5wU1OViC9BSv\n"
"qKGsqxeIGmNLX2oSAE+0feczLNPVh/KuCeNqa2gs5EaE6cRQk+bjAvliNytVLBpL\n"
"zorxAp2Rc0jIlYm+dtwidvacxAruORCBr0wMDo/RANjz07GzcoFi8miHAoGBAOdq\n"
"XZcjeAV4Rt3zSidkv/FXDwtqN/UQyffytemRcJLYCvc8oTM1LPtskuslyMkc1idu\n"
"9MaLVBVBwQSK4wNg39ybLEqsjwGfxXMGgGBwFHlQT61glfPDA3jAr6iWI8jzRGKn\n"
"0Vw9kbr00ByAfaxrr/5Dw0gr/z5UFHfHXFyHCSQhAoGARJFVnyEaINM+w0NWY8CB\n"
"ZhbWTRxSXTq80ybLLyazL0PV3W/mKmvjDSZiLEQKVxLE7ttKGiyQeuHdAG5P3Zng\n"
"L81IfxUXo6XHDYZlTZd0BZPdJ14YMjyXsfKUsbmpQcBCBIW1nDHrtx0f7ZGY7Ej/\n"
"24qjoTa287U1HHUmMJFklaECgYEAv4dQGIP5lQVcGdx/FiWTmwpD4F20HHcdwcI2\n"
"fy6pbk+ym7epbzlmllzhKA+oo5LjR9XUbvLnz4QRXVIZ2zT1cp9XRCKXZW+3uqC5\n"
"5Zc9yr4Gg+d5lDtmBy3q9Gv3CB0XD1P3uhEXKRXvnHdYDDlAev/Yg0YuxYZPPmdY\n"
"8ReuICECgYBqZb+9VrA5S58KJ23n7YftLULySLDygkqkbAE6pP4yjaUJB7MM42BP\n"
"wJGvlImwQH3ibEqapXdt7y73R8zVLvFB2HfrVI4IOpuOdHxsv5EtWk63otQwhSE0\n"
"Nmw8jRDTqHu044pj+3s5FtjGPS48o4BxJMbaZ3eQ7xVQU3Loa/WfbA==\n"
"-----END RSA PRIVATE KEY-----";

#define DOWORK_LOOP_NUM     3


//  Converts the light object into a JSON blob with reported properties that is ready to be sent across the wire as a twin.


extern char* serializeToJson(void* props);
extern bool parseFromJson(const char* json, bool twin_update_state, void* props);
extern void deviceTwinCallback(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size, void* userContextCallback);

//extern void reportedStateCallback(int status_code, void* userContextCallback);

extern int deviceMethodCallback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* response_size, void* userContextCallback);



void iothub_client_device_twin_and_methods_run(void)
{
    IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol;
    IOTHUB_DEVICE_CLIENT_LL_HANDLE iotHubClientHandle;
    handle = &iotHubClientHandle;
    device_t device;
    device._handle = &iotHubClientHandle;
    device.properties = lightABC_AllocateProps();

    // Select the Protocol to use with the connection
#ifdef SAMPLE_MQTT
    protocol = MQTT_Protocol;
#endif // SAMPLE_MQTT
#ifdef SAMPLE_MQTT_OVER_WEBSOCKETS
    protocol = MQTT_WebSocket_Protocol;
#endif // SAMPLE_MQTT_OVER_WEBSOCKETS
#ifdef SAMPLE_AMQP
    protocol = AMQP_Protocol;
#endif // SAMPLE_AMQP
#ifdef SAMPLE_AMQP_OVER_WEBSOCKETS
    protocol = AMQP_Protocol_over_WebSocketsTls;
#endif // SAMPLE_AMQP_OVER_WEBSOCKETS
#ifdef SAMPLE_HTTP
    protocol = HTTP_Protocol;
#endif // SAMPLE_HTTP

    if (IoTHub_Init() != 0)
    {
        printf("Failed to initialize the platform.\r\n");
    }
    else
    {
        if ((iotHubClientHandle = IoTHubDeviceClient_LL_CreateFromConnectionString(connectionString, protocol)) == NULL)
        {
            printf("ERROR: iotHubClientHandle is NULL!\r\n");
        }
        else
        {
            // Uncomment the following lines to enable verbose logging (e.g., for debugging).
            //bool traceOn = true;
            //(void)IoTHubDeviceClient_SetOption(iotHubClientHandle, OPTION_LOG_TRACE, &traceOn);
            if((IoTHubDeviceClient_LL_SetOption(iotHubClientHandle, OPTION_X509_CERT, x509certificate) != IOTHUB_CLIENT_OK) ||
            (IoTHubDeviceClient_LL_SetOption(iotHubClientHandle, OPTION_X509_PRIVATE_KEY, x509privatekey) != IOTHUB_CLIENT_OK))
            {
                (void)printf("failure to set option \"TrustedCerts\"\r\n");
            }

            //LightABC_t Light;
            //memset(&Light, 0, sizeof(LightABC_t));

            char* reportedProperties = serializeToJson(device.properties);
            (void)IoTHubDeviceClient_LL_SendReportedState(iotHubClientHandle, (const unsigned char*)reportedProperties, strlen(reportedProperties), reportedStateCallback, NULL);
            (void)IoTHubDeviceClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, &device);
            (void)IoTHubDeviceClient_LL_SetDeviceTwinCallback(iotHubClientHandle, deviceTwinCallback, &device);

            while (1) {
				IoTHubDeviceClient_LL_DoWork(iotHubClientHandle);
				ThreadAPI_Sleep(10);
			}

            IoTHubDeviceClient_LL_Destroy(iotHubClientHandle);
            free(reportedProperties);
            free(device.properties);
        }

        IoTHub_Deinit();
    }
}
