/*---------------------------------------------------------*/
/*       blink LED at PA2(SWC),PB6(SWD),PC0(NRST)          */
/*    This program is blink LED, connected PA2,PB6,PC0.    */
/*   Thease PIN's default function are SWC,SWD ans NRST    */
/*    Default function can desable and switch to GPIO.     */
/*    This program is check switched GPIO functionality.   */
/*                        CAUTION                          */
/*  This program will be disabled some default functions.  */
/*     Disabled functions(SWD,SWC,NRST) are not usable.    */
/*         Can not reset chip and download program.        */
/*   This program has ANTI BRICK routine. Connect PA0 to   */
/*  GND at boot time, program will loop before disabling   */
/* default functions. When looping, can connect programmer.*/
/*---------------------------------------------------------*/
#include "RTE_Components.h"
#include CMSIS_device_header

int main() {
    uint32_t i;
    /*+++++++++++++++++++++++ ANTI BRICK +++++++++++++++++++++++*/
    /*            ANTIBRICK avoids SWIO malfunctions.           */
    /*  GPIO PA0 connect to GND at boot time. MCU will looping. */
    /*       When looping, can connect progrmmers to SWIO.      */
    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* activate HSI */
    RCC->CR = RCC_CR_HSION; // Enable HSI
    /* wait for HSI stabled */
    while (!(RCC->CR & RCC_CR_HSIRDY));
    /* activate GPIOA,GPIOB,GPIOC*/
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN;
    /* set PA0 as input mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE0);
    /* activate pullup to PA0 */
    GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD0) | GPIO_PUPDR_PUPD0_0;
    /* loooping when PA0 coonnected to GND (ANTI BRICK loop) */
    while(!(GPIOA->IDR & GPIO_IDR_ID0));
    /*----- conclude ANTI BRICK -----*/

    /* prepare GPIO ports */
    /* set PA1 as output mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE1) | GPIO_MODER_MODE1_0;
    /* set PA1 to push-pull mode */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT1;
    /* set PA2 as output mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE2) | GPIO_MODER_MODE2_0;
    /* set PA2 to push-pull mode */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT2;
    /* set PB6 as output mode */
    GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE6) | GPIO_MODER_MODE6_0;
    /* set PB6 to push-pull mode */
    GPIOB->OTYPER &= ~GPIO_OTYPER_OT6;
    /* set PC0 as output mode */
    GPIOC->MODER = (GPIOC->MODER & ~GPIO_MODER_MODE0) | GPIO_MODER_MODE0_0;
    /* set PC0 to push-pull mode */
    GPIOC->OTYPER &= ~GPIO_OTYPER_OT0;
    /* loop for LED blinking */
    while (1) {
        GPIOA->ODR ^= GPIO_ODR_OD1; // Toggle PA1
        GPIOA->ODR ^= GPIO_ODR_OD2; // Toggle PA2(SWC)
        GPIOB->ODR ^= GPIO_ODR_OD6; // Toggle PB6(SWD)
        GPIOC->ODR ^= GPIO_ODR_OD0; // Toggle PC0(NRST)
        for(i = 0; i < 500000; i++);
    }
}