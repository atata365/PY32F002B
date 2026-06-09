/*---------------------------*/
/*    Make A1 radio wave     */
/*---------------------------*/
/* modurate FAMIMA chime */
/* RF signal output from PA0 */

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

/* Global valiables */
volatile bool flag = true;  // switch carrier wave
volatile bool mute = false; // mute flag
volatile uint32_t L = 0;    // SysTick counter

/* Interval data(1/freq.:us) */
const int interval[] = {
    1517,   //E
    1432,   //F
    1351,   //F#
    1276,   //G
    1204,   //G#
    1136,   //A
    1073,   //A#
    1012,   //B
    956,    //C
    902,    //C#
    851,    //D
    804,    //D#
    758,    //E
    716,    //F
    676,    //F#
    638,    //G
    602,    //G#
    568     //A
};

/*---------------------------------------*/
/* excetion process routuine for SysTick */
/* This routine called every 1ms.        */
/*---------------------------------------*/
extern "C" {
    void SysTick_Handler(void) {
        L++;
    }
}

/*---------------*/
/* Return millis */
/*---------------*/
uint32_t millis(void) {
    return L;
}

/*-------------------------*/
/* Blocking delay function */
/*-------------------------*/
void delay(uint32_t Wait_ms) {
    volatile uint32_t end_ms;
    end_ms = millis() + Wait_ms;
    while(millis() < end_ms);
}

/*-----------------------------------*/
/* TIM14 Interrupt Handler (IRQn=14) */
/* switch PWM output state           */
/*-----------------------------------*/
extern "C" {
    void TIM14_IRQHandler(void) {
        ((flag) || (mute)) ? TIM1->CCR1 = 12 : TIM1->CCR1 = 0;
        flag = !flag;   // toggle flag
        /* clear interrupt flag */
        TIM14->SR &= ~TIM_SR_UIF;
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
    SysTick->LOAD = 23999;    // 1ms interval (1kHz)
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
/* Make 1MHz carrier            */
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
    TIM1->CCR1 = 12;// duty 0%(OFF)
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

/*-------------------------------*/
/* Initialize and settings TIM14 */
/* For make AFsignal             */
/*-------------------------------*/
void Init_TIM14(void) {
    /* activate TIM14 */
    RCC->APBENR2 |= RCC_APBENR2_TIM14EN;
    /* prescaler (make counter freq) */
    /* 0..65535(16bit) */
    /* (24000000(24MHz) / 24) = 1MHz */    
    TIM14->PSC = 23;        // setting value is actial value -1
    /* ARR(Auto Reload Resister) = 0..65535(16bit) */ 
    /* 1MHz/1000 = 1kHz */
    TIM14->ARR = 1000 - 1;  // setting value is actial value -1
    /* CCR1 = 0..65535(16bit) */ 
    //TIM14->CCR1 = 100 - 1;// duty 0% -> not use CC1
    /* set PWM mode */
    /* PWM mode 1 , activate preload*/
    TIM14->CCMR1 = (TIM14->CCMR1 & ~TIM_CCMR1_OC1M) | TIM_CCMR1_OC1PE;
    /* activate TIM14 counter */
    TIM14->CR1 = TIM_CR1_CEN;
    /* activate TIM14 updates interrupt */
    TIM14->DIER |= TIM_DIER_UIE;
    /* enable interrupts in NVIC */
    NVIC->ISER[0] |= (1 << TIM14_IRQn);
    /* clear counter */
    TIM14->CNT = 0;
}

/*-------------*/
/* output tone */
/*-------------*/
void tone(int interval,int length) {
    interval == 0 ? mute = true : mute = false;
    TIM14->ARR = interval - 1;  // setting value is actial value -1
    delay(length);
    /* clear counter */
    TIM14->CNT = 0;
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
    Init_GPIO();
    Init_TIM1();
    Init_TIM14();
    Init_SysTick();
    while(1){
        tone(interval[14], 500); // F#
        tone(interval[10], 500); // D
        tone(interval[5], 500); // A
        tone(interval[10], 500); // D
        tone(interval[12], 500); // E
        tone(interval[17], 500); // A
        tone(0, 500); // mute
        tone(interval[12], 500); // E
        tone(interval[14], 500); // F#
        tone(interval[12], 500); // E
        tone(interval[5], 500); // A
        tone(interval[10], 500); // E
        tone(0, 3000); // mute
    }
}