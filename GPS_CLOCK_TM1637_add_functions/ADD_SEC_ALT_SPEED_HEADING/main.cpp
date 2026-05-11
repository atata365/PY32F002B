/* minimum GPS clock */
/* This program is maken for PY32F002B SOP14 package. */
/* receve GPS(NMEA) data from NEO-6M(or simular) and */
/* display time to the TM1637 based 4-digit 7-segment display. */ 
/* - USART settings - */
/* fixed baud rate: 9600 bps */
/* - connections -*/
/* data receive port as PA4(#9 USART RX) */
/* data transmit port as PA6(#10 USART TX) for debugging */
/* display clock port as PB0(#4 clock to TX1637) */
/* display data port as PB1(#3 data to TX1637) */
/* brightness change and mode cange switch connect to PA0 */
/* port numbers are based on SOP14 package */
/* - operation - */
/* long press the switch to change LED brightness */
/* short press the switch to change display mode */
/* mode 0: display hours and minutes */
/* mode 1: display minutes and seconds */
/* mode 2: display altitude */
/* mode 3: display speed */
/* mode 4: display heading */
#include "RTE_Components.h"
#include CMSIS_device_header

#define buffer_size 1024
#define work_buffer_size 80
#define max_touple_length 12
#define modes 5
#define avoid_chettring_time 100 // debounce time in milliseconds
#define long_press_time 500 // long press duration in milliseconds 
#define max_brightness 8 // maximum brightness level (for long press)
/* voratile register for receive data buffer */
volatile uint8_t received_data[buffer_size]; // buffer to store received data
volatile uint16_t data_index = 0; // index for received data
volatile uint16_t data_length = 0; // length of received data
volatile uint8_t data_lines = 0; // count of newline characters in received data
/* voratile register for LED display */
volatile uint32_t colon_blink; // variable for colon blinking control
volatile uint8_t brightness = 2; // variable for brightness control
/* voratile register for EXTI(SW) */
volatile uint16_t avoid_chettering = avoid_chettring_time; // debounce time in milliseconds
volatile uint16_t long_press_duration = long_press_time; // long press duration in milliseconds
volatile uint32_t last_SysTick = 0; // last SysTick value for debounce and long press detection
volatile uint8_t current_mode = 0; // variable for mode control
volatile bool mode_changed = false; // flag to indicate mode change
/* voratile register for SysTick count */
volatile uint32_t L;

const uint8_t digit_code[] = {
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
    0b00000000, // 10:[blank]
    0b01110111, // 11:A
    0b00111000, // 12:L
    0b01111000, // 13:t
    0b01110011, // 14:P
    0b01011110, // 15:d
    0b01110110, // 16:H
    0b01111001, // 17:E
    0b01000000  // 18:-
};

/*--------------------------------*/
/* ISR for SysTick                */
/* This routine called every 1ms. */
/*--------------------------------*/
extern "C" void SysTick_Handler(void) {
    L++;
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
    while (USART1->SR & USART_SR_ORE) (void)USART1->DR;
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
/* pin numbers are based on SOP14 package                 */
/*--------------------------------------------------------*/
void INIT_GPIOs() {
    /* enable GPIOA(USART1 TX/RX,mode SW) and GPIOB(clock and data for TM1637) */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN;
    /* initialize GPIO for USART (PA4 , PA6) */
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
    //GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE6_Msk) | GPIO_MODER_MODE6_1; 
    /* change PA6(TX) to push-pull mode */
    //GPIOA->OTYPER |= GPIO_OTYPER_OT6;
    /* set PA6(TX) speed to very HIGH speed */
    //GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED6_0 | GPIO_OSPEEDR_OSPEED6_1;
    /* activate pull-up */
    //GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD6_Msk) | GPIO_PUPDR_PUPD6_0;
    /* initialize GPIOB for TM1637 (PB0:clock , PB1:data) */
    /* set PB0 and PB1 to output mode */
    GPIOB->MODER = (GPIOB->MODER & ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk))
                                     | GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0;
    /* set PB0 and PB1 OTYPE to push-pull */
    GPIOB->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1); /* set PB0 and PB1 to push-pull mode */
    /* set PB0 and PB1 speed to very HIGH speed */
    GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED0_0 | GPIO_OSPEEDR_OSPEED0_1
                     | GPIO_OSPEEDR_OSPEED1_0 | GPIO_OSPEEDR_OSPEED1_1;
    /* initialize PA0 for mode change switch */
     /* set PA0 to input mode */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE0_Msk);
    /* activate pull-up for PA0 */
    GPIOA->PUPDR = (GPIOA->PUPDR & ~GPIO_PUPDR_PUPD0_Msk) | GPIO_PUPDR_PUPD0_0;
}

