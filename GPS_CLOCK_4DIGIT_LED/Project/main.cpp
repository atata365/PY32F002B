/* GPIO INTERRUPT */
/* activate GPIO interrupt */
/* input pin:PA0(#5) - connect to SW(pull-upped)*/
/* output pin:PA1(#6) - connect to LED */
/* short press switch, turn on the LED partially */
/* long press switch, turn on the LED and locked */
/* pin numbers are based on the PY32F002B SOP14 package */

/* fixed baud rate: 9600 bps */
/* data receive port as PA4(USART RX #9) */
/* repeat data port as PA6(USART TX #10) */
/* port numbers are based on SOP14 package */

#include "RTE_Components.h"
#include CMSIS_device_header

#define AVOID_CHETTERING 50
#define LONG_PRESS_DURATION 600

/* voratile resister for SysTick */
volatile uint32_t L = 0;
/* voratile resister for last SysTick */
volatile uint32_t lAST_SysTick = 0;
/* voratile resister for switch on duration */
volatile uint32_t DURATION = 0;

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
        /* check EXTI interrupt line */
        if(EXTI->PR & EXTI_PR_PR0) {
            /* clear EXTI interrupt flag */
            EXTI->PR |= EXTI_PR_PR0;
            /* check rising edge*/
            if(!(GPIOA->IDR & GPIO_IDR_ID0)) {
                /* avoid chettering  */
                if(L - lAST_SysTick > AVOID_CHETTERING) {
                    DURATION = 0;
                    lAST_SysTick = L;
                    /* rising edge*/
                    /* turn on the LED at PA1 */
                    GPIOA->BSRR = GPIO_BSRR_BS1;
                }
            } else {
                /* falling edge */
                DURATION = L - lAST_SysTick;
                if(DURATION < LONG_PRESS_DURATION) {
                    /* turn off the LED at PA1 */
                    GPIOA->BSRR = GPIO_BSRR_BR1;
                }
            }
        }
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

/*--------------------------------------------------------*/
/* initialize GPIO PA4(USART RX #9) and PA6(USART TX #10) */
/* pin nubbers are based on SOP14 package                 */
/*--------------------------------------------------------*/
void INIT_GPIOs() {
    /* initialize GPIO for USART (PA4 , PA6) */
    /* enable GPIOA clock */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    /* set PA6(TX) and PA4(RX) to AF1 (USART1) */
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(GPIO_AFRL_AFSEL4_Msk | GPIO_AFRL_AFSEL6_Msk))
                                        | GPIO_AFRL_AFSEL4_0 | GPIO_AFRL_AFSEL6_0;
    /* settings for GPIO PA4 */
    /* set PA4(RX) to multiplexed mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE4_Msk) | GPIO_MODER_MODE4_1;
    /* activate pull-up */
    GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD4_Msk) | GPIO_PUPDR_PUPD4_0;
    /* settings for GPIO PA6 */
    /* change PA6(TX) to multiplexed mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE6_Msk) | GPIO_MODER_MODE6_1; 
    /* change PA6(TX) to push-pull mode */
    GPIOA->OTYPER |= GPIO_OTYPER_OT6;
    /* set PA6(TX) speed to very HIGH speed */
    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED6_0 | GPIO_OSPEEDR_OSPEED6_1;
    /* activate pull-up */
    GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD6_Msk) | GPIO_PUPDR_PUPD6_0;
}

/*-------------------*/
/* initialize USART1 */
/*-------------------*/
/* USART1 TX -> PA6(#10) */
/* USART1 RX -> PA4(#9) */
/* pin nubbers are based on SOP14 package */
void INIT_USART() {
    /* enable USART1 clock */
    RCC->APBENR2 |= RCC_APBENR2_USART1EN;
    /* set baud rate to 9600 bps (assuming 24 MHz clock) */
    /* USARTDIV = 24 MHz / (16 * 9600) = 156.25 */
    /* USART_BRR_DIV_Mantissa = 156 */
    /* USART_BRR_DIV_Fraction = 0.25 * 16 = 4 */ 
    USART1->BRR = 156 << USART_BRR_DIV_Mantissa_Pos | 4 << USART_BRR_DIV_Fraction_Pos;
    /* enable USART,transmitter,receiver */
    USART1->CR1 |= USART_CR1_UE |USART_CR1_TE | USART_CR1_RE;
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