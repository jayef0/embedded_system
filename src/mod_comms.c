#include <stdio.h>

#include "mod_state_machine.h"
#include "mod_comms.h"

#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"


/* Misc. module specific varables */
static uint8_t _is_init = 0;
static uint8_t _is_running = 0;

/* Structures defined for the comms thread */
static osThreadId_t _comms_thread_id;
static osThreadAttr_t _comms_attr =
{
    .name = "commsModule",
    .priority = osPriorityNormal //Set initial thread priority
};


/* Task to manage comm thread module */
void module_comms_task(void *argument)
{
    UNUSED(argument);
    while(1)
    {
        // Update state of fault detection module
        module_comms_heartbeat(); //send the heartbeat
        // Wait 500ms between updates
        osDelay(3000); //wait another 3 seconds to send to the control room
    }
}



///////////////////////////////////// INTERFACE FUNCTIONS /////////////////////////////////////
void module_comms_start_received(void)
{
    // send start signal to state machine module
    module_state_machine_start_signal();

    // Update the state machine module
    module_state_machine_update();

    // Retrieve state machine state
    modStateMachineTypeDef* sstateMachineState;
    sstateMachineState = module_state_machine_get_state();

    // Print fault detection state
    printf("State machine state: %i\n", sstateMachineState->smState);
}

void module_comms_heartbeat(void)
{
    // Read current state and error code
    modStateMachineTypeDef* smState;

    smState = module_state_machine_get_state();

    /* Print state to serial */
    printf("Status: State %i, Error: %i\n", smState->smState, smState->Error);
}


///////////////////////////////////// ADDITIONAL FUNCTIONS /////////////////////////////////////
void module_comms_init(void)
{
   if (!_is_init){

    // create thread for kernel to manage
    _comms_thread_id = osThreadNew(module_comms_task, NULL, &_comms_attr);
    _is_init = 1;
    _is_running = 1;

    }


}


/* start outputs module */
void module_comms_start(void)
{
    if(!_is_running && _is_init)
    {

         osThreadResume(_comms_thread_id ); //resume thread
        _is_running = 1;
    }
    printf("Comms module on\n");
}

/* stop outputs module */
void module_comms_stop(void)
{
    if(_is_running && _is_init)
    {

        osThreadSuspend(_comms_thread_id ); //suspend thread
        _is_running = 0;

    }
    printf("Comms module off\n");
}
