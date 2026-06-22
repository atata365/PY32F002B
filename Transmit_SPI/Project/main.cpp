/* Testprogram for transmit from SPI interface */
/* Using GPIOs */
/*----------------------------------------*/
/* Initialize GPIO's for USART and SPI    */
/* SPI_NSS  PA1  #6  (Software driven)    */
/* SPI_MOSI PA0  #5  (AF0)                */
/* SPI_SCK  PB0  #4  (AF0)                */
/* Cmd/Dat  PB1  #3  (Software driven)    */
/* Reset    PB2  #2  (Software driven)    */
/* USART_RX PA4  #9  (AF1)                */
/* USART_TX PA6  #10 (AF1) use for debug  */
/* Pin numbers are based on SOP14 package */
/*----------------------------------------*/

#include "RTE_Components.h"
#include CMSIS_device_header

volatile uint32_t L;

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

/*----------------------------------------*/
/* Initialize GPIO's for USART and SPI    */
/* SPI_NSS  PA1  #6  (Software driven)    */
/* SPI_MOSI PA0  #5  (AF0)                */
/* SPI_SCK  PB0  #4  (AF0)                */
/* Cmd/Dat  PB1  #3  (Software driven)    */
/* Reset    PB2  #2  (Software driven)    */
/* USART_RX PA4  #9  (AF1)                */
/* USART_TX PA6  #10 (AF1) use for debug  */
/* Pin numbers are based on SOP14 package */
/*----------------------------------------*/
void INIT_GPIOs(void) {
    /* activate GPIOA and GPIOb for USART and SPI */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN;

    /* Setting GPIOs for SPI */
    /* set alternate function PA0(MOSI), PB0(SCK) to AF0 (SPI) */
    GPIOA->AFR[0] = (GPIOA->AFR[0] & ~GPIO_AFRL_AFSEL0_Msk);
    /* set PB0(SCK) AF0 (SPI) */
    GPIOB->AFR[0] = (GPIOB->AFR[0] & ~GPIO_AFRL_AFSEL0_Msk);

    /* settings for GPIO PA1(NSS) */
    /* set PA1(NSS) to multiplexed mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE1_Msk) | GPIO_MODER_MODE1_0;
    /* set PA1(NSS) speed to very HIGH speed */
    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED1_0 | GPIO_OSPEEDR_OSPEED1_1;
    /* activate pull-up */
    GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD1_Msk) | GPIO_PUPDR_PUPD1_0;

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
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM | SPI_CR1_BR_0 | SPI_CR1_BR_1;
     /* enable SPI1 */
    SPI1->CR1 |= SPI_CR1_SPE;
    /* Inactive NSS(HIGH) */
    GPIOA->BSRR = GPIO_BSRR_BS1;
}

void SPI_Transmit(uint8_t code) {
    /* wait until transmit buffer is empty */
    while (!(SPI1->SR & SPI_SR_TXE));
    /* send data */
    SPI1->DR = code;
    /* wait until transmission is complete */
    while (SPI1->SR & SPI_SR_BSY);
}

int main() {
    INIT_GPIOs();
    INIT_USART();
    /* start SysTick */
    INIT_SysTick();
    INIT_SPI();
    SPI_Transmit(0);
    
    while (1) {
        /* Activate NSS(LOW) */
        GPIOA->BSRR = GPIO_BSRR_BR1;
        SPI_Transmit(0x55); // send 0x55 continuously
        SPI_Transmit(0xaa); // send 0xaa continuously
        SPI_Transmit(0x1); // send 0x55 continuously
        SPI_Transmit(0x2); // send 0xaa continuously
        SPI_Transmit(0x3); // send 0x55 cont    inuously
        SPI_Transmit(0x4); // send 0xaa continuously
        /* Inactive NSS(HIGH) */
        GPIOA->BSRR = GPIO_BSRR_BS1;
        delay(1); // wait for 1 second

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