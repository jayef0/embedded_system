#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

#include "mod_inputs.h"
#include "mod_fault_detection.h"

/* Misc. module specific varables */
ADC_HandleTypeDef hadc1;
ADC_ChannelConfTypeDef sConfigADC;
static uint8_t _is_init = 0;
static uint8_t _is_running = 0;

static uint8_t _was_pressed = 0;    // Sticky flag for Interrupt Routine (SELF-ADDED)

/* Structures defined for the fault detection thread */
static osThreadId_t _inputs_thread_id;
static osThreadAttr_t _inputs_attr =
{
    .name = "inputsModule",
    .priority = osPriorityHigh //Set initial thread priority
};

///////////////////////////////////// THREAD FUNCTIONS /////////////////////////////////////
/* Task to manage fault detection module */
void module_inputs_task(void *argument)
{
    UNUSED(argument);
    while(1)
    {
        // Update state of fault detection module
        module_inputs_update();
        // Wait 500ms between updates
        osDelay(500);
    }
}

/* Update sensor measuments and push to fault detection module */
void module_inputs_update(void)
{
    // create local error flag to track faults
    errorCode localErrorFlag = FAULT_HEALTHY;

    //Update  sensor measurements and push them to the fault detection module

    //Start the ADC
    HAL_ADC_Start(&hadc1);

    //Poll for ADC values:
    HAL_ADC_PollForConversion(&hadc1,10);

    //Setting up variables for Pressure Sensor 1................................................:

    //used by all inputs:

    float Vp;  //voltage
    float Ip;  //current
    float Vadc_max = 3.3 ; // maximum voltage used.
    float R = 0.125;   // resistor

    //specifically for pressure sensor 1:

    float Pmin = 0 ;  // minimum pressure 1
    float Pmax = 5000; //maximum pressure 1
    float Pressure1;  //Pressure Sensor 1
    float P_ADC = HAL_ADC_GetValue(&hadc1);  //getting the values from the ADC

    Vp  = (Vadc_max/4095.0)*P_ADC; // mapping the ADC scale to the voltage scale.
    Ip  = Vp/R;                    // calculating the current

    if( Ip < 4 || Ip >= 20.5){  //if the current is less than 4mA or more than 20.5mA, it's a fault

         //set the flag
         localErrorFlag =   FAULT_PRESSURE1_SENSOR; //hardware fault for pressure sensor 1

    }

    Pressure1 =  Pmin + ((Ip -4.0)/(20.0-4.0))*(Pmax -  Pmin);

    module_fault_detect_set_pressure1(Pressure1);  //send the pressure sensor 1 value to the fault detection module

    //Setting up variables for Temperature Sensor 1..............................................:

    float Tmax = 200; //max temperature 1
    float Tmin = 0;   //min temperature 1
    float Temp1; //Temperature Sensor 1

    HAL_ADC_PollForConversion(&hadc1,10); // waits for EOC flag to be set (Rank2)

    float T_ADC = HAL_ADC_GetValue(&hadc1);//getting the values from the ADC

    Vp  = (Vadc_max/4095.0)* T_ADC;// mapping the ADC scale to the voltage scale.
    Ip  = Vp/R; // calculating the current


    if( Ip < 4 || Ip >= 20.5){
        //set flag
        localErrorFlag =   FAULT_TEMPERATURE1_SENSOR;   //hardware fault for temperature sensor 1

    }

    Temp1 =  Tmin + ((Ip -4.0)/(20.0-4.0))*(Tmax -  Tmin); //calculate the temperature
    module_fault_detect_set_temperature1(Temp1); //send temperature values to Fault Mode detection module


//Setting up variables for Pressure Sensor 2................................................:

    float Pmin2 = 0 ;  // minimum pressure 2
    float Pmax2 = 2500; //maximum pressure 2
    float Pressure2;  //Pressure Sensor 2

    HAL_ADC_PollForConversion(&hadc1,10); // Re-poll (Sequencer increases to Rank 3)

    float P_ADC_2 = HAL_ADC_GetValue(&hadc1); //getting the values from the ADC

    Vp  = (Vadc_max/4095.0)* P_ADC_2;// mapping the ADC scale to the voltage scale.
    Ip  = Vp/R; // calculating the current


    if( Ip < 4 || Ip >= 20.5){
        //set the flag
        localErrorFlag =   FAULT_PRESSURE2_SENSOR; //hardware fault for pressure sensor 2
    }

    Pressure2 =  Pmin2 + ((Ip -4.0)/(20.0-4.0))*(Pmax2 -  Pmin2);
    module_fault_detect_set_pressure2(Pressure2);  //send the pressure sensor 2 value to the fault detection module

 //Setting up variables for Temperature Sensor 2..........................................:

 //Note: temperature of sensor 2 is the outlet of the evaporator which is the inlet for the compressor

    float Tmax2 = 100; //max temperature 2
    float Tmin2 = -50;   //min temperature 2
    float Temp2; //Temperature Sensor 2

    HAL_ADC_PollForConversion(&hadc1,10); // Re-poll (Sequencer increases to Rank 4)

    float T_ADC_2 = HAL_ADC_GetValue(&hadc1); //get ADC values

    Vp  = (Vadc_max/4095.0)* T_ADC_2;// mapping the ADC scale to the voltage scale.
    Ip  = Vp/R; // calculating the current



    if( Ip < 4 || Ip >= 20.5){
        //set flag
        localErrorFlag =   FAULT_TEMPERATURE2_SENSOR;   //hardware fault for temperature sensor 2

    }

    Temp2 =  Tmin2 + ((Ip -4.0)/(20.0-4.0))*(Tmax2 -  Tmin2); //calculate the temperature
    module_fault_detect_set_temperature2(Temp2); //send temperature values to Fault Mode detection module

//Setting up variables for Temperature Sensor 3..........................................:

    float Tmax3 = 100; //max temperature 3
    float Tmin3 = -50;   //min temperature 3
    float Temp3; //Temperature Sensor 3

    HAL_ADC_PollForConversion(&hadc1,10); // waits for EOC flag to be set (Rank5)

    float T_ADC_3 = HAL_ADC_GetValue(&hadc1); //get ADC values

    Vp  = (Vadc_max/4095.0)* T_ADC_3; //mapping the ADC scale to the voltage scale
    Ip  = Vp/R; //calculating the current


    if( Ip < 4 || Ip >= 20.5){
        //set flag
        localErrorFlag =   FAULT_TEMPERATURE3_SENSOR;   //hardware fault for temperature sensor 3

    }

    Temp3 =  Tmin3 + ((Ip -4.0)/(20.0-4.0))*(Tmax3 -  Tmin3); //calculate the temperature
    module_fault_detect_set_temperature3(Temp3); //send temperature values to Fault Mode detection module

//Setting up variables for Temperature Sensor 4 (Redundant Component)..........................................:

    float Tmax4 = 200; //max temperature 4
    float Tmin4 = 0;   //min temperature 4
    float Temp4; //Temperature Sensor 4

    HAL_ADC_PollForConversion(&hadc1,10); // waits for EOC flag to be set (Rank6)

    float T_ADC_4 = HAL_ADC_GetValue(&hadc1);

    Vp  = (Vadc_max/4095.0)* T_ADC_4; //mapping the ADC scale to the voltage scale
    Ip  = Vp/R; //calculating the current


    if( Ip < 4 || Ip >= 20.5){
        //set flag
        localErrorFlag =   FAULT_TEMPERATURE4_SENSOR;   //hardware fault for temperature sensor 4

    }

    Temp4 =  Tmin4 + ((Ip -4.0)/(20.0-4.0))*(Tmax4 -  Tmin4); //calculate the temperature
    module_fault_detect_set_temperature4(Temp4); //send temperature values to Fault Mode detection module


//After going through all the inputs..........................................................................:

    //send to the Fault Detection Module if any hardware faults are found or else flag will remain healthy:
    module_fault_detect_set_error(localErrorFlag);

    //Stop the ADC
    HAL_ADC_Stop(&hadc1);



//Read Estop state and push states to fault detection module........................................:

// Note: Only when ESTOP button is release, the button will then be considered Healthy

     //healthy = 0 (not pressed), faulty = 1 (pressed) :
    module_fault_detect_set_estop(_was_pressed); // Interrupt routine is used to detect the press (on falling edge)

    if (_was_pressed == 1 && HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_9) == 1){ //was pressed (detect with the interrupt)
    // Reading back pin to ensure that HOLDING of the pin will not cause it to revert to healthy, only when it is released that the pin will be high.

    module_fault_detect_set_estop(_was_pressed); //healthy = 0 (not pressed), faulty = 1 (pressed)
    _was_pressed = 0 ;  //set back to 0

    }








}


