/* test program for 4digit LED display with TM1637*/
/* used GPIOs PA0(clock) and PA1(data) */
/* PIN assign PA0:#5, PA1:#6 (SOP14 package) */
#include "RTE_Components.h"
#include CMSIS_device_header

/* counter, incremented every counter reset */
volatile uint32_t COUNT;

void TIM14init() {
    /* clear counter */
    TIM14->CNT = 0;
    /* activate TIM14 */
    RCC->APBENR2 |= RCC_APBENR2_TIM14EN;
    /* prescaler (make counter freq) */
    /* 0..65535(16bit) */
    /* (24000000(24MHz) / 1) = 24MHz */
    TIM14->PSC = 0;
}

void GPIOinit() {
    /* activate GPIOA */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    /* set PA0 snd PA1 to General purpose output mode */
    GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1))
     | GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0;
    /* set PA0 and PA1 to push-pull mode */
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1);
    /* set PA0 and PA1 to high */
    GPIOA->BSRR |= GPIO_BSRR_BS0 | GPIO_BSRR_BS1;
}

void wait(uint16_t count) {
    if( TIM14->CNT > 0x7fff) {
        /* counter overflow, reset counter */
        TIM14->CNT = 0;
    }
    uint16_t start_count = TIM14->CNT;
    while((TIM14->CNT - start_count) < count);
}

int main() {
    /* select clock source=HSI */
    RCC->CR = RCC_CR_HSION;
    /* wait for HSI stabirised */
    while(!(RCC->CR & RCC_CR_HSIRDY));
    /* initialize GPIO */
    GPIOinit();
    /* initialize TIM14*/
    TIM14init();
    int i = 0,j = 0,k = 0,l = 0;

    while(1){
        GPIOA->BSRR |= GPIO_BSRR_BS0 | GPIO_BSRR_BS1;
        wait(239);
        GPIOA->BSRR |= GPIO_BSRR_BR0 | GPIO_BSRR_BR1;
        wait(239);
    }
}   