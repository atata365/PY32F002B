/* Display characters to ST7567 based LCD */
/*-----------------------------------------*/
/* Initialize GPIO's for USART and SPI     */
/* BACKLIGHT PA1  #6  (Software driven)    */
/* SPI_MOSI  PA0  #5  (AF0)                */
/* SPI_SCK   PB0  #4  (AF0)                */
/* Cmd/Dat   PB1  #3  (Software driven)    */
/* Reset     PB2  #2  (Software driven)    */
/* USART_RX  PA4  #9  (AF1)                */
/* USART_TX  PA6  #10 (AF1) use for debug  */
/* Pin numbers are based on SOP14 package  */
/*-----------------------------------------*/

#include "RTE_Components.h"
#include CMSIS_device_header

/* Horizontal gap is horizon mem size - display size */
#define H_gap 132 - 128

/* Global valiables */
volatile uint32_t L;    // SysTick counter
volatile uint8_t CUR_PAGE;  // Current page
volatile uint8_t CUR_COL;   // Current column
volatile bool HREVERSE_status = false;

/* working valiables(internaly used) */
#define MAX_PAGE 8
#define MAX_COL 128

uint8_t NextPage;
uint8_t NextCol;
uint8_t CurH_mag = 1;
uint8_t CurV_mag = 1;

