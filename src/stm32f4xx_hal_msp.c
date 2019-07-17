#include "stm32f4xx_hal.h"

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
   	/* Timer 2: Initialise the onboard LED for heartbeat on PA5 */
    if(htim->Instance == TIM2)
    {
        /* Enable peripheral clocks */
        __HAL_RCC_TIM2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /* Configure PA5 in output pushpull mode */
        GPIO_InitTypeDef  GPIO_InitStructure;
        GPIO_InitStructure.Pin = GPIO_PIN_5;
        GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStructure.Pull = GPIO_PULLUP;
        GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStructure.Alternate = GPIO_AF1_TIM2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
    }
}