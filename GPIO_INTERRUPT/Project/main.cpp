/* GPIO INTERRUPT */
/* activate GPIO interrupt */
/* input pin:PA0(#5) - connect to SW(pull-upped)*/
/* output pin:PA1(#6) - connect to LED */
/* pin numbers are based on the PY32F002B SOP14 package */

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

/* EXTI0_1 Interrupt Handler */
extern "C" {
    __attribute__((interrupt)) void EXTI0_1_IRQHandler(void) {
        /* check rise or fall */
        if(GPIOA->IDR & GPIO_IDR_ID0) {
            /* falling edge */
            /* turn off the LED at PA1 */
            GPIOA->BSRR = GPIO_BSRR_BR1;
        } else {
            /* rising edge*/
            /* turn on the LED at PA1 */
            GPIOA->BSRR = GPIO_BSRR_BS1;
        }
        /* clear EXTI interrupt flag */
        EXTI->PR |= EXTI_PR_PR0;
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

void INIT_GPIO(void) {
    /* activate and enables PA0 as input,PA1 as output */
    /* activate GPIOA */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    /* set PA0 to input, PA1 to OUTPUT */
    GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1))
                                  | GPIO_MODER_MODE1_0;
    /* set PA1 to push-pull mode */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT1_Msk;
    /* set PA0 to pull-up */
    GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD0_Msk) | GPIO_PUPDR_PUPD0_0;
}

void INIT_EXTI(void) {
    /* set EXTI0 to PA0 (EXTI_EXTICR1_EXTI0 = 0,EXTI0 as Px0,0 as PA) */
    EXTI->EXTICR[0] &= ~EXTI_EXTICR1_EXTI0_Msk;
    /* set EXTI0 to rising edge trigger */
    EXTI->RTSR |= EXTI_RTSR_RT0;
    /* set EXTI0 to falling edge trigger */
    EXTI->FTSR |= EXTI_FTSR_FT0;
    /* enable EXTI0 */
    EXTI->IMR |= EXTI_IMR_IM0;
    /* enable interrupts in NVIC */
    NVIC->ISER[0] |= (1 << EXTI0_1_IRQn);
}

int main() {
    /* initialize GPIO */
    INIT_GPIO();
    /* initialize EXTI */
    INIT_EXTI();
    /* start SysTick */
    initSysTick();
    //GPIOA->BSRR = GPIO_BSRR_BS1; /* turn on the LED at PA1 */
    while(1){
        //GPIOA->BSRR = GPIO_BSRR_BS1; /* turn on the LED at PA1 */
        //delay(1000);
        //GPIOA->BSRR = GPIO_BSRR_BR1; /* turn off the LED at PA1 */
        //delay(1000);
    }
}