/* UART repeater */
/* This program is maken for PY32F002B SOP14 package */
/* It's a simple UART repeater that forwards data received */ 
/* on USART1 to the same USART1 */
/* fixed baud rate: 9600 bps */
/* data receive port as PA4(USART RX #9) */
/* ->connect to GPS module TX */
/* repeat data port as PA6(USART TX #10) */
/* ->connect to RX port as usb-serial module RX */
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

int main() {
    uint8_t work_buffer[work_buffer_size]; // buffer for processing data
    int i = 0; // index for work buffer
    int j = 0; // index for sending data
    uint16_t work_length; // length of data in work buffer
    uint16_t work_index; // index for processing data in work buffer
    uint8_t touple_buffer[max_touple_length]; // buffer for extracted touple

    INIT_GPIOs();
    INIT_USART();
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