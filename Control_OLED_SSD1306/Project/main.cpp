/* Display characters to SSD1306 based OLED */
/*-----------------------------------------*/
/* Initialize GPIO's for USART and I2C     */
/* Anti Brick   PA0  #5                    */
/* SWC/I2C_SCL  PA2  #7  (AF6)             */
/* SWD/I2C_SDA  PB6  #8  (AF6)             */
/* USART_RX     PA4  #9  (AF1)             */
/* USART_TX     PA6  #10 (AF1)             */
/* Pin numbers are based on SOP14 package  */
/*-----------------------------------------*/

#include "RTE_Components.h"
#include "py32f002bx5.h"
#include CMSIS_device_header

#define SSD1306_ADDR 0x3C

/* Horizontal gap is horizon mem size - display size */
#define H_gap 132 - 128
/* Pages and columns */
#define MAX_PAGE 8
#define MAX_COL 128

/* Global valiables */
volatile uint32_t L;       // SysTick counter
volatile uint8_t CUR_PAGE; // Current page
volatile uint8_t CUR_COL;  // Current column
volatile bool HREVERSE_status = false;
volatile uint8_t NextPage;
volatile uint8_t NextCol;
volatile uint8_t CurH_mag = 1;
volatile uint8_t CurV_mag = 1;

/* Character font data */
/*---------------------------*/
/* font data for LCD display */
/*---------------------------*/
#define UPPERCASE
#define LOWERCASE
#define EXT_CHR
// 7x8 font
const char font_chr[][7] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // SP 0x20
    {0x00, 0x00, 0x00, 0x6F, 0x6F, 0x00, 0x00}, // !
    {0x00, 0x0B, 0x07, 0x00, 0x0B, 0x07, 0x00}, // “
    {0x14, 0x14, 0x7F, 0x14, 0x7F, 0x14, 0x14}, // #
    {0x24, 0x2A, 0x2A, 0x7F, 0x2A, 0x2A, 0x12}, // $
    {0x47, 0x25, 0x13, 0x08, 0x64, 0x52, 0x71}, // %
    {0x60, 0x96, 0x89, 0x89, 0x56, 0x20, 0x50}, // &
    {0x00, 0x00, 0x0B, 0x07, 0x00, 0x00, 0x00}, // ‘
    {0x00, 0x00, 0x1C, 0x22, 0x41, 0x00, 0x00}, // (
    {0x00, 0x00, 0x41, 0x22, 0x1C, 0x00, 0x00}, // )
    {0x00, 0x44, 0x28, 0xFE, 0x28, 0x44, 0x00}, // *
    {0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00}, // +
    {0x00, 0x00, 0xD0, 0x70, 0x20, 0x00, 0x00}, // ,
    {0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00}, // -
    {0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00}, // .
    {0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01}, // /
    {0x3E, 0x41, 0x41, 0x41, 0x41, 0x41, 0x3E}, // 0 0x30
    {0x00, 0x04, 0x02, 0x7F, 0x00, 0x00, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x49, 0x45, 0x42}, // 2
    {0x41, 0x49, 0x49, 0x49, 0x4D, 0x4B, 0x31}, // 3
    {0x20, 0x30, 0x28, 0x24, 0x22, 0x7F, 0x20}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x30, 0x48, 0x4C, 0x4A, 0x49, 0x48, 0x30}, // 6
    {0x03, 0x01, 0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x09, 0x49, 0x29, 0x19, 0x09, 0x06}, // 9
    {0x00, 0x00, 0x36, 0x36, 0x00, 0x00, 0x00}, // :
    {0x00, 0x00, 0x56, 0x36, 0x00, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x14, 0x22, 0x22, 0x41, 0x41}, // <
    {0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24}, // =
    {0x41, 0x41, 0x22, 0x22, 0x14, 0x14, 0x08}, // >
    {0x00, 0x02, 0x01, 0x51, 0x09, 0x06, 0x00}, // ?