/*-------------------*/
/* initialize USART1 */
/*-------------------*/
/* USART1 TX -> PA6(#10) */
/* USART1 RX -> PA4(#9) */
/* pin numbers are based on SOP14 package */
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
    /* reset PB1(data) */
    GPIOB->BSRR = GPIO_BSRR_BR1;
    /* wait */
    __NOP();
    /* reset PB0(clock) */
    GPIOB->BSRR = GPIO_BSRR_BR0;
    /* wait */
    __NOP();
}

/*---------------------*/
/* make stop condition */
/*---------------------*/
void stop_condition() {
    /* make stop condition */
    /* set PB0(clock) */
    GPIOB->BSRR = GPIO_BSRR_BS0;
    /* wait */
    __NOP();
    /* set PB1(data) */
    GPIOB->BSRR = GPIO_BSRR_BS1;
    /* wait */
    __NOP();
}

/*-----------------*/
/* send 8 bit data */
/*-----------------*/
void send_data(uint8_t data){
    for(int i=0; i<8; i++){
        /* reset(LOW) PB0(clock) */
        GPIOB->BSRR = GPIO_BSRR_BR0;
        /* set or reset data bit(PB1) */
        if(data & 1){
            /* set PB1(data) */
            GPIOB->BSRR = GPIO_BSRR_BS1;
        } else {
            /* reset PB1(data) */
            GPIOB->BSRR = GPIO_BSRR_BR1;
        }
        /* wait */
        __NOP();
        /* set(HIGH) PB0(clock) */
        GPIOB->BSRR = GPIO_BSRR_BS0;
        /* wait for 100ms(HOLD time) */
        __NOP();
        data >>= 1;
    }
    /* reset PB0(clock) */
    GPIOB->BSRR = GPIO_BSRR_BR0;
    /* reset_PB1(data)*/
    GPIOB->BSRR = GPIO_BSRR_BR1;
    /* wait */
    __NOP();
    /* wait for ACK(discare ACK) */
    /* set PB0(clock) */
    GPIOB->BSRR = GPIO_BSRR_BS0;
    /* wait */
    __NOP();
    /* reset PB0(clock) */
    GPIOB->BSRR = GPIO_BSRR_BR0;
}

