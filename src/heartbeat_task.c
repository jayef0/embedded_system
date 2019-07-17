#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

#include "heartbeat_task.h"

#define TIMERPERIOD 500

static TIM_HandleTypeDef   _htim2;
static TIM_OC_InitTypeDef  _sConfigPWM;
static osTimerId_t _heartbeatTimerID;
static osTimerAttr_t _heartbeatTimerAttr = { .name = "heartbeat" };

static void _heartbeat_update(void *arg);

static uint8_t _is_running = 0;
static uint8_t _is_init = 0;

void heartbeat_task_init(void)
{
    // if (!_is_init)
    // {
        /* Configure Timer2 to generate single PWM output on GPIOA5 */
        /* Set timer, prescaler, mode, period and clkdiv */
        _htim2.Instance = TIM2;
        _htim2.Init.Prescaler = 1;
        _htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
        _htim2.Init.Period = TIMERPERIOD;
        _htim2.Init.ClockDivision = 0;

        _sConfigPWM.OCMode = TIM_OCMODE_PWM1;
        _sConfigPWM.Pulse = 0;
        _sConfigPWM.OCPolarity = TIM_OCPOLARITY_HIGH;
        _sConfigPWM.OCFastMode = TIM_OCFAST_DISABLE;

        HAL_TIM_PWM_Init(&_htim2);
        HAL_TIM_PWM_ConfigChannel(&_htim2, &_sConfigPWM, TIM_CHANNEL_1);

        /* Enable clock for Timer2 */
        HAL_TIM_PWM_Start(&_htim2, TIM_CHANNEL_1);

        // CMSIS-RTOS API v2 Timer Documentation: https://www.keil.com/pack/doc/CMSIS/RTOS2/html/group__CMSIS__RTOS__TimerMgmt.html
        _heartbeatTimerID = osTimerNew(_heartbeat_update, osTimerPeriodic, (void *)0, &_heartbeatTimerAttr);   // Create the timer in the OS as a periodic with null input argument

    //     _is_init = 1;
    // }

    heartbeat_task_start();
}

void heartbeat_task_start(void)
{
    if (!_is_running)
    {
        osTimerStart(_heartbeatTimerID, 10);                        //Start the timer at 5ms overflow
        _is_running = 1;
    }
}

void heartbeat_task_stop(void)
{
    if (_is_running)
    {
        osTimerStop(_heartbeatTimerID);
        _is_running = 0;
    }
}

uint8_t heartbeat_task_is_running(void)
{
    return _is_running;
}

void _heartbeat_update(void *arg)
{
    static uint8_t  heartbeat_flag  = 0;
    static int      brightness      = 0;
    // Choose whether to increment/decrement
    if(brightness == TIMERPERIOD)
    {
        heartbeat_flag = 0;
    }
    else if(brightness == 0)
    {
        heartbeat_flag = 1;
    }
    // Increment/decrement the brightness of the onboard LED
    if(heartbeat_flag)
    {
        __HAL_TIM_SET_COMPARE(&_htim2, TIM_CHANNEL_1, brightness+=5);
    }
    else
    {
        __HAL_TIM_SET_COMPARE(&_htim2, TIM_CHANNEL_1, brightness-=5);
    }
}

