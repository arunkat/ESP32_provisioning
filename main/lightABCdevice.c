#include "lightABCdevice.h"
#include "device.h"

#include <stdio.h>
#include <stdlib.h>



#include "parson.h"


void * lightABC_AllocateProps()
{
    LightABC_t * props = malloc(sizeof(LightABC_t));
    memset(props, 0, sizeof(LightABC_t));
    return (void *) props;
}


char* serializeToJson(void* props)
{
    char* result;

    LightABC_t *light = (LightABC_t *)props;
    JSON_Value* root_value = json_value_init_object();
    JSON_Object* root_object = json_value_get_object(root_value);

    // Only reported properties:

    (void)json_object_set_boolean(root_object, "lightOn", light->lightOn);
    (void)json_object_set_boolean(root_object, "colorMode", light->colorMode);
    (void)json_object_set_number(root_object, "grayLevel", light->grayLevel);
    (void)json_object_set_number(root_object, "rLevel", light->rLevel);
    (void)json_object_set_number(root_object, "gLevel", light->gLevel);
    (void)json_object_set_number(root_object, "bLevel", light->bLevel);

    result = json_serialize_to_string(root_value);

    json_value_free(root_value);

    return result;
}

//  Converts the desired properties of the Device Twin JSON blob received from IoT Hub into a Car object.
bool _parseFromJson(const char* json, bool twin_update_state, void* props)
{
    LightABC_t *light = (LightABC_t *)props;
    bool result = true;
    JSON_Value* root_value = NULL;
    JSON_Object* root_object = NULL;

    if (!light) {
        result = false;
    } else {
        memset(light, 0, sizeof(LightABC_t));

        root_value = json_parse_string(json);
        root_object = json_value_get_object(root_value);

        // Only desired properties:
        JSON_Value* lightOnVal;
        JSON_Value* colorModeVal;
        JSON_Value* grayLevelVal;
        JSON_Value* rLevelVal;
        JSON_Value* gLevelVal;
        JSON_Value* bLevelVal;

        JSON_Object* obj = root_object;

        if (twin_update_state){
            obj = json_object_get_object(root_object, "desired");
        }

        lightOnVal = json_object_get_value(obj, "lightOn");
        colorModeVal = json_object_get_value(obj, "colorMode");
        grayLevelVal = json_object_get_value(obj, "grayLevel");
        rLevelVal = json_object_get_value(obj, "rLevel");
        gLevelVal = json_object_get_value(obj, "gLevel");
        bLevelVal = json_object_get_value(obj, "bLevel");
/*
        if (update_state == DEVICE_TWIN_UPDATE_COMPLETE)
        {
            changeOilReminder = json_object_dotget_value(root_object, "desired.changeOilReminder");
            desired_maxSpeed = json_object_dotget_value(root_object, "desired.settings.desired_maxSpeed");
            latitude = json_object_dotget_value(root_object, "desired.settings.location.latitude");
            longitude = json_object_dotget_value(root_object, "desired.settings.location.longitude");
        }
        else
        {
            changeOilReminder = json_object_dotget_value(root_object, "changeOilReminder");
            desired_maxSpeed = json_object_dotget_value(root_object, "settings.desired_maxSpeed");
            latitude = json_object_dotget_value(root_object, "settings.location.latitude");
            longitude = json_object_dotget_value(root_object, "settings.location.longitude");
        }
*/
        if (lightOnVal){
            light->lightOn = json_value_get_boolean(lightOnVal);
            printf("lightOn: %d", light->lightOn);
        }
        if (colorModeVal){
            light->colorMode = json_value_get_boolean(colorModeVal);
        }
        if (grayLevelVal){
            light->grayLevel = json_value_get_number(grayLevelVal);
        }
        if (rLevelVal){
            light->rLevel = json_value_get_number(rLevelVal);
        }
        if (gLevelVal){
            light->gLevel = json_value_get_number(gLevelVal);
        }
        if (bLevelVal){
            light->bLevel = json_value_get_number(bLevelVal);
        }
        json_value_free(root_value);
    }
    return result;
}



