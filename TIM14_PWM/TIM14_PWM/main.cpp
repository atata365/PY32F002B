#include "RTE_Components.h"
#include CMSIS_device_header

void TIM14init() {
    /* activate TIM14 */
    RCC->APBENR2 |= RCC_APBENR2_TIM14EN;
    /* prescaler (make counter freq) */
    /* 0..65535(16bit) */
    /* (24000000(24MHz) / 4800) = 5kHz */    
    TIM14->PSC = 4799;
    /* ARR(Auto Reload Resister) = 0..65535(16bit) */ 
    /* 5kHz/5000=1Hz */
    TIM14->ARR = TIM_ARR_ARR_Msk & 4999;
    /* CCR1 = 0..65535(16bit) */ 
    TIM14->CCR1 = 2499;// duty 50%
    /* set PWM mode */
    /* PWM mode 1 , activate preload*/
    TIM14->CCMR1 = (TIM14->CCMR1 & ~TIM_CCMR1_OC1M)
                                | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1   // mode1
                                | TIM_CCMR1_OC1PE; // enables preload
    /* activate CC1E PWM output */
    TIM14->CCER |= TIM_CCER_CC1E;
    /* turn on the PWM output */
    TIM14->BDTR |= TIM_BDTR_MOE;
    /* activate TIM14 counter*/
    TIM14->CR1 = TIM_CR1_CEN;
}

int main() {
    /* select clock source=HSI */
    RCC->CR = RCC_CR_HSION;
    /* wait for HSI stabirised */
    while(!(RCC->CR & RCC_CR_HSIRDY));
    /* activate GPIOA */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    /* set PA4 to alternate function mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE4)
					| GPIO_MODER_MODE4_1; // alternate function mode
    /* set PA4 to push-pull mode */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT4;
    /* set PA4 alternate function AF5(TIM14_CH1)(set 0b101) */
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~GPIO_AFRL_AFSEL4_Msk)
					| GPIO_AFRL_AFSEL4_2 | GPIO_AFRL_AFSEL4_0;
    /* initialize TIM14*/
    TIM14init();
    while(1){
    }
}