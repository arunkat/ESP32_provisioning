#ifndef LIGHTABCDEVICE_H
#define LIGHTABCDEVICE_H


#include <stdint.h>
#include <stdbool.h>


typedef struct
{
    bool lightOn;
    bool colorMode;
    float grayLevel;
    float rLevel;
    float gLevel;
    float bLevel;
} LightABC_t;

void * lightABC_AllocateProps();

#endif