/* Character font data */
/*----------------------------*/
/* font data for OLED display */
/*----------------------------*/
#define UPPERCASE
#define LOWERCASE
#define EXT_CHR
// 7x8 font
const char font_chr[][7] = {
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  // SP 0x20
  { 0x00, 0x00, 0x00, 0x6F, 0x6F, 0x00, 0x00 },  // !
  { 0x00, 0x0B, 0x07, 0x00, 0x0B, 0x07, 0x00 },  // “
  { 0x14, 0x14, 0x7F, 0x14, 0x7F, 0x14, 0x14 },  // #
  { 0x24, 0x2A, 0x2A, 0x7F, 0x2A, 0x2A, 0x12 },  // $
  { 0x47, 0x25, 0x13, 0x08, 0x64, 0x52, 0x71 },  // %
  { 0x60, 0x96, 0x89, 0x89, 0x56, 0x20, 0x50 },  // &
  { 0x00, 0x00, 0x0B, 0x07, 0x00, 0x00, 0x00 },  // ‘
  { 0x00, 0x00, 0x1C, 0x22, 0x41, 0x00, 0x00 },  // (
  { 0x00, 0x00, 0x41, 0x22, 0x1C, 0x00, 0x00 },  // )
  { 0x00, 0x44, 0x28, 0xFE, 0x28, 0x44, 0x00 },  // *
  { 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00 },  // +
  { 0x00, 0x00, 0xD0, 0x70, 0x20, 0x00, 0x00 },  // ,
  { 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00 },  // -
  { 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00 },  // .
  { 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 },  // /
  { 0x3E, 0x41, 0x41, 0x41, 0x41, 0x41, 0x3E },  // 0 0x30
  { 0x00, 0x04, 0x02, 0x7F, 0x00, 0x00, 0x00 },  // 1
  { 0x42, 0x61, 0x51, 0x49, 0x49, 0x45, 0x42 },  // 2
  { 0x41, 0x49, 0x49, 0x49, 0x4D, 0x4B, 0x31 },  // 3
  { 0x20, 0x30, 0x28, 0x24, 0x22, 0x7F, 0x20 },  // 4
  { 0x27, 0x45, 0x45, 0x45, 0x45, 0x45, 0x39 },  // 5
  { 0x30, 0x48, 0x4C, 0x4A, 0x49, 0x48, 0x30 },  // 6
  { 0x03, 0x01, 0x01, 0x71, 0x09, 0x05, 0x03 },  // 7
  { 0x36, 0x49, 0x49, 0x49, 0x49, 0x49, 0x36 },  // 8
  { 0x06, 0x09, 0x49, 0x29, 0x19, 0x09, 0x06 },  // 9
  { 0x00, 0x00, 0x36, 0x36, 0x00, 0x00, 0x00 },  // :
  { 0x00, 0x00, 0x56, 0x36, 0x00, 0x00, 0x00 },  // ;
  { 0x08, 0x14, 0x14, 0x22, 0x22, 0x41, 0x41 },  // <
  { 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24 },  // =
  { 0x41, 0x41, 0x22, 0x22, 0x14, 0x14, 0x08 },  // >
  { 0x00, 0x02, 0x01, 0x51, 0x09, 0x06, 0x00 },  // ?
#ifdef UPPERCASE
  { 0x3E, 0x41, 0x5D, 0x55, 0x4D, 0x11, 0x7E },  // @ 0x40
  { 0x78, 0x14, 0x12, 0x11, 0x12, 0x14, 0x78 },  // A
  { 0x7F, 0x49, 0x49, 0x49, 0x49, 0x49, 0x36 },  // B
  { 0x3E, 0x41, 0x41, 0x41, 0x41, 0x41, 0x22 },  // C
  { 0x7F, 0x41, 0x41, 0x41, 0x41, 0x22, 0x1C },  // D
  { 0x7F, 0x49, 0x49, 0x49, 0x49, 0x49, 0x41 },  // E
  { 0x7F, 0x09, 0x09, 0x09, 0x09, 0x09, 0x01 },  // F
  { 0x3E, 0x41, 0x41, 0x41, 0x49, 0x49, 0x38 },  // G
  { 0x7F, 0x08, 0x08, 0x08, 0x08, 0x08, 0x7F },  // H
  { 0x00, 0x00, 0x41, 0x7F, 0x41, 0x00, 0x00 },  // I
  { 0x30, 0x40, 0x40, 0x40, 0x41, 0x3F, 0x01 },  // J
  { 0x7F, 0x20, 0x10, 0x08, 0x14, 0x22, 0x41 },  // K
  { 0x7F, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 },  // L
  { 0x7F, 0x02, 0x04, 0x08, 0x04, 0x02, 0x7F },  // M
  { 0x7F, 0x02, 0x04, 0x08, 0x10, 0x20, 0x7F },  // N
  { 0x1C, 0x22, 0x41, 0x41, 0x41, 0x22, 0x1C },  // O
  { 0x7F, 0x09, 0x09, 0x09, 0x09, 0x09, 0x06 },  // P 0x50
  { 0x1C, 0x22, 0x41, 0x41, 0x51, 0x22, 0x5C },  // Q
  { 0x7F, 0x09, 0x09, 0x09, 0x19, 0x29, 0x46 },  // R
  { 0x26, 0x49, 0x49, 0x49, 0x49, 0x49, 0x32 },  // S
  { 0x01, 0x01, 0x01, 0x7F, 0x01, 0x01, 0x01 },  // T
  { 0x1F, 0x20, 0x40, 0x40, 0x40, 0x20, 0x1F },  // U
  { 0x0F, 0x10, 0x20, 0x40, 0x20, 0x10, 0x0F },  // V
  { 0x7F, 0x20, 0x10, 0x08, 0x10, 0x20, 0x7F },  // W
  { 0x41, 0x22, 0x14, 0x08, 0x14, 0x22, 0x41 },  // X
  { 0x01, 0x02, 0x04, 0x78, 0x04, 0x02, 0x01 },  // Y
  { 0x41, 0x61, 0x51, 0x49, 0x45, 0x43, 0x41 },  // Z
  { 0x00, 0x00, 0x7F, 0x41, 0x41, 0x00, 0x00 },  // [
  { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40 },  // '\'
  { 0x00, 0x00, 0x41, 0x41, 0x7F, 0x00, 0x00 },  // ]
  { 0x00, 0x04, 0x02, 0x01, 0x02, 0x04, 0x00 },  // ^
  { 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40 },  // _
#endif
#ifdef LOWERCASE
  { 0x00, 0x00, 0x01, 0x03, 0x06, 0x00, 0x00 },  // ` 0x60
  { 0x30, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x7C },  // a
  { 0x7F, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38 },  // b
  { 0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x28 },  // c
  { 0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x7F },  // d
  { 0x38, 0x54, 0x54, 0x54, 0x54, 0x54, 0x58 },  // e
  { 0x00, 0x08, 0x08, 0x7E, 0x09, 0x09, 0x00 },  // f
  { 0x0C, 0x52, 0x52, 0x52, 0x52, 0x52, 0x3E },  // g
  { 0x7F, 0x08, 0x04, 0x04, 0x04, 0x04, 0x78 },  // h
  { 0x00, 0x00, 0x00, 0x7D, 0x00, 0x00, 0x00 },  // I
  { 0x30, 0x40, 0x40, 0x40, 0x40, 0x40, 0x3D },  // j
  { 0x7F, 0x20, 0x10, 0x18, 0x24, 0x42, 0x42 },  // k
  { 0x00, 0x00, 0x00, 0x7F, 0x40, 0x00, 0x00 },  // l
  { 0x7C, 0x04, 0x04, 0x78, 0x04, 0x04, 0x78 },  // m
  { 0x7C, 0x08, 0x04, 0x04, 0x04, 0x04, 0x78 },  // n
  { 0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38 },  // o
  { 0x7C, 0x14, 0x14, 0x14, 0x14, 0x14, 0x08 },  // p 0x70
  { 0x08, 0x14, 0x14, 0x14, 0x14, 0x14, 0x7C },  // q
  { 0x7C, 0x08, 0x04, 0x04, 0x04, 0x04, 0x08 },  // r
  { 0x48, 0x54, 0x54, 0x54, 0x54, 0x54, 0x24 },  // s
  { 0x00, 0x04, 0x04, 0x3F, 0x44, 0x44, 0x00 },  // t
  { 0x3C, 0x40, 0x40, 0x40, 0x40, 0x20, 0x7C },  // u
  { 0x0C, 0x10, 0x20, 0x40, 0x20, 0x10, 0x0C },  // v
  { 0x7C, 0x20, 0x10, 0x08, 0x10, 0x20, 0x7C },  // w
  { 0x44, 0x28, 0x10, 0x10, 0x10, 0x28, 0x44 },  // x
  { 0x04, 0x48, 0x50, 0x20, 0x10, 0x08, 0x04 },  // y
  { 0x44, 0x64, 0x54, 0x54, 0x54, 0x4C, 0x44 },  // z
  { 0x00, 0x00, 0x08, 0x36, 0x41, 0x00, 0x00 },  // {
  { 0x00, 0x00, 0x00, 0x77, 0x00, 0x00, 0x00 },  // |
  { 0x00, 0x00, 0x41, 0x36, 0x08, 0x00, 0x00 },  // }
  { 0x08, 0x04, 0x04, 0x08, 0x10, 0x10, 0x08 },  // ~
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },  //(SP)
#endif
#ifdef EXT_CHR
  { 0x00, 0x06, 0x09, 0x09, 0x09, 0x06, 0x00 },  // ° 0x80
