#include "RTE_Components.h"
#include CMSIS_device_header

/* TIM1 Break, Update, Trigger and Commutation Interrupt Handler (IRQn=13) */
extern "C" 
    __attribute__((interrupt)) void TIM1_BRK_UP_TRG_COM_IRQHandler(void) {
    /* clear update interrupt flag */
    TIM1->SR &= ~TIM_SR_UIF;
    /* turn on the LED at PA0 */
    GPIOA->BSRR = GPIO_BSRR_BS0;
}

/* TIM1 Capture Compare Interrupt Handler (IRQn=14) */
extern "C"
    __attribute__((interrupt)) void TIM1_CC_IRQHandler(void) {
    /* clear capture compare interrupt flag */
    TIM1->SR &= ~TIM_SR_CC1IF;
    /* turn off the LED at PA0 */
    GPIOA->BSRR = GPIO_BSRR_BR0;
}

void TIM1init() {
    /* clear counter */
    TIM1->CNT = 0;
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
    TIM1->CCR1 = 249;// duty 5%
    /* set PWM mode */
    /* PWM mode 1 , activate preload*/
    TIM1->CCMR1 = (TIM1->CCMR1 & ~TIM_CCMR1_OC1M)
                | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1   // mode1
                | TIM_CCMR1_OC1PE; // enables preload
    /* activate TIM1 counter*/
    TIM1->CR1 = TIM_CR1_CEN;
    /* activate TIM1 BRK_UP_TRG_COM and CC1 interrupt */
    TIM1->DIER |= TIM_DIER_UIE | TIM_DIER_CC1IE;
    /* enable interrupts in NVIC */
    NVIC->ISER[0] = (1 << TIM1_BRK_UP_TRG_COM_IRQn) | (1 << TIM1_CC_IRQn);
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
    /* initialize TIM1*/
    TIM1init();
    while(1){
    }
}