///////////////////////////////////// INITIALISATION FUNCTIONS /////////////////////////////////////
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
  //Initialize low level resources for ADC

     __HAL_RCC_ADC1_CLK_ENABLE(); // enabling the ADC clock
    __HAL_RCC_GPIOA_CLK_ENABLE(); // enabling the GPIO clock

    GPIO_InitTypeDef  GPIO_InitStructure; // initialising the structure for the pins.

    GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 |  GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7; //pins of sensors
    GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;       //analog values
    GPIO_InitStructure.Pull = GPIO_NOPULL;            //No pull down or up resistors will be used.


// Initialising for PB0 pin for redundant component:

    GPIO_InitTypeDef  GPIO_InitStructure1;

    GPIO_InitStructure1.Pin  = GPIO_PIN_0; // Pin 0
    GPIO_InitStructure1.Mode = GPIO_MODE_ANALOG; //analog values
    GPIO_InitStructure1.Pull = GPIO_NOPULL; //no pull down or up resistors will be used

    __HAL_RCC_GPIOB_CLK_ENABLE() ; // Enable clock B

    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure1);   // initialising the analog pin for PB0
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);    // initialising the analog pins as for A pin series

}

/* initialise inputs module as a thread */
void module_inputs_init(void)
{
    // initialise hardware
    module_inputs_configure_hardware();
    // create thread for kernel to manage
    _inputs_thread_id = osThreadNew(module_inputs_task, NULL, &_inputs_attr);
    _is_init = 1;
    _is_running = 1;
}

