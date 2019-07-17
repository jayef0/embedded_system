#include <stdint.h>
#include <stdlib.h>
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

#include "mod_fault_detection.h"
#include "mod_state_machine.h"
#include "fault_codes.h"
#include "ref_r410a_props.h"



/* Misc. module specific varables */
float maxPressureOut = 4200.0;
float maxTemperatureOut = 100.0;
float maxTemperature1Int = 80.0;
float minTemperature3 = 0.0;

float sum  = 0.0 ; // SELF-ADDED, use as a timer (update every 0.5 seconds)
//Global variable, value can change in function without getting re-initialised.



static uint8_t _is_init = 0;
static uint8_t _is_running = 0;

/* Structure to store values pushed from inputs module */
static modFaultDetecTypeDef sFaultDetecState;

/* Structures defined for the fault detection thread */
static osThreadId_t _fault_detect_thread_id;
static osThreadAttr_t _fault_detect_attr =
{
    .name = "faultDetectModule",
    .priority = osPriorityNormal //Set initial thread priority
};



///////////////////////////////////// INTERFACE FUNCTIONS /////////////////////////////////////
/* Update the state of the estop */
void module_fault_detect_set_estop(healthState newEstop)
{
    // 0= healthy, 1 = fault
    sFaultDetecState.Estop = newEstop;
}

/* Update pressure 1 */
void module_fault_detect_set_pressure1(float newPressure)
{
    sFaultDetecState.pressure1 = newPressure;
}

/* Update pressure 2 */
void module_fault_detect_set_pressure2(float newPressure)
{
    sFaultDetecState.pressure2 = newPressure;
}

/* Update the tank temperature */
void module_fault_detect_set_temperature1(float newTemperature)
{
    sFaultDetecState.temperature1 = newTemperature;
}

/* Update the tank temperature */
void module_fault_detect_set_temperature2(float newTemperature)
{
    sFaultDetecState.temperature2 = newTemperature;
}

/* Update the tank temperature */
void module_fault_detect_set_temperature3(float newTemperature)
{
    sFaultDetecState.temperature3 = newTemperature;
}

/* Update the tank temperature */
void module_fault_detect_set_temperature4(float newTemperature)
{
    sFaultDetecState.temperature4 = newTemperature;
}

/* Update the hardware error codes */
/* Note: This function is to be used for hardware error detection only */
void module_fault_detect_set_error(errorCode newError)
{
    sFaultDetecState.inputError = newError;
}


///////////////////////////////////// THREAD FUNCTIONS /////////////////////////////////////
/* Thread to manage fault detection module */
void module_fault_detect_task(void *argument)
{
    UNUSED(argument);
    while(1)
    {
        // Update state of fault detection module
        module_fault_detect_update();
        // Wait 500ms between updates
        osDelay(500);
    }
}