//----------------------------------------------callback definitions----------------------------------------------


void deviceTwinCallback(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char* payLoad, size_t size, void* userContextCallback)
{
    printf("in device twin callback\n\n%s\n\n",payLoad);
    
    (void)update_state;
    (void)size;

    device_t* dev = (device_t*)userContextCallback;
    LightABC_t *oldLightStatePtr = dev->properties;
    LightABC_t newLightState;
    _parseFromJson((const char*)payLoad, update_state == DEVICE_TWIN_UPDATE_COMPLETE, &newLightState); bool lightOn;
    bool colorMode;
    float grayLevel;
    float rLevel;
    float gLevel;
    float bLevel;

    if(oldLightStatePtr->lightOn != newLightState.lightOn){
        printf("LightOn changed state to %s\n", newLightState.lightOn?"On":"Off");
        oldLightStatePtr->lightOn = newLightState.lightOn;
    }
    if(oldLightStatePtr->colorMode != newLightState.colorMode){
        printf("colorMode changed state to %s\n", newLightState.colorMode?"On":"Off");
        oldLightStatePtr->colorMode = newLightState.colorMode;
    }
    if(oldLightStatePtr->grayLevel != newLightState.grayLevel){
        printf("grayLevel changed to %f\n", newLightState.grayLevel);
        oldLightStatePtr->grayLevel = newLightState.grayLevel;
    }
    if(oldLightStatePtr->rLevel != newLightState.rLevel){
        printf("rLevel changed to %f\n", newLightState.rLevel);
        oldLightStatePtr->rLevel = newLightState.rLevel;
    }
    if(oldLightStatePtr->gLevel != newLightState.gLevel){
        printf("gLevel changed to %f\n", newLightState.gLevel);
        oldLightStatePtr->gLevel = newLightState.gLevel;
    }
    if(oldLightStatePtr->bLevel != newLightState.bLevel){
        printf("bLevel changed to %f\n", newLightState.bLevel);
        oldLightStatePtr->bLevel = newLightState.bLevel;
    }

    //report changes
    char* reportedProperties = serializeToJson((void *)oldLightStatePtr);
    device_sendReportState(dev, (unsigned char*)reportedProperties, strlen(reportedProperties));
    free(reportedProperties);
}

int deviceMethodCallback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* response_size, void* userContextCallback)
{
    
    device_t* dev = (device_t*)userContextCallback;
    LightABC_t* LightStatePtr = dev->properties;
    (void)payload;
    (void)size;

    int result;
    //respond to method call
    if(!strcmp(method_name, "lightOn")){
        const char deviceMethodResponse[] = "{ \"Response\": \"Ack\" }";
        LightStatePtr->lightOn = true;
        printf("light on\n");
        *response_size = sizeof(deviceMethodResponse)-1;
        *response = malloc(*response_size);
        (void)memcpy(*response, deviceMethodResponse, *response_size);
        result = 200;
    } else if(!strcmp(method_name, "lightOff")){
        const char deviceMethodResponse[] = "{ \"Response\": \"Ack\" }";
        LightStatePtr->lightOn = false;
        printf("light off\n");
        *response_size = sizeof(deviceMethodResponse)-1;
        *response = malloc(*response_size);
        (void)memcpy(*response, deviceMethodResponse, *response_size);
        result = 200;
    } else {
        // All other entries are ignored.
        const char deviceMethodResponse[] = "{\"Error\": \"Unsupported Method\"}";
        *response_size = sizeof(deviceMethodResponse)-1;
        *response = malloc(*response_size);
        (void)memcpy(*response, deviceMethodResponse, *response_size);
        result = -1;
    }

    //report changes
    char* reportedProperties = serializeToJson((void *)LightStatePtr);
    device_sendReportState(dev, (const unsigned char*)reportedProperties, strlen(reportedProperties));
    free(reportedProperties);
    return result;
}