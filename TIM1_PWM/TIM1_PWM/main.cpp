#include "RTE_Components.h"
#include CMSIS_device_header

void TIM1init() {
    /* activate TIM1 */
    RCC->APBENR2 |= RCC_APBENR2_TIM1EN;
    /* prescaler (make counter freq) */
    /* 0..65535(16bit) */
    /* (24000000(24MHz) / 4800) = 5kHz */    
    TIM1->PSC = 4799;
    /* ARR(Auto Reload Resister) = 0..65535(16bit) */ 
    /* 5kHz/5000=1Hz */
    TIM1->ARR = TIM_ARR_ARR_Msk & 4999;
    /* CCR1 = 0..65535(16bit) */ 
//    TIM1->CCR1 = 2499;// duty 50%
    TIM1->CCR1 = 2499;// duty 50%
    /* set PWM mode */
    /* PWM mode 1 , activate preload*/
    TIM1->CCMR1 = (TIM1->CCMR1 & ~TIM_CCMR1_OC1M)
                                | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1   // mode1
                                | TIM_CCMR1_OC1PE; // enables preload
    /* activate CC1E PWM output */
    TIM1->CCER |= TIM_CCER_CC1E;
    /* turn on the PWM output */
    TIM1->BDTR |= TIM_BDTR_MOE;
    /* activate TIM1 counter*/
    TIM1->CR1 = TIM_CR1_CEN;
}

int main() {
    /* select clock source=HSI */
    RCC->CR = RCC_CR_HSION;
    /* wait for HSI stabirised */
    while(!(RCC->CR & RCC_CR_HSIRDY));
    /* activate GPIOA */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    /* set PA0 to alternate function mode, PA1 to General purpose output mode */
    GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1))
                                                      | GPIO_MODER_MODE0_1                   
                                                      | GPIO_MODER_MODE1_0;
    /* set PA1 to push-pull mode */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT1;
    /* set PA0 alternate function AF2(TIM1_CH1)(set 0b010) */
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~GPIO_AFRL_AFSEL0_Msk) | GPIO_AFRL_AFSEL0_1;
    /* turn on the LED(PA1) */
    GPIOA->BSRR = GPIO_BSRR_BS1;
    /* initialize TIM1*/
    TIM1init();
    while(1){
    }
}