/* Module update routine */
void module_fault_detect_update(void)
{

// Modulus type function to make sure deviation of temperature from temperature 4 and 1 is a POSITIVE NUMBER at all times for the condition to work:

float deviation = sFaultDetecState.temperature1 - sFaultDetecState.temperature4 ; //temperature deviation

if( deviation < 0.0 ){ //if negative number then change it to positive

    deviation = -(deviation); //deviation is changed to a positive number

 }

//printf("Deviations: %f\n", deviation); //used to check deviations values to check whether condition is triggered correctly

//Calculating the saturation pressure at the inlet of the compressor:

float satpressure ; //Saturation Pressure
satpressure =  ref_410a_sat_pressure(sFaultDetecState.temperature2 ); //using temperature from temperature of evaporator outlet to calculate saturation pressure
//printf("\n Saturation Pressure (kPA): %f", satpressure); //printing values to check validity of condition in serial command



//Check for errors and push appropriate error codes to the state machine module (SMM):.........................................

if(sFaultDetecState.inputError == FAULT_HEALTHY){ //if no hardware faults then only check the values


        // create local error flag to track faults
        errorCode localErrorFlag  = FAULT_HEALTHY;



        if (sFaultDetecState.Estop == 1){  //if estop is pressed, set fault.

         localErrorFlag = FAULT_ESTOP; // Set ESTOP Fault flag

         }

        if (sFaultDetecState.pressure1 > maxPressureOut){ //pressure of compressor outlet

        localErrorFlag = FAULT_OUTLET_PRESSURE_OVER; //send over pressure fault to the SMM

        }

        if(sFaultDetecState.temperature1 > maxTemperature1Int || sFaultDetecState.temperature4 > maxTemperature1Int){ //condition for more than 80 Celsius.
        //temperature of compressor outlet. Use temperature sensor 4 too to make use of the extra safety from a redundant component.

            sum  = sum + 0.5 ; //timer (Counter, each update is 0.5 seconds)
            //printf("The sum: %f \n", sum); //checking whether is the sum calculating correctly.

                if(sum > 15.0 ){ //greater than 15 seconds continuously then send fault.

                   localErrorFlag =  FAULT_OUTLET_TEMPERATURE_OVER; //Set over temperature fault flag

                 }


        }else{ //not continuously 15 seconds. re-time

         sum = 0.0 ; //re-count from 0.

        }

        if(sFaultDetecState.temperature1 > maxTemperatureOut || sFaultDetecState.temperature4 > maxTemperatureOut ) { //condition for more than 100 Celsius
        //Temperature of compressor outlet. Use temperature sensor 4 too to make use of the extra safety from a redundant component.

        localErrorFlag = FAULT_OUTLET_TEMPERATURE_OVER; //send over temperature fault to the SMM

        }

        if(sFaultDetecState.pressure2 < 800.0){ //pressure of evaporator outlet cannot be lesser than 800kPA

        localErrorFlag = FAULT_UNDER_PRESSURE; // Set under pressure fault flag

        }


        if( sFaultDetecState.pressure2 > satpressure){ //if pressure at sensor 2 is more than saturation pressure, it means that it is in liquid state

        localErrorFlag = FAULT_LIQUID_COMPRESSOR; //Set there is liquid in compressor intake flag

        }

        if( deviation > 5.0 ) { //temperature deviation betweeen temperature sensor 4 and 1


        localErrorFlag = FAULT_TEMPERATURE_DEVIATION; //set temperature deviation flag

        }

        if(sFaultDetecState.temperature3 < minTemperature3){ //temperature of evaporator inlet cannot be below 0 Celsius


        localErrorFlag = FAULT_EVAPERATOR_FREEZING; //set evaporator freezing flag

        }

        if( (sFaultDetecState.temperature1 < 70.0 && sFaultDetecState.temperature4 < 70.0) && localErrorFlag  == FAULT_HEALTHY ){ // using this condition as a temperature checker.
        // If all fault detection is HEALTHY, make sure temperature 1 and 4 is below 70 before clearing.

        //Sending the information to the SMM regarding whether is the temperature for sensor 1 and 4 is both below 70 Celsius:

        module_state_machine_tempchecker(0);  // 0 for can go to healthy if start signal is given in SM_waiting state, 1 for back to fault state.


        }else{ //can go to healthy. In state machine module, it will be the condition in SM_Waiting together with the start signal.


        module_state_machine_tempchecker(1); // 0 for can go to healthy is given in SM_waiting state, 1 for back to fault state.

        }




        module_state_machine_set_fault(localErrorFlag); //set the fault from the flag. (Send to the SMM)


}

else{ // there is hardware fault

module_state_machine_set_fault(sFaultDetecState.inputError); //insert the hardware fault that occur and send to the SMM

}





}

///////////////////////////////////// INITIALISATION FUNCTIONS /////////////////////////////////////
/* Start thread to manage fault detection module */
void module_fault_detect_init(void)
{
    if (!_is_init)
    {
        // Initialise thread
        _fault_detect_thread_id = osThreadNew(module_fault_detect_task, NULL, &_fault_detect_attr);
        _is_init = 1;
        _is_running = 1;
    }
}


///////////////////////////////////// ADDITIONAL FUNCTIONS /////////////////////////////////////
/* start fault detection module */
void module_fault_detect_start(void)
{
    if(!_is_running && _is_init)
    {
        osThreadResume(_fault_detect_thread_id);
        _is_running = 1;
    }
    printf("Fault detection module on\n");
}

/* stop fault detection module */
void module_fault_detect_stop(void)
{
    if(_is_running && _is_init)
    {
        osThreadSuspend(_fault_detect_thread_id);
        _is_running = 0;
    }
    printf("Fault detection module off\n");
}

/* Function to read pointer to internal state structure */
modFaultDetecTypeDef* module_fault_detect_get_state(void)
{
    return &sFaultDetecState;
}