#ifdef UPPERCASE
    {0x3E, 0x41, 0x5D, 0x55, 0x4D, 0x11, 0x7E}, // @ 0x40
    {0x78, 0x14, 0x12, 0x11, 0x12, 0x14, 0x78}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x41, 0x41, 0x49, 0x49, 0x38}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x00, 0x41, 0x7F, 0x41, 0x00, 0x00}, // I
    {0x30, 0x40, 0x40, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x20, 0x10, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x04, 0x08, 0x04, 0x02, 0x7F}, // M
    {0x7F, 0x02, 0x04, 0x08, 0x10, 0x20, 0x7F}, // N
    {0x1C, 0x22, 0x41, 0x41, 0x41, 0x22, 0x1C}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x09, 0x09, 0x06}, // P 0x50
    {0x1C, 0x22, 0x41, 0x41, 0x51, 0x22, 0x5C}, // Q
    {0x7F, 0x09, 0x09, 0x09, 0x19, 0x29, 0x46}, // R
    {0x26, 0x49, 0x49, 0x49, 0x49, 0x49, 0x32}, // S
    {0x01, 0x01, 0x01, 0x7F, 0x01, 0x01, 0x01}, // T
    {0x1F, 0x20, 0x40, 0x40, 0x40, 0x20, 0x1F}, // U
    {0x0F, 0x10, 0x20, 0x40, 0x20, 0x10, 0x0F}, // V
    {0x7F, 0x20, 0x10, 0x08, 0x10, 0x20, 0x7F}, // W
    {0x41, 0x22, 0x14, 0x08, 0x14, 0x22, 0x41}, // X
    {0x01, 0x02, 0x04, 0x78, 0x04, 0x02, 0x01}, // Y
    {0x41, 0x61, 0x51, 0x49, 0x45, 0x43, 0x41}, // Z
    {0x00, 0x00, 0x7F, 0x41, 0x41, 0x00, 0x00}, // [
    {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40}, // '\'
    {0x00, 0x00, 0x41, 0x41, 0x7F, 0x00, 0x00}, // ]
    {0x00, 0x04, 0x02, 0x01, 0x02, 0x04, 0x00}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40}, // _
#endif
#ifdef LOWERCASE
    {0x00, 0x00, 0x01, 0x03, 0x06, 0x00, 0x00}, // ` 0x60
    {0x30, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A, 0x7C}, // a
    {0x7F, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x28}, // c
    {0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x54, 0x54, 0x58}, // e
    {0x00, 0x08, 0x08, 0x7E, 0x09, 0x09, 0x00}, // f
    {0x0C, 0x52, 0x52, 0x52, 0x52, 0x52, 0x3E}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x04, 0x04, 0x78}, // h
    {0x00, 0x00, 0x00, 0x7D, 0x00, 0x00, 0x00}, // I
    {0x30, 0x40, 0x40, 0x40, 0x40, 0x40, 0x3D}, // j
    {0x7F, 0x20, 0x10, 0x18, 0x24, 0x42, 0x42}, // k
    {0x00, 0x00, 0x00, 0x7F, 0x40, 0x00, 0x00}, // l
    {0x7C, 0x04, 0x04, 0x78, 0x04, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x14, 0x14, 0x08}, // p 0x70
    {0x08, 0x14, 0x14, 0x14, 0x14, 0x14, 0x7C}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x54, 0x54, 0x24}, // s
    {0x00, 0x04, 0x04, 0x3F, 0x44, 0x44, 0x00}, // t
    {0x3C, 0x40, 0x40, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x0C, 0x10, 0x20, 0x40, 0x20, 0x10, 0x0C}, // v
    {0x7C, 0x20, 0x10, 0x08, 0x10, 0x20, 0x7C}, // w
    {0x44, 0x28, 0x10, 0x10, 0x10, 0x28, 0x44}, // x
    {0x04, 0x48, 0x50, 0x20, 0x10, 0x08, 0x04}, // y
    {0x44, 0x64, 0x54, 0x54, 0x54, 0x4C, 0x44}, // z
    {0x00, 0x00, 0x08, 0x36, 0x41, 0x00, 0x00}, // {
    {0x00, 0x00, 0x00, 0x77, 0x00, 0x00, 0x00}, // |
    {0x00, 0x00, 0x41, 0x36, 0x08, 0x00, 0x00}, // }
    {0x08, 0x04, 0x04, 0x08, 0x10, 0x10, 0x08}, // ~
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //(SP)
#endif
#ifdef EXT_CHR
    {0x00, 0x06, 0x09, 0x09, 0x09, 0x06, 0x00}, // ° 0x80
#endif
};

/*----------------------------------------*/
/* Excetion process routuine for SysTick. */
/* This routine called every 1ms.         */
/*----------------------------------------*/
extern "C" {
void SysTick_Handler(void) { L++; }
}

