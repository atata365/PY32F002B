#include "RTE_Components.h"
#include CMSIS_device_header

int main() {
    /*+++++++++++++++++++++++ ANTI BRICK +++++++++++++++++++++++*/
    /*           ANTI BRICK avoids SWIO malfunctions.           */
    /*  GPIO PA0 connect to GND at boot time. MCU will looping. */
    /*       When looping, can connect progrmmers to SWIO.      */
    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* activate HSI */
    RCC->CR = RCC_CR_HSION; // Enable HSI
    /* wait for HSI stabled */
    while (!(RCC->CR & RCC_CR_HSIRDY));
    RCC->IOPENR = RCC_IOPENR_GPIOAEN; // Enable GPIOA
    /* set PA0 as input mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE0);
    /* pull-up PA0 */
    GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD0) | GPIO_PUPDR_PUPD0_0;
    /* when looping PA0 is 0(low) */
    while(!(GPIOA->IDR & GPIO_IDR_ID0));
    /*----- conclude ANTI BRICK -----*/

    /*---------------------------------------*/
    /*   turn off the SWDIO ports(SWD,SWC)   */
    /* can use SWD ports as GPIO PA2 and PB6 */
    /*---------------------------------------*/
    /* note: SYSCFG setting is not necessery to disabling SWDIO */
    /* activate SYSCFG module */
    //RCC->APBENR2 |= RCC_APBENR2_SYSCFGEN;
    /* disable SWIO（SWD-PB6:#8/SWCLK-PA2:#7) */
    /* pin numbers are based on SOP14 package */
    //SYSCFG->CFGR1 |= (1 << 8);
    
    /* turn off the SWC (can use GPIO PB6) */
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN; // Enable GPIOB clock
    GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE6) | GPIO_MODER_MODE6_0; // Set PB6 as output
    GPIOB->OTYPER &= ~GPIO_OTYPER_OT6; // Set PB6 as push-pull mode

    /* turn off the SWD (can use GPIO PA2) */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE2) | GPIO_MODER_MODE2_0; // Set PA2 as output
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT2; // Set PA2 as push-pull mode

    /* blink LED at PA2 and PB6 */
    GPIOB->ODR ^= GPIO_ODR_OD6; // Toggle PB6
    for(int i = 0; i < 100; i++) {
        GPIOB->ODR ^= GPIO_ODR_OD6; // Toggle PB6
        GPIOA->ODR ^= GPIO_ODR_OD2; // Toggle PA2
        for (volatile int i = 0; i < 500000; i++); // Delay
    }
    /* activate SWDIO(normally operation) */
    /* set PA2(SWC) and PB6(SWD) to Alternate Function mode */
    /* set PA2(SWC) to pull-downed alternate function mode*/
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE2) | GPIO_MODER_MODE2_1; // AF mode
    GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD2) | GPIO_PUPDR_PUPD2_1; // pull-down
    /* set PB6(SWD) to pull-upped alternate function mode*/
    GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE6) | GPIO_MODER_MODE6_1; // AF mode
    GPIOB->PUPDR = (GPIOB->PUPDR & ~GPIO_PUPDR_PUPD6) | GPIO_PUPDR_PUPD6_0; // pull-up
    while(1);
}