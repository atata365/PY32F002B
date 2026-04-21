/* 500kHz square wave output from GPIO PA0 */
/* used GPIO PA0(square wave) */
/* PIN assign PA0:#5(SOP14 package) */
#include "RTE_Components.h"
#include CMSIS_device_header

void GPIOinit() {
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN; // enable clock for GPIOA
    GPIOA->MODER &= ~GPIO_MODER_MODE0; // clear mode bits for PA0
    GPIOA->MODER |= GPIO_MODER_MODE0_0; // set PA0 as output
}

/* wait 100ns */
void wait(void) {
    __nop(); // simple delay loop, adjust as needed for timing
    __nop();
    __nop();
    __nop();
    __nop();
    __nop();
    __nop();
    __nop();
    __nop();
    __nop();
    __nop();
}

int main() {
    GPIOinit();
    while(1){
        wait(); // wait for 100ns
        GPIOA->BSRR = GPIO_BSRR_BS0; // set PA0 high
        wait(); // wait for 100ns
        GPIOA->BSRR = GPIO_BSRR_BR0; // set PA0 low
    }
}   