/*-----------------------------------------*/
/* display control - on/off and brightness */
/*-----------------------------------------*/
void display_control(uint8_t on_off,uint8_t bright){
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
    command |= (bright & 0b00000111); /* set brightness */
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

/*------------------------*/
/* display time on TM1637 */
/*------------------------*/
void display_time(int hh, int mm, bool colon) {
    int m = hh / 10; // tens digit of hours
    int n = hh % 10; // units digit of hours
    int o = mm / 10; // tens digit of minutes
    int p = mm % 10; // units digit of minutes
    col_mode(0); // auto increment mode
    set_col(0); // set column to 0
    send_data(digit_code[m]);
    if(colon) {
        send_data(digit_code[n] | 0b10000000); // add colon bit to the units digit of hours
    } else {
        send_data(digit_code[n]);
    }
    send_data(digit_code[o]); 
    send_data(digit_code[p]); 
    stop_condition();
    display_control(1,brightness); // display on, brightness 4/16
}

/*---------------------------*/
/* display 4-digit on TM1637 */
/*---------------------------*/
void display_4_digit(uint8_t *col_data) {
    col_mode(0); // auto increment mode
    set_col(0); // set column to 0
    send_data(digit_code[col_data[0]]);
    send_data(digit_code[col_data[1]]);
    send_data(digit_code[col_data[2]]);
    send_data(digit_code[col_data[3]]);
    stop_condition();
    display_control(1,brightness); // display on, brightness 4/16
}

/*--------------------------------------------------*/
/* get a line of data from the received data buffer */
/*--------------------------------------------------*/
int get_a_line(uint8_t *dest, int max_length) {
    int i; // index for destination buffer
    int work_index; // index for processing data in work buffer
    int work_length; // length of data to be processed in work buffer
    
    /* check if data is ready to be sent */
    if (data_lines > 0) { // check if there are lines of data to be sent
        data_lines--; // decrement line count
        work_index = data_index; // calculate start index of data in buffer
        work_length = data_length; // calculate length of data to be sent
        if(work_index < work_length) work_index = work_index + buffer_size - work_length;
        else work_index -= work_length;
        for(i = 0;i < max_length;i++){
            if(work_index >= buffer_size) work_index = 0;
            dest[i] = received_data[work_index]; // copy data to work buffer
            work_index++;
            if(dest[i] == '\n') {
                i++; // include newline character in work buffer
                break; // stop copying if newline character is found
            }
        }
        data_length -= i; // update data length after processing
        return i; // return length of the line copied to work buffer}
    }
    return 0; // return 0 if no data is ready
}

void clear_buffer(void) {
    data_length = 0; // reset data length
    data_lines = 0; // reset line count
}

/*---------------------------------------------------------------------*/
/* ISR for EXTI0_1 (EXTI line 0 and 1)                                 */
/* This routine is called when an interrupt occurs on EXTI line 0 or 1 */
/*---------------------------------------------------------------------*/
extern "C"  void EXTI0_1_IRQHandler(void) {
    /* check EXTI interrupt line */
    if(EXTI->PR & EXTI_PR_PR0) {
        /* clear EXTI interrupt flag */
        EXTI->PR = EXTI_PR_PR0;
        /* check rising edge*/
        if(!(GPIOA->IDR & GPIO_IDR_ID0)) {
            /* avoid chettering  */
            if(L - last_SysTick > avoid_chettering) {
                /* rising edge*/
                last_SysTick = L; // update last SysTick value
            }
        } else {
            /* falling edge */
            if((L - last_SysTick) < long_press_duration) {
                /* short press */
                mode_changed = true; // set mode change flag
                current_mode = (current_mode + 1) % modes; // toggle mode
            } else {
                /* long press */
                brightness = (brightness + 2) % max_brightness; // change brightness
                mode_changed = false; // set mode change flag to false for long press
            }
        }
    }
}

/*----------------------------*/
/* activate EXTI0_1 interrupt */
/*----------------------------*/
void activate_EXTI(void) {
    /* initialize EXTI for PA0 */
    /* set EXTICR1(External interrupt select register 1) */
    /* Ex. Px0:EXTI_EXTICR1_EXTI0_Msk */
    /*     0x1:EXTI_EXTICR1_EXTI1_Msk */
    /*                : */
    /*     0x7:EXTI_EXTICR1_EXTI7_Msk */
    /*     PAx:0 */
    /*     PBx:EXTI_EXTICR1_EXTIx_0 */
    /*     PCx:EXTI_EXTICR1_EXTIx_1 (PC have line 0 or 1) */
    EXTI->EXTICR[0] = (EXTI->EXTICR[0] & ~EXTI_EXTICR1_EXTI0_Msk) | 0;
    /* unmask EXTI line 0 */
    EXTI->IMR |= EXTI_IMR_IM0;
    /* trigger on both rising and falling edge */
    EXTI->RTSR |= EXTI_RTSR_RT0; // rising edge trigger
    EXTI->FTSR |= EXTI_FTSR_FT0; // falling edge trigger
    /* enable interrupts in NVIC */
    NVIC->ISER[0] |= (1 << EXTI0_1_IRQn);
}

/*----------------------*/
/* get specified record */
/*----------------------*/
int check_record_header(uint8_t *record_header, uint8_t *work) {
    int i;  
    for(i = 0;i < work_buffer_size;i++){
        if(work[i] != record_header[i]) break; // if character does not match, break the loop
    }
    return i; // return matched characters count
}

int main() {
    uint8_t work_buffer[work_buffer_size]; // buffer for processing data
    uint8_t touple_buffer[max_touple_length]; // buffer for extracted touple
    int hh, mm, ss; // variables for time components
    int i, j; // index variable for loops
    uint8_t col_data[4]; // buffer for column data to be displayed

    INIT_GPIOs(); // initialize GPIOs for USART and TM1637
    col_data[0] = 18;
    col_data[1] = 18;
    col_data[2] = 18;
    col_data[3] = 18;
    display_4_digit(col_data); // display "----" on TM1637 at startup
    initSysTick(); // initialize SysTick for timing functions
    activate_EXTI(); // activate EXTI for mode change switch
    INIT_USART(); // initialize USART for receiving GPS data
    while (1) {
        switch(current_mode) {
            case 0:
                /* display HH:MM*/
                if(get_a_line(work_buffer, work_buffer_size) > 0) { // get a line of data from the received data buffer
                    if(check_record_header((uint8_t *)"$GNGGA", work_buffer) == 6) { // check if the record is GNGGA
                        if(get_toupe(work_buffer,touple_buffer,1) == 10) { // get time
                            colon_blink = millis(); // reset colon blink timer
                            hh = (touple_buffer[0] - '0') * 10 + (touple_buffer[1] - '0');
                            mm = (touple_buffer[2] - '0') * 10 + (touple_buffer[3] - '0');
                            hh = (hh + 9) % 24; // convert UTC to JST (UTC+9);
                            display_time(hh, mm, true); // display time on TM1637
                            mode_changed = false; // reset mode change flag
                        }
                    }
                }
                if((millis() - colon_blink > 500) && (colon_blink != 0)) { // toggle colon every 500ms
                    colon_blink = 0; // reset colon blink timer
                    display_time(hh, mm, false); // toggle colon off
                }
                break;
            case 1:
                /* display MM:SS */
                if(get_a_line(work_buffer, work_buffer_size) > 0) { // get a line of data from the received data buffer
                    if(check_record_header((uint8_t *)"$GNGGA", work_buffer) == 6) { // check if the record is GNGGA
                        if(get_toupe(work_buffer,touple_buffer,1) == 10) { // get time
                            mm = (touple_buffer[2] - '0') * 10 + (touple_buffer[3] - '0');
                            ss = (touple_buffer[4] - '0') * 10 + (touple_buffer[5] - '0');
                            display_time(mm, ss, true); // display time on TM1637
                            mode_changed = false; // reset mode change flag
                        }
                    }
                }
                break;
            case 2:
                /* display altitude */
                if(get_a_line(work_buffer, work_buffer_size) > 0) { // get a line of data from the received data buffer
                    if(check_record_header((uint8_t *)"$GNGGA", work_buffer) == 6) { // check if the record is GNGGA
                        i = get_toupe(work_buffer,touple_buffer,9); // get altitude
                        if(i >= 3) {
                            if(mode_changed) {
                                col_data[0] = 11;
                                col_data[1] = 12;
                                col_data[2] = 13;
                                col_data[3] = 10;
                                display_4_digit(col_data); // display "ALt " on TM1637
                                mode_changed = false; // reset mode change flag
                            } else {
                                for(j = 0;j < i;j++) if((touple_buffer[j]) == '.') break; // stop if decimal point is found
                                i = 4 - j; // calculate number of digits to be displayed after decimal point
                                for(j = 0;j < i;j++) col_data[j] = 10; // shift digits to the right
                                j = 0; // index for digits to be displayed
                                for(;i < 4;i++) col_data[i] = touple_buffer[j++] - '0'; // convert character to digit and store in column data buffer
                                display_4_digit(col_data); //
                            }
                        }
                    }
                }
                break;
            case 3:
                /* display speed */
                if(get_a_line(work_buffer, work_buffer_size) > 0) { // get a line of data from the received data buffer
                    if(check_record_header((uint8_t *)"$GNVTG", work_buffer) == 6) { // check if the record is GNGGA
                        i = get_toupe(work_buffer,touple_buffer,7); // get speed
                        if(i >= 3) {
                            if(mode_changed) {
                                col_data[0] = 5;
                                col_data[1] = 14;
                                col_data[2] = 15;
                                col_data[3] = 10;
                                display_4_digit(col_data); // display "SPd " on TM1637
                                mode_changed = false; // reset mode change flag
                            } else {
                                for(j = 0;j < i;j++) if((touple_buffer[j]) == '.') break; // stop if decimal point is found
                                i = 4 - j; // calculate number of digits to be displayed before decimal point
                                for(j = 0;j < i;j++) col_data[j] = 10; // shift digits to the right
                                j = 0; // index for digits to be displayed
                                for(;i < 4;i++) col_data[i] = touple_buffer[j++] - '0'; // convert character to digit and store in column data buffer
                                display_4_digit(col_data); //
                            }
                        }
                    }
                }
                break;
            case 4:
                /* display heading */
                if(get_a_line(work_buffer, work_buffer_size) > 0) { // get a line of data from the received data buffer
                    if(check_record_header((uint8_t *)"$GNVTG", work_buffer) == 6) { // check if the record is GNGGA
                        i = get_toupe(work_buffer,touple_buffer,1); // get heading
                        if(i >= 3) {
                            if(mode_changed) {
                                col_data[0] = 16;
                                col_data[1] = 17;
                                col_data[2] = 15;
                                col_data[3] = 10;
                                display_4_digit(col_data); // display "HEd " on TM1637
                                mode_changed = false; // reset mode change flag
                            } else {
                                for(j = 0;j < i;j++) if((touple_buffer[j]) == '.') break; // stop if decimal point is found
                                i = 4 - j; // calculate number of digits to be displayed before decimal point
                                for(j = 0;j < i;j++) col_data[j] = 10; // shift digits to the right
                                j = 0; // index for digits to be displayed
                                for(;i < 4;i++) col_data[i] = touple_buffer[j++] - '0'; // convert character to digit and store in column data buffer
                                display_4_digit(col_data); //
                            }
                        }
                    }
                }
                break;
            default:
                break;
        }
    }
}