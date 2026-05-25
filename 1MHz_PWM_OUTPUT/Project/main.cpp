/*-------------------------------------------*/
/*      1MHz PWM ouput (use TIM1,AF2)        */
/*-------------------------------------------*/
/*       PWM output and GPIO relation        */
/* TIMM#   CH   GPIO  PIN#  OTHER_FUNCTION   */
/*   1      1    PA0   5                     */
/*   1      2    PA1   6                     */
/*   1      3    PA4   9                     */
/*   1      4    PA2   7    SWC(SWDIO CLOCK) */
/* pin numbers are based on sop14 packge.    */
/*-------------------------------------------*/
#include "RTE_Components.h"
#include CMSIS_device_header

/*------------------------------*/
/* Initialize and settings TIM1 */
/*------------------------------*/
void Init_TIM1(void) {
    /* activate TIM1 */
    RCC->APBENR2 |= RCC_APBENR2_TIM1EN;
    /* prescaler (make counter freq) */
    /* 0..65535(16bit) */
    /* set prescaler as 0 (disable prescaling=24MHz) */    
    TIM1->PSC = 0;
    /* ARR(Auto Reload Resister) = 0..65535(16bit) */ 
    /* 24MHzkHz/24=1MHz (setting value is actial value -1) */
    TIM1->ARR = TIM_ARR_ARR_Msk & 23;
    /* CCR1 = 0..65535(16bit) */
    /* set CCR1 value for 50% duty cycle (24/2=12)*/
    TIM1->CCR1 = 12;// duty 50%
    /* set PWM mode */
    /* PWM mode 1 , activate preload*/
    TIM1->CCMR1 = (TIM1->CCMR1 & ~TIM_CCMR1_OC1M)
                                | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1   // pwm mode1
                                | TIM_CCMR1_OC1PE; // enables preload
    /* activate CC1E PWM output */
    TIM1->CCER |= TIM_CCER_CC1E;
    /* turn on the PWM output (MOE:Main Output Enable) */
    TIM1->BDTR = TIM_BDTR_MOE; // main output enable
    /* reset(0) counter */
    TIM1->CNT = 0;
    /* activate TIM1 counter*/
    TIM1->CR1 |= TIM_CR1_CEN;
}
/*-----------------------*/
/*    Initialize GPIO    */
/* GPIO A0 as PWM output */
/*-----------------------*/
void Init_GPIO(void) {
    /* activate GPIOA (Clock feed to GPIOA module) */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    /* set PA0 to alternate function mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE0) | GPIO_MODER_MODE0_1;
    /* set PA0 alternate function AF2(TIM1_CH1:set 0b010) */
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~GPIO_AFRL_AFSEL0_Msk) | GPIO_AFRL_AFSEL0_1;
}

int main() {
    /* select clock source=HSI */
    RCC->CR = RCC_CR_HSION;
    /* wait for HSI stabirised */
    while(!(RCC->CR & RCC_CR_HSIRDY));
    Init_GPIO();
    Init_TIM1();
    while(1){
    }
}