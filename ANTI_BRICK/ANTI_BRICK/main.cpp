#include "RTE_Components.h"
#include CMSIS_device_header

int main() {
    /*+++++++++++++++++++++++ ANTI BRICK +++++++++++++++++++++++*/
    /*            ANTIBRICK avoids SWIO malfunctions.           */
    /*  GPIO PA0 connect to GND at boot time. MCU will looping. */
    /*       When looping, can connect programmer to SWIO.      */
    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* activate HSI */
    RCC->CR = RCC_CR_HSION; // Enable HSI
    /* wait for HSI stabled */
    while (!(RCC->CR & RCC_CR_HSIRDY));
    RCC->IOPENR = RCC_IOPENR_GPIOAEN; // Enable GPIOA clock
    /* set PA0 as input mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE0);
    /* pull-up PA0 */
    GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD0) | GPIO_PUPDR_PUPD0_0;
    /* when looping PA0 is 0(low) */
    while(!(GPIOA->IDR & GPIO_IDR_ID0));
    /*----- conclude ANTI BRICK -----*/
    /* set PA1 to output mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE1) | GPIO_MODER_MODE1_0;
    /* set PA1 to push-pull mode */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT1;

    while (1) {
        GPIOA->ODR ^= GPIO_ODR_OD1; // Toggle PA1
        for (volatile int i = 0; i < 500000; i++); // Delay
    }
}