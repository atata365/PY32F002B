#include "RTE_Components.h"
#include CMSIS_device_header

int main() {
    /*+++++++++++++++++++++++ ANTI BRICK +++++++++++++++++++++++*/
    /*            ANTIBRICK avoids SWIO malfunctions.           */
    /*  GPIO PA0 connect to GND at boot time. MCU will looping. */
    /*       When looping, can connect progrmmers to SWIO.      */
    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    uint32_t i;
    /* activate HSI */
    RCC->CR = RCC_CR_HSION; // Enable HSI
    /* wait for HSI stabled */
    while (!(RCC->CR & RCC_CR_HSIRDY));
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN; // Enable GPIOA clock
    RCC->IOPENR |= RCC_IOPENR_GPIOCEN; // Enable GPIOC clock
    /* set PA0 as input mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE0);
    /* activate pullup to PA0 */
    GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD0) | GPIO_PUPDR_PUPD0_0;
    /* set PC0 as output mode */
    GPIOC->MODER = (GPIOC->MODER & ~GPIO_MODER_MODE0) | GPIO_MODER_MODE0_0;
    /* set PC0 to push-pull mode */
    GPIOC->OTYPER &= ~GPIO_OTYPER_OT0;
    /* set PA1 as output mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE1) | GPIO_MODER_MODE1_0;
    /* set PA1 to push-pull mode */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT1;
    while(!(GPIOA->IDR & GPIO_IDR_ID0));
    while(!(GPIOA->IDR & GPIO_IDR_ID0));
    

    while (1) {
        GPIOC->ODR ^= GPIO_ODR_OD0; // Toggle PC0
        GPIOA->ODR ^= GPIO_ODR_OD1; // Toggle PA1
        for(i = 0; i < 500000; i++);
    }
}