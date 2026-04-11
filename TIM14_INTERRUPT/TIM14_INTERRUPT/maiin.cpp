#include "RTE_Components.h"
#include CMSIS_device_header

/* TIM14 Interrupt Handler (IRQn=14) */
extern "C" 
    __attribute__((interrupt)) void TIM14_IRQHandler(void) {

    /* check if the interrupt is from the update event */
    if (TIM14->SR & TIM_SR_UIF) {
        /* turn on the LED at PA0 */
        GPIOA->BSRR = GPIO_BSRR_BS0;
    } else if (TIM14->SR & TIM_SR_CC1IF) {
        /* clear capture compare interrupt flag */
        TIM14->SR &= ~TIM_SR_CC1IF;
        /* turn off the LED at PA0 */
        GPIOA->BSRR = GPIO_BSRR_BR0;
    }
    /* clear update interrupt flag */
    TIM14->SR &= ~TIM_SR_UIF;
}

void TIM14init() {
    /* clear counter */
    TIM14->CNT = 0;
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
    /* activate TIM14 counter*/
    TIM14->CR1 = TIM_CR1_CEN;
    /* activate TIM14 BRK_UP_TRG_COM and CC1 interrupt */
    TIM14->DIER |= TIM_DIER_UIE | TIM_DIER_CC1IE;
    /* enable interrupts in NVIC */
    NVIC->ISER[0] |= (1 << TIM14_IRQn);
}

int main() {
    /* select clock source=HSI */
    RCC->CR = RCC_CR_HSION;
    /* wait for HSI stabirised */
    while(!(RCC->CR & RCC_CR_HSIRDY));
    /* activate GPIOA */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    /* set PA0 to General purpose output mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE0) | GPIO_MODER_MODE0_0;
    /* set PA0 to push-pull mode */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT0;
    /* initialize TIM14*/
    TIM14init();
    while(1){
    }
}