/*-------------------------------------*/
/* Initialize and start SysTick timer. */
/*-------------------------------------*/
void INIT_SysTick(void) {
  /* load value (sys_clk(24MHz) / 1000(1ms)) - 1 */
  /* 23999 = (24000000 / 1000) - 1 */
  /* max 24bit = 16777215 = 0.699Hz */
  SysTick->LOAD = SysTick_LOAD_RELOAD_Msk & 23999;
  /* clear current counter value */
  SysTick->VAL = 0;
  /* clear millis */
  L = 0;
  /* set clock_source,exception_request,enables_counter */
  /* clock source: 0b1xx = processor clock* */
  /*               0b0xx = external clock   */
  /* exception: 0b1x = enable* */
  /*            0b0x = disable */
  /* enables counter: 0b1 = enables counter* */
  /*                  0b0 = disable counter  */
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk |
                  SysTick_CTRL_ENABLE_Msk;
}

/*----------------------------*/
/* Return with SysTick count. */
/*----------------------------*/
uint32_t millis(void) { return L; }

/*---------------------------------------------*/
/* Blocking that waits for the specified time. */
/*---------------------------------------------*/
void delay(uint32_t Wait_ms) {
  volatile uint32_t end_ms;
  end_ms = millis() + Wait_ms;
  while (millis() < end_ms)
    ;
}

/*-----------------------------------------*/
/* Initialize GPIO's for USART and I2C     */
/* Anti Brick   PA0  #5                    */
/* SWC/I2C_SCL  PA2  #7  (AF6)             */
/* SWD/I2C_SDA  PB6  #8  (AF6)             */
/* USART_RX     PA4  #9  (AF1)             */
/* USART_TX     PA6  #10 (AF1)             */
/* Pin numbers are based on SOP14 package  */
/*-----------------------------------------*/
void INIT_GPIOs(void) {
  /* activate GPIOA and GPIOb for USART and SPI */
  RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN;

  /* Setting GPIOs for I2C */
  /* set alternate function I2C */
  /* set PA2 to AF6 (I2C_SCL) */
  GPIOA->AFR[0] = (GPIOA->AFR[0] & ~GPIO_AFRL_AFSEL2_Msk) | GPIO_AFRL_AFSEL2_2 |
                  GPIO_AFRL_AFSEL2_1;
  /* set PB6 to AF6 (I2C_SDA) */
  GPIOB->AFR[0] = (GPIOB->AFR[0] & ~GPIO_AFRL_AFSEL6_Msk) | GPIO_AFRL_AFSEL6_2 |
                  GPIO_AFRL_AFSEL6_1;

  /* settings for GPIO PA2(I2C_SCL) */
  /* set PA2(I2C_SCL) to multiplexed mode */
  GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE2_Msk) | GPIO_MODER_MODE2_1;
  /* set PA2(I2C_SCL) speed to very HIGH speed */
  GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED2_0 | GPIO_OSPEEDR_OSPEED2_1;
  /* activate pull-up */
  GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD2_Msk) | GPIO_PUPDR_PUPD2_0;

  /* settings for GPIO PB6(I2C_SDA) */
  /* set PB6(I2C_SDA) to multiplexed mode */
  GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE6_Msk) | GPIO_MODER_MODE6_1;
  /* set PB0(I2C_SDA) speed to very HIGH speed */
  GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED6_0 | GPIO_OSPEEDR_OSPEED6_1;
  /* activate pull-up */
  GPIOB->PUPDR = (GPIOB->PUPDR & ~GPIO_PUPDR_PUPD6_Msk) | GPIO_PUPDR_PUPD6_0;

  /* Setting GPIOs for USART1 */
  /* set PA4(RX) abd PA6(TX) to AF1 (USART1) */
  GPIOA->AFR[0] =
      (GPIOA->AFR[0] & ~(GPIO_AFRL_AFSEL4_Msk | GPIO_AFRL_AFSEL6_Msk)) |
      GPIO_AFRL_AFSEL4_0 | GPIO_AFRL_AFSEL6_0;

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

  /* Settings for GPIO PA1(used for debug LED) */
  /* Set PA1 to output mode */
  GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE1_Msk) | GPIO_MODER_MODE1_0;
  /* Set PA1 to push-pull mode */
  GPIOA->OTYPER &= ~GPIO_OTYPER_OT1;
  /* Set PA1 speed to very LOW speed */
  GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED1_0 | GPIO_OSPEEDR_OSPEED1_1);
  /* Set PA1 to no pull-up/pull-down */
  GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD1_Msk;
}

