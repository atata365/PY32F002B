/* UART repeater */
/* This program is maken for PY32F002B SOP14 package */
/* It's a simple UART repeater that forwards data received */ 
/* on USART1 to the same USART1 */
/* fixed baud rate: 9600 bps */
/* data receive port as PA4(USART RX #9) */
/* repeat data port as PA6(USART TX #10) */
/* port numbers are based on SOP14 package */
#include "RTE_Components.h"
#include CMSIS_device_header

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
    INIT_GPIOs();
    INIT_USART();
    while (1) {
        /* check overrun error abd clear overrun error */
        while(USART1->SR & USART_SR_ORE) (void)USART1->DR;
        /* receive data from USART1 */
        if (USART1->SR & USART_SR_RXNE) {
            char received_char = USART1->DR; // read received data
            /* echo back the received character */
            while (!(USART1->SR & USART_SR_TXE)); // wait until transmit data register is empty
            USART1->DR = received_char; // send back the received character
            /* wait for transmission to complete */
            while (!(USART1->SR & USART_SR_TC));
        }
        /* transmit data via USART1 */
        //if (USART1->SR & USART_SR_TXE) {
        //    USART1->DR = 'A'; // send character 'A' continuously
        //    /* wait for transmission to complete */
        //    while (!(USART1->SR & USART_SR_TC));
        //}
    }
}