void module_fault_detect_manual_estop(int newEstop)
{
    // Set estop value
    module_fault_detect_set_estop(newEstop);

    // Update the module
    module_fault_detect_update();

    // Retrieve state machine state
    modStateMachineTypeDef* sstateMachineState;
    sstateMachineState = module_state_machine_get_state();

    // Print fault detection state
    printf("Fault detection module output error code: %i\n", sstateMachineState->Error);
}

void module_fault_detect_manual_error(int newError)
{
    // Set estop value
    module_fault_detect_set_error(newError);

    // Update the module
    module_fault_detect_update();

    // Retrieve state machine state
    modStateMachineTypeDef* sstateMachineState;
    sstateMachineState = module_state_machine_get_state();

    // Print fault detection state
    printf("Fault detection module output error code: %i\n", sstateMachineState->Error);
}

void module_fault_detect_manual_pressure1(float newPressure)
{
    // Set estop value
    module_fault_detect_set_pressure1(newPressure);

    // Update the module
    module_fault_detect_update();

    // Retrieve state machine state
    modStateMachineTypeDef* sstateMachineState;
    sstateMachineState = module_state_machine_get_state();

    // Print fault detection state
    printf("Fault detection module output error code: %i\n", sstateMachineState->Error);
}

void module_fault_detect_manual_pressure2(float newPressure)
{
    // Set estop value
    module_fault_detect_set_pressure2(newPressure);

    // Update the module
    module_fault_detect_update();

    // Retrieve state machine state
    modStateMachineTypeDef* sstateMachineState;
    sstateMachineState = module_state_machine_get_state();

    // Print fault detection state
    printf("Fault detection module output error code: %i\n", sstateMachineState->Error);
}

void module_fault_detect_manual_temperature1(float newTemperature)
{
    // Set estop value
    module_fault_detect_set_temperature1(newTemperature);

    // Update the module
    module_fault_detect_update();

    // Retrieve state machine state
    modStateMachineTypeDef* sstateMachineState;
    sstateMachineState = module_state_machine_get_state();

    // Print fault detection state
    printf("Fault detection module output error code: %i\n", sstateMachineState->Error);
}

void module_fault_detect_manual_temperature2(float newTemperature)
{
    // Set estop value
    module_fault_detect_set_temperature2(newTemperature);

    // Update the module
    module_fault_detect_update();

    // Retrieve state machine state
    modStateMachineTypeDef* sstateMachineState;
    sstateMachineState = module_state_machine_get_state();

    // Print fault detection state
    printf("Fault detection module output error code: %i\n", sstateMachineState->Error);
}

void module_fault_detect_manual_temperature3(float newTemperature)
{
    // Set estop value
    module_fault_detect_set_temperature3(newTemperature);

    // Update the module
    module_fault_detect_update();

    // Retrieve state machine state
    modStateMachineTypeDef* sstateMachineState;
    sstateMachineState = module_state_machine_get_state();

    // Print fault detection state
    printf("Fault detection module output error code: %i\n", sstateMachineState->Error);
}

void module_fault_detect_manual_temperature4(float newTemperature)
{
    // Set estop value
    module_fault_detect_set_temperature4(newTemperature);

    // Update the module
    module_fault_detect_update();

    // Retrieve state machine state
    modStateMachineTypeDef* sstateMachineState;
    sstateMachineState = module_state_machine_get_state();

    // Print fault detection state
    printf("Fault detection module output error code: %i\n", sstateMachineState->Error);
}

void module_fault_detect_print_inputs(void)
{
    // Print input values to console
    printf("Estop status (0=healthy, 1=fault): %i\n", sFaultDetecState.Estop);
    printf("Pressure 1 status (kPa): %f\n", sFaultDetecState.pressure1);
    printf("Pressure 2 status (kPa): %f\n", sFaultDetecState.pressure2);
    printf("Temperature 1 status (C): %f\n", sFaultDetecState.temperature1);
    printf("Temperature 2 status (C): %f\n", sFaultDetecState.temperature2);
    printf("Temperature 3 status (C): %f\n", sFaultDetecState.temperature3);
    printf("Temperature 4 status (C): %f\n", sFaultDetecState.temperature4);
    printf("Error code: %i\n", sFaultDetecState.inputError);
}
