/* test program for 8digit LED display with 74HC595x2 */
/* used GPIOs PB0(DIO), PB1(CLK), PB2(LATCH) */
/* PIN assign PB0:#4, PB1:#3, PB2:#2 (SOP14 package) */
#include "RTE_Components.h"
#include CMSIS_device_header

/* activate GPIO and set mode */
/* set PB0,PB1,PB2 to push-pull output mode */
void GPIOinit(){
    /* activate GPIOB */
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    /* set PB0, PB1, PB2 to general purpose output mode */
    GPIOB->MODER = (GPIOB->MODER & ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2))
                 | GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0 | GPIO_MODER_MODE2_0;
    /* set PB0, PB1, PB2 to push-pull mode */
    GPIOB->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1 | GPIO_OTYPER_OT2);
    /* set PB0, PB1, PB2 to low */
    GPIOB->BSRR |= GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2;
}

int main() {
    /* select clock source=HSI */
    RCC->CR = RCC_CR_HSION;
    /* wait for HSI stabirised */
    while(!(RCC->CR & RCC_CR_HSIRDY));
    /* initialize GPIO */
    GPIOinit();

    while(1){
        /* set(HIGH) PB0(DATA) */
        GPIOB->BSRR |= GPIO_BSRR_BS0;
        /* set(HIGH) PB1(CLK) */
        GPIOB->BSRR |= GPIO_BSRR_BS1;
        /* reset(low) PB0(DATA) */
        GPIOB->BSRR |= GPIO_BSRR_BS0;
        /* reset(low) PB1(CLK) */
        GPIOB->BSRR |= GPIO_BSRR_BR1;
    }
}   