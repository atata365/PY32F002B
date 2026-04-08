#include "RTE_Components.h"
#include CMSIS_device_header

int main() {
    RCC->CR = RCC_CR_HSION; // Enable HSI
    while (!(RCC->CR & RCC_CR_HSIRDY)); // Wait for HSI to be ready
    RCC->IOPENR = RCC_IOPENR_GPIOAEN; // Enable GPIOA clock
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE0_Msk) | GPIO_MODER_MODE0_0; // Set PA0 as output
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT0; // Set PA0 as push-pull
    GPIOA->BSRR = GPIO_BSRR_BS0; // Set PA0 high
    while (1) {
        GPIOA->ODR ^= GPIO_ODR_OD0; // Toggle PA0
        for (volatile int i = 0; i < 100000; i++); // Delay
    }
}
