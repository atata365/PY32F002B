#include "RTE_Components.h"
#include CMSIS_device_header

/* voratile resister for sys tick count */
volatile uint32_t L;

/*---------------------------------------*/
/* excetion process routuine for SysTick */
/* This routine called every 1ms.        */
/*---------------------------------------*/
extern "C" {
    void SysTick_Handler(void) {
        L++;
    }
}

/*-------------------------------------*/
/* initSysTick                         */
/* start SysTick interrupt and counter */
/*-------------------------------------*/
void initSysTick(void) {
    /* load value (sys_clk(24MHz) / 1000(1ms)) - 1 */
    /* 23999 = (24000000 / 1000) - 1 */
    /* max 24bit = 16777215 = 0.699Hz */
    SysTick->LOAD = SysTick_LOAD_RELOAD_Msk & 23999;
    /* clear current counter value */
    SysTick->VAL  = 0;
    /* clear millis */
    L = 0;
    /* set clock_source,exception_request,enables_counter */
    /* clock source: 0b1xx = processor clock* */
    /*               0b0xx = external clock   */
    /* exception: 0b1x = enable* */
    /*            0b0x = disable */
    /* enables counter: 0b1 = enables counter* */
    /*                  0b0 = disable counter  */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;
}

uint32_t millis(void) {
    return L;
}

void delay(uint32_t Wait_ms) {
    volatile uint32_t end_ms;
    end_ms = millis() + Wait_ms;
    while(millis() < end_ms);
}

int main() {
    /* activate GPIOA */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    /* set PA0 to General purpose output mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE0) | GPIO_MODER_MODE0_0;
    /* set PA0 to push-pull mode */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT0;
    /* start SysTick */
    initSysTick();
    while(1){
        /* turn on the LED at PA0 */
        GPIOA->BSRR |= GPIO_BSRR_BS0;
        delay(50);
        /* turn off the LED at PA0 */
        GPIOA->BSRR |= GPIO_BSRR_BR0;
        delay(800);
        /* turn on the LED at PA0 */
        GPIOA->BSRR |= GPIO_BSRR_BS0;
        delay(50);
        /* turn off the LED at PA0 */
        GPIOA->BSRR |= GPIO_BSRR_BR0;
        delay(200);
        /* turn on the LED at PA0 */
        GPIOA->BSRR |= GPIO_BSRR_BS0;
        delay(50);
        /* turn off the LED at PA0 */
        GPIOA->BSRR |= GPIO_BSRR_BR0;
        delay(1000);
    }
}