#endif
};

/*---------------------------------------*/
/* excetion process routuine for SysTick */
/* This routine called every 1ms.        */
/*---------------------------------------*/
extern "C" {
    void SysTick_Handler(void) {
        L++;
    }
}

/*-------------------------------------*/
/* initSysTick                         */
/* start SysTick interrupt and counter */
/*-------------------------------------*/
void INIT_SysTick(void) {
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

/*-----------------------------------------*/
/* Initialize GPIO's for USART and SPI     */
/* BACKLIGHT PA1  #6  (Software driven)    */
/* SPI_MOSI  PA0  #5  (AF0)                */
/* SPI_SCK   PB0  #4  (AF0)                */
/* Cmd/Dat   PB1  #3  (Software driven)    */
/* Reset     PB2  #2  (Software driven)    */
/* USART_RX  PA4  #9  (AF1)                */
/* USART_TX  PA6  #10 (AF1) use for debug  */
/* Pin numbers are based on SOP14 package  */
/*-----------------------------------------*/
void INIT_GPIOs(void) {
    /* activate GPIOA and GPIOb for USART and SPI */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN;

    /* Setting GPIOs for SPI */
    /* set alternate function PA0(MOSI), PB0(SCK) to AF0 (SPI) */
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~GPIO_AFRL_AFSEL0_Msk);
    /* set PB0(SCK) AF0 (SPI) */
    GPIOB->AFR[0] = (GPIOB->AFR[0] & ~GPIO_AFRL_AFSEL0_Msk);

    /* settings for GPIO PA1(BACKLIGHT) */
    /* set PA1(BACKLIGHT) to ouput mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE1_Msk) | GPIO_MODER_MODE1_0;
    /* set PA1 ro push-pull mode */
    GPIOA->OTYPER = (GPIOA->OTYPER & ~GPIO_OTYPER_OT1_Msk) | GPIO_OTYPER_OT1; 

    /* settings for GPIO PA0(MOSI) */
    /* set PA0(MOSI) to multiplexed mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE0_Msk) | GPIO_MODER_MODE0_1;
    /* set PA0(MOSI) speed to very HIGH speed */
    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED0_0 | GPIO_OSPEEDR_OSPEED0_1;
    /* activate pull-up */
    GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD0_Msk) | GPIO_PUPDR_PUPD0_0;

    /* settings for GPIO PB0(SCK) */
    /* set PB0(SCK) to multiplexed mode */
    GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE0_Msk) | GPIO_MODER_MODE0_1;
    /* set PB0(SCK) speed to very HIGH speed */
    GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED0_0 | GPIO_OSPEEDR_OSPEED0_1;
    /* activate pull-up */
    GPIOB->PUPDR = (GPIOB->PUPDR & ~GPIO_PUPDR_PUPD0_Msk) | GPIO_PUPDR_PUPD0_0;

    /* settings for GPIO PB1(Cmd/Data) */
    /* set PB1(Cmd/Data) to Push/Pull mode */
    GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE1_Msk) | GPIO_MODER_MODE1_0;
    /* set PB1(Cmd/Data) speed to very HIGH speed */
    GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED1_0 | GPIO_OSPEEDR_OSPEED1_1;

    /* settings for GPIO PB2(Reset) */
    /* set PB2(Reset) to Push/Pull mode */
    GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE2_Msk) | GPIO_MODER_MODE2_0;
    /* set PB2(Reset) speed to very HIGH speed */
    GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED2_0 | GPIO_OSPEEDR_OSPEED2_1;

    /* Setting GPIOs for USART1 */
    /* set PA4(RX) abd PA6(TX) to AF1 (USART1) */
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
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT6;
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
/* pin numbers are based on SOP14 package */
void INIT_USART(void) {
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

void INIT_SPI(void){
    RCC->APBENR2|= RCC_APBENR2_SPI1EN;
     /* set SPI1 to master mode, full duplex, 8-bit data frame, clock polarity low, clock phase first edge */
    //SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_BR_0 | SPI_CR1_BR_1;
    /* SPI clock rate=fPClk/2(BR0,BR1,BR2 as 0) (MAX speed) */
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM;
    /* enable SPI1 (Note: Do this opertion shuld be separatly) */
    SPI1->CR1 |= SPI_CR1_SPE;
    /* Turn on the backllight (PA1->LOW) */
    GPIOA->BSRR = GPIO_BSRR_BR1;
}

void SPI_Transmit(uint8_t code) {
    /* wait until transmit buffer is empty */
    while (!(SPI1->SR & SPI_SR_TXE));
    /* send data */
    SPI1->DR = code;
    /* wait until transmission is complete */
    while (SPI1->SR & SPI_SR_BSY);
}

void SEND_CMD(uint8_t cmd) {
    /* set Cmd/Dat pin LOW for command */
    GPIOB->BSRR = GPIO_BSRR_BR1;
    SPI_Transmit(cmd);
}

void SEND_DATA(uint8_t data) {
    /* set Cmd/Dat pin HIGH for data */
    GPIOB->BSRR = GPIO_BSRR_BS1;
    SPI_Transmit(data);
}

void HW_RESET_ST7567(void) {
    /* Reset the LCD */
    GPIOB->BSRR = GPIO_BSRR_BR2; // Set Reset pin LOW
    delay(10); // Wait for 10 ms
    GPIOB->BSRR = GPIO_BSRR_BS2; // Set Reset pin HIGH
    delay(10); // Wait for 10 ms
}

void SW_RESET_ST7567(void) {
    /* Soft reset the LCD */
    SEND_CMD(0xE2); // Soft reset command
    delay(10); // Wait for 10 ms
}

void SW_ST7567(bool SW) {
    SW ? SEND_CMD(0xAF) : SEND_CMD(0xAE); // Display ON/OFF
}

/* Set display direction */
/* true:upside downn, false:normly */
void VREVERSE_ST7567(bool dir) {
    dir ? SEND_CMD(0xC0) : SEND_CMD(0xC8);
}

/* true:upside downn, false:normly */
void HREVERSE_ST7567(bool dir) {
    dir ? SEND_CMD(0xA1) : SEND_CMD(0xA0);
    HREVERSE_status = dir;
}

void INVERSE_ST7567(bool inverse) {
    inverse ? SEND_CMD(0xA7) : SEND_CMD (0xA6); // Inverse display or Normal display
}

/* Set reguration ratio (contrast) */
void SETRR_ST7567(uint8_t RR) {
    SEND_CMD(0x20 + (RR & 0x7));
}

void INIT_ST7567(){
    /* Reset the LCD */
    HW_RESET_ST7567();
    /* Soft reset the LCD */
    //SW_RESET_ST7567();  

    /* Initialize the ST7567 LCD controller */
    SW_ST7567(false); // Display OFF
    SEND_CMD(0xA3); // Set bias voltage
    //DISPDIR_ST7567(true); // Set direction:Left to right
    //DISPDIR_ST7567(true); // Set direction:Right to left
    //SEND_CMD(0xA5); // ALLpixel ON
    //SEND_CMD(0xA4); // ALLpixel ON
    SEND_CMD(0x20); // Set voltage resistor ratio
    //SEND_CMD(0x2F); // Power control set
    SEND_CMD(0x2F); // Power control set
    //SEND_CMD(0x40); // Set display start line
    SW_ST7567(true); // Display ON
}

void BACK_LIGHT(bool light_on){
    light_on ? GPIOA->BSRR = GPIO_BSRR_BR1 : GPIOA->BSRR = GPIO_BSRR_BS1;
}

void SET_PAGE(uint8_t page) {
    SEND_CMD(0xB0 | (page & 0x07)); // Set page address
}

void SET_COLUMN(uint8_t column) {
    if(HREVERSE_status) column += H_gap;
    SEND_CMD(0x10 | ((column >> 4) & 0x0F)); // Set column address (high nibble)
    SEND_CMD(0x00 | (column & 0x0F)); // Set column address (low nibble)
}

void SET_CURSOR(uint8_t page, uint8_t column) {
    SET_PAGE(page);
    SET_COLUMN(column);
    NextPage = page;
    NextCol = column;
}

void CLEAR_SCREEN(void) {
    for (uint8_t i = 0; i < 8; i++) {
        SET_PAGE(i); 
        SET_COLUMN(0);
        for (uint8_t j = 0; j < 128; j++) {
            SEND_DATA(0); // Fill the page with data
        }
    }
    NextPage = 0;
    NextCol = 0;
}


void SET_MAG(uint8_t V_mag, uint8_t H_mag) {
    CurV_mag = V_mag;
    CurH_mag = H_mag;
}

/*---------------------------------------------*/
/* putChr: put a character on current position */
/*               current position to be update */
/*---------------------------------------------*/
void PUT_CHR(uint8_t chr) {
    uint32_t work, ext_work;
    uint16_t dot_pos, i, j, k;
    uint8_t page, col, V_mag, H_mag;

    page = NextPage;
    col = NextCol;
    V_mag = CurV_mag;
    H_mag = CurH_mag;
    chr -= 0x20;                 // Adjust character data offset
    if (V_mag > 8) V_mag = 8;    // max V_mag=8
    if (H_mag > 16) H_mag = 16;  // max H_mag=16
    for (i = 0; i < 7; i++) {
        work = 0;
        ext_work = 0;
        /* vertical magnifies */
        for (j = 0; j < 8; j++) {
            if ((font_chr[chr][i] & (1 << j))) {
                dot_pos = j * V_mag;
                for (k = 0; k < V_mag; k++) {
                    if ((dot_pos + k) < 32) work |= 1 << (dot_pos + k);
                    else ext_work |= 1 << (dot_pos - 32 + k);
                }
            }
        }
        /* display character */
        for (j = 0; j < V_mag; j++) {  // Vertical loop
            SET_CURSOR(page + j, col + i * H_mag);
            for (k = 0; k < H_mag; k++) {  // Horizontal loop
                if (j < 4) SEND_DATA(work & 0xff);
                else SEND_DATA(ext_work & 0xff);
            }
            if (j < 4) work = work >> 8;
            else ext_work = ext_work >> 8;
        }
    }
    /* Make horizontal space(between character) */
    for (j = 0; j < V_mag; j++) {
        SET_CURSOR(page + j, col + i * H_mag);
        for (k = 0; k < H_mag; k++) SEND_DATA(0);
    }
    NextPage = page;
    NextCol = col + (H_mag << 3);          // H_mag * 8
    if (NextCol > (128 - (H_mag << 3))) {  // H_mag * 8
        NextCol = 0;
        NextPage += V_mag;
    }
    /* next page will reset to 0 if next page will be 0 */
    if (NextPage >= MAX_PAGE) NextPage = 0;
}
/*--------------------------------------------*/
/* putStr: put characters on current position */
/*--------------------------------------------*/
void PUT_STR(char str[]) {
    int i = 0;
    while (str[i]) {
        PUT_CHR(str[i]);
        i++;
    }
}
/* private functions */


int main() {
    uint8_t i;
    INIT_GPIOs();
    INIT_USART();
    /* start SysTick */
    INIT_SysTick();
    INIT_SPI();
    INIT_ST7567();
    CLEAR_SCREEN();
    VREVERSE_ST7567(false);
    HREVERSE_ST7567(false);
    SETRR_ST7567(4);
    
    //PUT_CHR('A');
    SET_CURSOR(3,16);
    SET_MAG(2, 1);
    PUT_STR((char *)"Hello World!");
    //SET_CURSOR(4,0);
    //PUT_STR((char *)"abcdefghijklmnopqrstuvwxyz");
    //SET_MAG(8, 7);
    //SET_CURSOR(1,12);
    //PUT_STR((char *)"abcdefghijklmnopqrstuvwxyz");
    //PUT_CHR('A');
    //PUT_CHR('S');
    
    while (1) {
        BACK_LIGHT(true);
        INVERSE_ST7567(false);
        delay(5000); // Wait for 1 second
        INVERSE_ST7567(true);
        delay(1000);
        /* check overrun error abd clear overrun error */
        //while(USART1->SR & USART_SR_ORE) (void)USART1->DR;
        /* receive data from USART1 */
        //if (USART1->SR & USART_SR_RXNE) {
        //    char received_char = USART1->DR; // read received data
        //    /* echo back the received character */
        //    while (!(USART1->SR & USART_SR_TXE)); // wait until transmit data register is empty
        //    USART1->DR = received_char; // send back the received character
        //    /* wait for transmission to complete */
        //    while (!(USART1->SR & USART_SR_TC));
        //}
        /* transmit data via USART1 */
        //if (USART1->SR & USART_SR_TXE) {
        //    USART1->DR = 'A'; // send character 'A' continuously
        //    /* wait for transmission to complete */
        //    while (!(USART1->SR & USART_SR_TC));
        //}
    }
}