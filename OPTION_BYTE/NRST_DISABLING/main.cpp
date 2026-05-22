#include "RTE_Components.h"
#include CMSIS_device_header

/*-----------------------------------------------------*/
/*                   disables NRST                     */
/* This program will change NRST setting at unvolatile */
/* OPTION BYTE.                                        */
/* Can connect LED to PC1 for stasus check.            */
/* When done the change OPTION BYTE, LED will slow     */
/* blink 10 times. After that LED will fast blink.     */
/* When NRST was already disabled, LED will fast blink.*/
/*                                                     */
/* note:This program had include ANTI BRICK routine.   */
/* When MCU does not respond to programmer, connect    */
/* PA0 to GND and restart MCU.(Power OFF/ON)           */
/* MCU will looping. When looping MCU, programmer can  */
/* connect to MCU and abailable reprogramming.         */
/*-----------------------------------------------------*/

int main() {
    uint32_t i,j;

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

    /* set PA1 as output mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE1) | GPIO_MODER_MODE1_0;
    /* set PA1 as push-pull mode */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT1;
    /* when NRST already disabled, fast blink LED at PA1 */
    if((FLASH->OPTR & FLASH_OPTR_NRST_MODE) != 0){
        while(1){
            GPIOA->ODR ^= GPIO_ODR_OD1; // Toggle PA1
            for (i = 0; i < 50000; i++); // Delay
        }
    }
    /*----------------------------------------*/
    /* change OPTION_BYTE for disable NRST    */
    /* GPIO PC0 will usable for I/O operation */
    /*----------------------------------------*/
    /* unlock FLASH */
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
    /* wait for clear busy bit */
    while (FLASH->SR & FLASH_SR_BSY);
    /* unlock Option Bytes */
    FLASH->OPTKEYR = FLASH_OPTKEY1;
    FLASH->OPTKEYR = FLASH_OPTKEY2;
    /* wait for clear busy bit */
    while (FLASH->SR & FLASH_SR_BSY);
    /* set NRST bit(disable NRST,PC0 as GPIO) */
    FLASH->OPTR |= FLASH_OPTR_NRST_MODE;
    /* start write OPTION BYTE */
    FLASH->CR |= FLASH_CR_OPTSTRT;
    /* some 32bit data write to 0x40022080 */
    /* trigger a formal program operation */
    *(volatile uint32_t *)0x40022080 = 0x55AACC33;
    /* wait for write option byte */
    while (FLASH->SR & FLASH_SR_BSY);
    /* wait for write completes successfully.*/
    while ((FLASH->SR & FLASH_SR_EOP) != 0);
    /* slow blink LED at PC0 10 times */
    for(j = 0;j < 10;j++){
        GPIOA->ODR ^= GPIO_ODR_OD1; // Toggle PA1
        for (i = 0; i < 600000; i++); // Delay
    }
    /* restart MCU for refresh FLASH->OPTR */
    FLASH->CR |= FLASH_CR_OBL_LAUNCH;
    while(1);
}