/*----------------------------------*/
/* Reset PA2 and PB6 as SWC and SWD */
/*----------------------------------*/
void RESET_SWD() {
  /* set PA2 to AF0 (SWC) */
  GPIOA->AFR[0] = 0;
  /* set PB6 to AF0 (SWD) */
  GPIOB->AFR[0] = 0;
  /* settings for GPIO PA2(SWC) */
  /* set PA2(SWC) to alternate function mode */
  GPIOA->MODER = 0x0000FFEF;
  GPIOA->OTYPER = 0x00000000;
  GPIOA->OSPEEDR = 0x00000000;
  GPIOA->PUPDR = 0x00000020;
  /* settings for GPIO PB6(SWD) */
  /* set PB6(SWD) to analog mode */
  GPIOB->MODER = 0x0000EFFF;
  GPIOB->OTYPER = 0x00000000;
  GPIOB->OSPEEDR = 0x00003000;
  GPIOB->PUPDR = 0x00001000;
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
  USART1->BRR =
      156 << USART_BRR_DIV_Mantissa_Pos | 4 << USART_BRR_DIV_Fraction_Pos;
  /* enable USART,transmitter,receiver */
  USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

/*-----------------------------------*/
/* Initialize and startup I2C module */
/*-----------------------------------*/
void INIT_I2C(void) {
  /* Turn on the I2C module */
  RCC->APBENR1 |= RCC_APBENR1_I2CEN;
  /* Set I2C mode(FM+) to I2C_SCL(PA2)*/
  SYSCFG->CFGR1 |= SYSCFG_CFGR1_I2C_PA2_FMP;
  /* Set I2C mode(FM+) to I2C_SDA(PB6)*/
  SYSCFG->CFGR1 |= SYSCFG_CFGR1_I2C_PB6_FMP;
  /* start I2C I/F */
  I2C1->CR1 |= I2C_CR1_START;

  /* Set I2C F/S(SM->0,FM->1),CCR_DUTY(2;1->0 or 16:9->1),CCR */
  /* CCR value:SM(duty 1:1)=24*10^6 / 2 * (100*10^3) =  120 */
  /*           FM(duty 2:1)=24*10^6 / 3 * (400*10^3) =   20 */
  /*           FM(duty16:9)=24*10^6 / 25 * (400*10^3) =   2.4->2 */
  I2C1->CCR = I2C_CCR_FS | 20;
  /* TRISE=1000ns(SM),300ns(FM) */
  /* Setting Value:24*16^6 * 300*10^-9 + 1 = 8.2-->8 */
  I2C1->TRISE = 8;

  /* I2C enable */
  I2C1->CR1 |= I2C_CR1_PE;
  I2C1->CR1 |= I2C_CR1_START;
}

/*--------------------------*/
/* Generate start condition */
/*--------------------------*/
void I2C_START_CONDITION() {
  I2C1->CR1 |= I2C_CR1_START;
  while (!(I2C1->SR1 & I2C_SR1_SB))
    ;
}

/*-------------------------*/
/* Generate stop condition */
/*-------------------------*/
void I2C_STOP_CONDITION() {
  I2C1->CR1 |= I2C_CR1_STOP;
  while (I2C1->SR1 & I2C_SR1_SB)
    ;
}

/*-------------------------------------*/
/* Check address was receveed by slave */
/*-------------------------------------*/
void I2C_WAIT_ACK() {
  while (!(I2C1->SR1 & I2C_SR1_ADDR))
    ;
  (void)I2C1->SR2; // Clear ADDR flag by reading SR2
}

/*-----------------------*/
/* Send one byte of data */
/*-----------------------*/
void I2C_SEND_BYTE(uint8_t data) { I2C1->DR = data; }

/*---------------------------------*/
/* Wait for stop condition detected*/
/* --for slave mode--              */
/*---------------------------------*/
void I2C_WAIT_STOP() {
  while (I2C1->CR1 & I2C_CR1_STOP)
    ;
}

/*-----------------------------*/
/* Wait for I2C bus to be free */
/*-----------------------------*/
void I2C_WAIT_BUSY() {
  while (I2C1->SR2 & I2C_SR2_BUSY)
    ;
}

/*---------------------------------*/
/* Wait for transmit buffer empty */
/*---------------------------------*/
void I2C_WAIT_TXE() {
  while (!(I2C1->SR1 & I2C_SR1_TXE))
    ;
}

/*-----------------------------------*/
/* Wait for receive buffer not empty */
/*-----------------------------------*/
void I2C_WAIT_RXNE() {
  while (!(I2C1->SR1 & I2C_SR1_RXNE))
    ;
}

/*---------------------------------*/
/* Wait for byte transfer finished */
/*---------------------------------*/
void I2C_WAIT_BTF() {
  while (!(I2C1->SR1 & I2C_SR1_BTF))
    ;
}

/*---------------------------------*/
/* Wait for stop condition detected*/
/*---------------------------------*/
void I2C_WAIT_STOPF() {
  while (!(I2C1->SR1 & I2C_SR1_STOPF))
    ;
}

/*-------------------------*/
/* Wait for address sended */
/*-------------------------*/
void I2C_WAIT_ADDR() {
  while (!(I2C1->SR1 & I2C_SR1_ADDR))
    ;
  (void)I2C1->SR2; // Clear ADDR flag by reading SR2
}

/*---------------------------------------*/
/* Wait for clear the reply failure flag */
/*---------------------------------------*/
void I2C_WAIT_AF() {
  while (I2C1->SR1 & I2C_SR1_AF)
    ;
}

/*--------------------------*/
/* Wait for get arbitration */
/*--------------------------*/
void I2C_WAIT_ARLO() {
  while (I2C1->SR1 & I2C_SR1_ARLO)
    ;
}

void I2C_WAIT_BERR() {
  while (I2C1->SR1 & I2C_SR1_BERR)
    ;
}

void I2C_SWRESET() {
  I2C1->CR1 |= I2C_CR1_SWRST;  // Set software reset bit
  I2C1->CR1 &= ~I2C_CR1_SWRST; // Clear software reset bit
}

/*---------------------------------------------*/
/* Wait for OverLoad or UnderLoad flag cleared */
/*---------------------------------------------*/
void I2C_WAIT_OVR() {
  while (I2C1->SR1 & I2C_SR1_OVR)
    ;
}

int main() {
  uint8_t i;
  /*+++++++++++++++++++++++ ANTI BRICK +++++++++++++++++++++++*/
  /*            ANTIBRICK avoids SWIO malfunctions.           */
  /*  GPIO PA0 connect to GND at boot time. MCU will looping. */
  /*       When looping, can connect programmer to SWIO.      */
  /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
  RCC->IOPENR = RCC_IOPENR_GPIOAEN; // Enable GPIOA clock
  /* set PA0 as input mode */
  GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE0);
  /* pull-up PA0 */
  GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD0) | GPIO_PUPDR_PUPD0_0;
  /* when looping PA0 is 0(low) */
  while (!(GPIOA->IDR & GPIO_IDR_ID0))
    ;
  /*----- conclude ANTI BRICK -----*/

  INIT_GPIOs();
  INIT_USART();
  /* start SysTick */
  INIT_SysTick();
  INIT_I2C();
  // INIT_ST7567();
  //  CLEAR_SCREEN();
  //  VREVERSE_ST7567(false);
  //  HREVERSE_ST7567(false);
  //  SETRR_ST7567(4);

  // PUT_CHR('A');
  // SET_CURSOR(3, 16);
  // SET_MAG(2, 1);
  // PUT_STR((char *)"Hello World!");
  // PUT_STR((char *)"**");
  // SET_CURSOR(4,0);
  // PUT_STR((char *)"abcdefghijklmnopqrstuvwxyz");
  // SET_MAG(8, 7);
  // SET_CURSOR(1,12);
  // PUT_STR((char *)"abcdefghijklmnopqrstuvwxyz");
  // PUT_CHR('A');
  // PUT_CHR('S');

  /* Turn on the debug LED*/
  GPIOA->BSRR = GPIO_BSRR_BS1;

  while (GPIOA->IDR & GPIO_IDR_ID0) {

    I2C_START_CONDITION();
    I2C_SEND_BYTE(0x55); // Send slave address with write bit
    /* Turn off the debug LED*/
    GPIOA->BSRR = GPIO_BSRR_BR1;
    // I2C_WAIT_ACK();      // Wait for ACK from slave
    I2C_STOP_CONDITION();

    delay(1);
  }

  RESET_SWD();

  while (1) {
    // BACK_LIGHT(true);
    // INVERSE_ST7567(false);
    // delay(5000); // Wait for 1 second
    // INVERSE_ST7567(true);
    // delay(1000);
    /* check overrun error abd clear overrun error */
    // while(USART1->SR & USART_SR_ORE) (void)USART1->DR;
    /* receive data from USART1 */
    // if (USART1->SR & USART_SR_RXNE) {
    //     char received_char = USART1->DR; // read received data
    //     /* echo back the received character */
    //     while (!(USART1->SR & USART_SR_TXE)); // wait until transmit data
    //     register is empty USART1->DR = received_char; // send back the
    //     received character
    //     /* wait for transmission to complete */
    //     while (!(USART1->SR & USART_SR_TC));
    // }
    /* transmit data via USART1 */
    // if (USART1->SR & USART_SR_TXE) {
    //     USART1->DR = 'A'; // send character 'A' continuously
    //     /* wait for transmission to complete */
    //     while (!(USART1->SR & USART_SR_TC));
    // }
  }
}