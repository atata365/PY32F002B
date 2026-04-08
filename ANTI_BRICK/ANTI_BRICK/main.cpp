#include "RTE_Components.h"
#include CMSIS_device_header

int main() {
    /*+++++++++++++++++++++++ ANTI BRICK +++++++++++++++++++++++*/
    /*            ANTIBRICK avoids SWIO malfunctions.           */
    /*  GPIO PA1 connect to GND at boot time. MCU will looping. */
    /*       When looping, can connect progrmmers to SWIO.      */
    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* activate HSI */
    RCC->CR = RCC_CR_HSION; // Enable HSI
    /* wait for HSI stabled */
    while (!(RCC->CR & RCC_CR_HSIRDY));
    RCC->IOPENR = RCC_IOPENR_GPIOAEN; // Enable GPIOA clock
    /* set PA1 as input mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE1);
    /* pull-up PA1 */
    GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD1) | GPIO_PUPDR_PUPD1_0;
    /* when looping PA1 is 0(low) */
    while(!(GPIOA->IDR & GPIO_IDR_ID1_Msk));
    /*----- conclude ANTI BRICK -----*/
    /* set PA0 to output mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE0) | GPIO_MODER_MODE0_0;
    /* set PA0 to push-pull mode */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT0;

    while (1) {
        GPIOA->ODR ^= GPIO_ODR_OD0; // Toggle PA0
        for (volatile int i = 0; i < 500000; i++); // Delay
    }
}