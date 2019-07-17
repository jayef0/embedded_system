#ifndef MOD_FAULT_DETECTION_H
#define MOD_FAULT_DETECTION_H

#include <stdint.h>

#include "fault_codes.h"

typedef struct
{
    errorCode   inputError;     // Hardware errors detected by the inputs module
    float       pressure1;      // Pressure 1 as read from the inputs module
    float       pressure2;      // Pressure 2 as read from the inputs module
    float       temperature1;   // Temperature 1 as read from the inputs module
    float       temperature2;   // Temperature 2 as read from the inputs module
    float       temperature3;   // Temperature 3 as read from the inputs module
    float       temperature4;   // Temperature 4 as read from the inputs module
    healthState Estop;          // State of the estop (HEALTHY, FAULT)
} modFaultDetecTypeDef;

void module_fault_detect_update(void);
void module_fault_detect_init(void);
void module_fault_detect_task(void *argument);
modFaultDetecTypeDef* module_fault_detect_get_state(void);
void module_fault_detect_start(void);
void module_fault_detect_stop(void);
void module_fault_detect_set_estop(healthState);
void module_fault_detect_set_pressure1(float);
void module_fault_detect_set_pressure2(float);
void module_fault_detect_set_temperature1(float);
void module_fault_detect_set_temperature2(float);
void module_fault_detect_set_temperature3(float);
void module_fault_detect_set_temperature4(float);
void module_fault_detect_set_error(errorCode);
void module_fault_detect_manual_estop(int);
void module_fault_detect_manual_error(int);
void module_fault_detect_manual_pressure1(float);
void module_fault_detect_manual_pressure2(float);
void module_fault_detect_manual_temperature1(float);
void module_fault_detect_manual_temperature2(float);
void module_fault_detect_manual_temperature3(float);
void module_fault_detect_manual_temperature4(float);
void module_fault_detect_print_inputs(void);

#endif