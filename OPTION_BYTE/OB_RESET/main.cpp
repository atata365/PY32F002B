#include "RTE_Components.h"
#include CMSIS_device_header

/*-----------------------------------------------------*/
/*        reset option byte to factory setting         */
/* This program will change NRST setting at unvolatile */
/* OPTION BYTE.                                        */
/*-----------------------------------------------------*/

int main() {
    uint32_t i,j;

    /*---------------------------*/
    /* factory reset OPTION_BYTE */
    /*---------------------------*/
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
    /* reset NRST bit(enables NRST,PC0 as NRST) */
    FLASH->OPTR = 0xB000;
    /* start write OPTION BYTE */
    FLASH->CR |= FLASH_CR_OPTSTRT;
    /* some 32bit data write to 0x40022080 */
    /* trigger a formal program operation */
    *(volatile uint32_t *)0x40022080 = 0x55AACC33;
    /* wait for write option byte */
    while (FLASH->SR & FLASH_SR_BSY);
    /* wait for write completes successfully.*/
    while ((FLASH->SR & FLASH_SR_EOP) != 0);
    while(1);
}