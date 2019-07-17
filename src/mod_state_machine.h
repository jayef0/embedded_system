#ifndef MOD_STATE_MACHINE_H
#define MOD_STATE_MACHINE_H

#include <stdint.h>
#include "fault_codes.h"

typedef enum{
    SM_HEALTHY = 0,
    SM_FAULT,
    SM_WAITING,
} statemachineStates;

typedef struct
{
    errorCode               Error;      // All errors detected (hardware + limits)
    statemachineStates      smState;    // Current state of the state machine
} modStateMachineTypeDef;

void module_state_machine_init(void);
void module_state_machine_task(void *argument);
void    module_state_machine_update(void);
void    module_state_machine_start_signal(void);
modStateMachineTypeDef* module_state_machine_get_state(void);
void module_state_machine_start(void);
void module_state_machine_stop(void);
void module_state_machine_set_fault(errorCode);
void module_state_machine_manual_error(int);


void module_state_machine_tempchecker(int); //self-made to check temperature

#endif
