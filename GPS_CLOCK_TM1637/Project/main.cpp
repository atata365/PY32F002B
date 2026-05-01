/* UART repeater */
/* This program is maken for PY32F002B SOP14 package */
/* It's a simple UART repeater that forwards data received */ 
/* on USART1 to the same USART1 */
/* fixed baud rate: 9600 bps */
/* data receive port as PA4(USART RX #9) */
/* repeat data port as PA6(USART TX #10) */
/* port numbers are based on SOP14 package */
#include "RTE_Components.h"
#include CMSIS_device_header

#define buffer_size 1024
#define work_buffer_size 80
#define max_touple_length 12
volatile uint8_t received_data[buffer_size]; // variable to store received data
volatile uint16_t data_index = 0; // index for received data
volatile uint16_t data_length = 0; // length of received data
volatile uint8_t data_lines = 0; // count of newline characters in received data

/* voratile resister for sys tick count */
volatile uint32_t L;

const uint8_t digit_code[10] = {
    0b01011100, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111100, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01100111, // 9
};

/*--------------------------------*/
/* ISR for SysTick                */
/* This routine called every 1ms. */
/*--------------------------------*/
extern "C" {
    void SysTick_Handler(void) {
        L++;
    }
}

/*-------------------------------------*/
/* initSysTick                         */
/* start SysTick interrupt and counter */
/*-------------------------------------*/
void initSysTick(void) {
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

/*-----------------------------*/
/* return current milliseconds */
/*-----------------------------*/
uint32_t millis(void) {
    return L;
}

/*-------------------------*/
/* blocking delay function */
/*-------------------------*/
void delay(uint32_t Wait_ms) {
    volatile uint32_t end_ms;
    end_ms = millis() + Wait_ms;
    while(millis() < end_ms);
}

/*-------------------------*/
/* ISR for USART1(receive) */
/*-------------------------*/
extern "C" void USART1_IRQHandler() {
    /* check overrun error and clear overrun error */
    if (USART1->SR & USART_SR_ORE) (void)USART1->DR;
    /* receive data from USART1 */
    if (USART1->SR & USART_SR_RXNE) {
        char received_char = USART1->DR; // read received data
        //USART1->SR &= ~USART_SR_RXNE; // clear receive data register not empty flag
        received_data[data_index++] = received_char; // store received data
        data_length++;
        if (data_index >= buffer_size) data_index = 0; // wrap around if buffer is full
        if (data_length >= buffer_size) data_length = buffer_size; // cap data length at buffer size
        if (received_char == '\n') { // if newline character is received, set data ready flag
            data_lines++; // increment newline character count
        }
    }
}

/*--------------------------------------------------------*/
/* initialize GPIO PA4(USART RX #9) and PA6(USART TX #10) */
/* pin nubbers are based on SOP14 package                 */
/*--------------------------------------------------------*/
void INIT_GPIOs() {
    /* initialize GPIO for USART (PA4 , PA6) */
    /* enable GPIOA clock */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    /* set PA6(TX) and PA4(RX) to AF1 (USART1) */
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
    GPIOA->OTYPER |= GPIO_OTYPER_OT6;
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
/* pin nubbers are based on SOP14 package */
void INIT_USART() {
    /* enable USART1 clock */
    RCC->APBENR2 |= RCC_APBENR2_USART1EN;
    /* set baud rate to 9600 bps (assuming 24 MHz clock) */
    /* USARTDIV = 24 MHz / (16 * 9600) = 156.25 */
    /* USART_BRR_DIV_Mantissa = 156 */
    /* USART_BRR_DIV_Fraction = 0.25 * 16 = 4 */ 
    USART1->BRR = 156 << USART_BRR_DIV_Mantissa_Pos | 4 << USART_BRR_DIV_Fraction_Pos;
    /* enable USART,transmitter,receiver */
    USART1->CR1 |= USART_CR1_UE |USART_CR1_TE | USART_CR1_RE;
    /* enable USART1 receive interrupt */
    USART1->CR1 |= USART_CR1_RXNEIE;
    /* set USART1 interrupt priority and enable USART1 interrupt in NVIC */
    NVIC->IPR[USART1_IRQn] = 0; // set highest priority
    /* enable interrupts in NVIC */
    NVIC->ISER[0] = (1 << USART1_IRQn);
}

/*-------------------------------------------------*/
/* extract a specific tuple from the source buffer */
/*-------------------------------------------------*/
int get_toupe(uint8_t *source,uint8_t *dest,uint8_t num_touple) {
    uint8_t i, j = 0, current_touple = 0;  
    for(i = 0;i < work_buffer_size;i++){
        if(source[i] == ',') {
            current_touple++;
            continue;
        }
        if((source[i] == '\n') || (current_touple > num_touple)) break;
        if(current_touple == num_touple){   
            dest[j++] = source[i];
        }           
    }
    return j; // return length of the extracted touple
}

/*----------------------*/
/* make start condition */
/*----------------------*/
void start_condition() {
    /* make start condition */
    /* reset PA1(data) */
    GPIOA->BSRR |= GPIO_BSRR_BR1;
    /* wait */
    __NOP();
    /* reset PA0(clock) */
    GPIOA->BSRR |= GPIO_BSRR_BR0;
    /* wait */
    __NOP();
}

/*---------------------*/
/* make stop condition */
/*---------------------*/
void stop_condition() {
    /* make stop condition */
    /* set PA0(clock) */
    GPIOA->BSRR |= GPIO_BSRR_BS0;
    /* wait */
    __NOP();
    /* set PA1(data) */
    GPIOA->BSRR |= GPIO_BSRR_BS1;
    /* wait */
    __NOP();
}

/*-----------------*/
/* send 8 bit data */
/*-----------------*/
void send_data(uint8_t data){
    for(int i=0; i<8; i++){
        /* reset(LOW) PA0(clock) */
        GPIOA->BSRR |= GPIO_BSRR_BR0;
        /* set or reset data bit(PA1) */
        if(data & 1){
            /* set PA1(data) */
            GPIOA->BSRR |= GPIO_BSRR_BS1;
        } else {
            /* reset PA1(data) */
            GPIOA->BSRR |= GPIO_BSRR_BR1;
        }
        /* wait */
        __NOP();
        /* set(HIGH) PA0(clock) */
        GPIOA->BSRR |= GPIO_BSRR_BS0;
        /* wait for 100ms(HOLD time) */
        __NOP();
        data >>= 1;
    }
    /* reset PA0(clock) */
    GPIOA->BSRR |= GPIO_BSRR_BR0;
    /* reset_PA1(data)*/
    GPIOA->BSRR |= GPIO_BSRR_BR1;
    /* wait */
    __NOP();
    /* wait for ACK(discare ACK) */
    /* set PA0(clock) */
    GPIOA->BSRR |= GPIO_BSRR_BS0;
    /* wait */
    __NOP();
    /* reset PA0(clock) */
    GPIOA->BSRR |= GPIO_BSRR_BR0;
}

/*-----------------------------------------*/
/* display control - on/off and brightness */
/*-----------------------------------------*/
void display_control(uint8_t on_off,uint8_t brightness){
    uint8_t command;
    start_condition();
    /* command bit 0b1000xyyy */
    /* x=0 : disply off */
    /* x=1 : display on */
    /* yyy=000 : brightness 1/16 */
    /* yyy=001 : brightness 2/16 */
    /* yyy=010 : brightness 4/16 */
    /* yyy=011 : brightness 10/16 */
    /* yyy=100 : brightness 11/16 */
    /* yyy=101 : brightness 12/16 */
    /* yyy=110 : brightness 13/16 */
    /* yyy=111 : brightness 14/16 */
    /* comand bit set */
    command =0b10000000;
    if(on_off){
        command |= 0b00001000; /* display on */
    }
    command |= (brightness & 0b00000111); /* set brightness */
    send_data(command);
    stop_condition();
}

/*------------------------------------*/
/* column mode - 0:automatic, 1:fixed */
/*------------------------------------*/
void col_mode(uint8_t address_mode){
    uint8_t command;
    /* command bit 0b0100xyzz */
    /* x=0 : normal_mode */
    /* x=1 : test mode */
    /* y=0 : auto increment mode */
    /* y=1 : fixed address mode */
    /* zz=00 : write data to display */
    /* zz=10 : read key mode */
    /* comand bit set */
    command = 0b01000000;
    if(address_mode){
        command |= 0b00000100; // fixed address/
    }
    start_condition();
    send_data(command);
    stop_condition();
}

/*-------------------------*/
/* set column position     */
/* parameter can set 0...5 */
/*-------------------------*/
void set_col(uint8_t col){
    start_condition();
    send_data(0b11000000 | (col & 0b00000111));
}

void display_time(int hh, int mm) {
    int i = hh / 10; // tens digit of hours
    int j = hh % 10; // units digit of hours
    int k = mm / 10; // tens digit of minutes
    int l = mm % 10; // units digit of minutes
    col_mode(0); // auto increment mode
    set_col(0); // set column to 0
    send_data(digit_code[i]); 
    send_data(digit_code[j]); // with colon, 2nd digit & 0x80
    send_data(digit_code[k]); 
    send_data(digit_code[l]); 
    stop_condition();
    display_control(1,2); // display on, brightness 4/16
}

int main() {
    uint8_t work_buffer[work_buffer_size]; // buffer for processing data
    int i = 0; // index for work buffer
    int j = 0; // index for sending data
    uint16_t work_length; // length of data in work buffer
    uint16_t work_index; // index for processing data in work buffer
    uint8_t touple_buffer[max_touple_length]; // buffer for extracted touple
    int hh, mm, ss; // variables for time components

    INIT_GPIOs();
    INIT_USART();
    initSysTick(); // initialize SysTick for timing functions
    while (1) {
        /* check if data is ready to be sent */
        if (data_lines > 0) { // check if there are lines of data to be sent
            data_lines--; // decrement line count
            work_index = data_index; // calculate start index of data in buffer
            work_length = data_length; // calculate length of data to be sent
            if(work_index < work_length) work_index = work_index + buffer_size - work_length;
            else work_index -= work_length;
        	for(i = 0;i < work_buffer_size;i++){
                if(work_index >= buffer_size) work_index = 0;
	        	work_buffer[i] = received_data[work_index]; // copy data to work buffer
		        work_index++;
		        if(work_buffer[i] == '\n') {
                    i++; // include newline character in work buffer
                    break; // stop copying if newline character is found
                }
	        }
            data_length -= i; // update data length after processing
            i = get_toupe(work_buffer,touple_buffer,0); // get record indication
            if((touple_buffer[0] == '$')
             & (touple_buffer[1] == 'G')
             & (touple_buffer[2] == 'N')
             & (touple_buffer[3] == 'G')
             & (touple_buffer[4] == 'G')
             & (touple_buffer[5] == 'A')) {
                j = get_toupe(work_buffer,touple_buffer,1); // get time
                hh = (touple_buffer[0] - '0') * 10 + (touple_buffer[1] - '0');
                mm = (touple_buffer[2] - '0') * 10 + (touple_buffer[3] - '0');
                ss = (touple_buffer[4] - '0') * 10 + (touple_buffer[5] - '0');
                hh = (hh + 9) % 24; // convert UTC to JST (UTC+9);
                for(i = 0;i < j;i++) {
                    USART1->DR = touple_buffer[i]; // send data from work buffer
                    /* wait for transmission to complete */
                    while (!(USART1->SR & USART_SR_TC));
                }
                USART1->DR = '\n'; // send newline character after sending touple
                /* wait for transmission to complete */
                while (!(USART1->SR & USART_SR_TC));
            }
        }
    }
}