/* configure hardware for inputs module */
void module_inputs_configure_hardware(void)
{

    if (!_is_init)
    {


    // Initialize inputs related hardware

    // Configure ADC1 instance:

        hadc1.Instance = ADC1;
        hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2; //div 2 prescaler
        hadc1.Init.Resolution = ADC_RESOLUTION_12B; // 12-bit resolution
        hadc1.Init.DataAlign  = ADC_DATAALIGN_RIGHT; // Right Data Aligned
        hadc1.Init.ScanConvMode = ENABLE;    // Scan conversion mode Enabled
        hadc1.Init.ContinuousConvMode = DISABLE; //Continuous conversion mode disabled
        hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV; //end of conversion flag seelcted for single conversion


    // Configure the ADC:

    // First ADC channel (Rank 1):................................................................

        sConfigADC.Channel = ADC_CHANNEL_0; //Channel 0
        sConfigADC.Rank = 1 ; //Rank 1
        sConfigADC.SamplingTime = ADC_SAMPLETIME_480CYCLES; // 480 Cycle sample of time
        sConfigADC.Offset = 0;  //offset of 0



        HAL_ADC_Init(&hadc1); // Initialise ADC CH0
        HAL_ADC_ConfigChannel(&hadc1,&sConfigADC); // Configure the ADC channel 0


    //Next Channel ADC (Next Rank 2):..............................................................

        sConfigADC.Channel = ADC_CHANNEL_1; //Channel 1
        sConfigADC.Rank = 2 ; // Rank 2
        sConfigADC.SamplingTime = ADC_SAMPLETIME_480CYCLES; // 480 Cycle sample of time
        sConfigADC.Offset = 0;  //offset of 0

        HAL_ADC_ConfigChannel(&hadc1,&sConfigADC); // Configure the ADC channel 1\

    //Next Channel ADC (Next Rank 3):..............................................................

        sConfigADC.Channel = ADC_CHANNEL_4; //Channel 4
        sConfigADC.Rank = 3; // Rank 3
        sConfigADC.SamplingTime = ADC_SAMPLETIME_480CYCLES; // 480 Cycle sample of time
        sConfigADC.Offset = 0;  //offset of 0

        HAL_ADC_ConfigChannel(&hadc1,&sConfigADC); // Configure the ADC channel 1

    //Next Channel ADC (Next Rank 4):..............................................................

        sConfigADC.Channel = ADC_CHANNEL_6; //Channel 6
        sConfigADC.Rank = 4; // Rank 4
        sConfigADC.SamplingTime = ADC_SAMPLETIME_480CYCLES; // 480 Cycle sample of time
        sConfigADC.Offset = 0;  //offset of 0

        HAL_ADC_ConfigChannel(&hadc1,&sConfigADC); // Configure the ADC channel 1


    //Next Channel ADC (Next Rank 5):..............................................................

        sConfigADC.Channel = ADC_CHANNEL_7; //Channel 7
        sConfigADC.Rank = 5; // Rank 5
        sConfigADC.SamplingTime = ADC_SAMPLETIME_480CYCLES; // 480 Cycle sample of time
        sConfigADC.Offset = 0;  //offset of 0

        HAL_ADC_ConfigChannel(&hadc1,&sConfigADC); // Configure the ADC channel 1

    //Next Channel ADC (Next Rank 6):..............................................................

        sConfigADC.Channel = ADC_CHANNEL_8; //Channel 8
        sConfigADC.Rank = 6; // Rank 6
        sConfigADC.SamplingTime = ADC_SAMPLETIME_480CYCLES; // 480 Cycle sample of time
        sConfigADC.Offset = 0;  //offset of 0

        HAL_ADC_ConfigChannel(&hadc1,&sConfigADC); // Configure the ADC channel 1


    // Configure estop pin:......................................................................

        //Initializing structure of ESTOP PIN

        GPIO_InitTypeDef  GPIO_InitStructure;

        GPIO_InitStructure.Pin  = GPIO_PIN_9;
        GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING; // Interrupt on falling edge (PULL-UP means it will be high all the time, that is why interrupt on falling)
        GPIO_InitStructure.Pull = GPIO_PULLUP; // use PULL-UP resistor (high unless button is pressed)
        GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; //high frequency

        __HAL_RCC_GPIOC_CLK_ENABLE() ; //enabling clock
        HAL_GPIO_Init(GPIOC, &GPIO_InitStructure); //initialising the pin

        // enable the interrupt
        HAL_NVIC_SetPriority(EXTI9_5_IRQn , 0x0F, 0x0F); // setting up priority for interrupt routine
        HAL_NVIC_EnableIRQ(EXTI9_5_IRQn); //enabling interrupt handler



    }


}

//Self-function to handle interrupt routine:

void EXTI9_5_IRQHandler(void)
{
   _was_pressed = 1; //button was pressed

   HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9); //exit interrupt routine

}



///////////////////////////////////// ADDITIONAL FUNCTIONS /////////////////////////////////////
/* start inputs module */
void module_inputs_start(void)
{
    if(!_is_running && _is_init)
    {
        osThreadResume(_inputs_thread_id);
        _is_running = 1;
    }
    printf("Inputs module on\n");
}

/* stop inputs module */
void module_inputs_stop(void)
{
    if(_is_running && _is_init)
    {
        osThreadSuspend(_inputs_thread_id);
        _is_running = 0;
    }
    printf("Inputs module off\n");
}
