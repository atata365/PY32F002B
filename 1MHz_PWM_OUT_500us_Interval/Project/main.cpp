/*-------------------------------------------*/
/*  　    1MHz PWM intermittent output  　   */
/*            500us output interval          */
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

volatile int flag = 0;
/*---------------------------------------*/
/* excetion process routuine for SysTick */
/* This routine called every 1ms.        */
/*---------------------------------------*/
extern "C" {
    void SysTick_Handler(void) {
        if (flag) TIM1->CCR1 = 12;  // ON (duty 50%) 
        else TIM1->CCR1 = 0;  // OFF (duty 0%)
        flag ^= 1;
    }
}

/*-------------------------------------*/
/* initSysTick                         */
/* start SysTick interrupt and counter */
/*-------------------------------------*/
void Init_SysTick(void) {
    /* load value (sys_clk(24MHz) / 2kHz(2000) = 24000000/2000 -> 12000 */
    /* setting value is actial value -1 */
    /* 11999 = (24000000 / 2000) - 1 */
    /* max 24bit = 16777215 = 0.699Hz */
    SysTick->LOAD = 11999;
    /* clear current counter value */
    SysTick->VAL  = 0;
    /* set clock_source,exception_request,enables_counter */
    /* clock source: 0b1xx = processor clock* */
    /*               0b0xx = external clock   */
    /* exception(simuler to interrupt): 0b1x = enable* */
    /*                                  0b0x = disable */
    /* enables counter: 0b1 = enables counter* */
    /*                  0b0 = disable counter  */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;
}
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
    /* 24MHz/24=1MHz (setting value is actial value -1) */
    TIM1->ARR = TIM_ARR_ARR_Msk & 23;
    /* CCR1 = 0..65535(16bit) */
    /* This value switched 12/0 in SysTick_Handler */
    TIM1->CCR1 = 0;// duty 0%(OFF)
    /* set PWM mode */
    /* PWM mode 1 , activate preload*/
    TIM1->CCMR1 = (TIM1->CCMR1 & ~TIM_CCMR1_OC1M)
                                | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 // pwm mode1
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
    RCC->CR |= RCC_CR_HSION;
    /* wait for HSI stabirised */
    while(!(RCC->CR & RCC_CR_HSIRDY));
    Init_GPIO();
    Init_TIM1();
    Init_SysTick();
    while(1){
    }
}