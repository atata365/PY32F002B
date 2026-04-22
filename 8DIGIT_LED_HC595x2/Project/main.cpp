/* test program for 8digit LED display with 74HC595x2 */
/* used GPIOs PB0(DIO), PB1(CLK), PB2(LATCH) */
/* PIN assign PB0:#4, PB1:#3, PB2:#2 (SOP14 package) */
#include "RTE_Components.h"
#include CMSIS_device_header

/* counter, incremented every counter reset */
volatile uint32_t COUNT;

/* numeric patten for 7seg. LED */
const uint8_t digit_code[] = {
    0b00111010, // 0
    0b01100000, // 1
    0b11011010, // 2
    0b11110010, // 3
    0b01100110, // 4
    0b10110110, // 5
    0b00111110, // 6
    0b11100000, // 7
    0b11111110, // 8
    0b11100110, // 9
    0b00000010, // -
    0b10011100, // [
    0b11110000  // ]
};

/* convert table for digit position to bit pattern */
const uint8_t digit_pos[ ] = {
    0b11101111, // position 0(10^0)
    0b11011111, // position 1
    0b10111111, // position 2
    0b01111111, // position 3
    0b11111110, // position 4
    0b11111101, // position 5
    0b11111011, // position 6
    0b11110111  // position 7
};

/* activate GPIO and set mode */
/* set PB0,PB1,PB2 to push-pull output mode */
void GPIOinit(){
    /* activate GPIOB */
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    /* set PB0, PB1, PB2 to general purpose output mode */
    GPIOB->MODER = (GPIOB->MODER & ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE2))
                 | GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0 | GPIO_MODER_MODE2_0;
    /* set PB0, PB1, PB2 to push-pull mode */
    GPIOB->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1 | GPIO_OTYPER_OT2);
    /* set PB0, PB1, PB2 to low */
    GPIOB->BSRR |= GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2;
}

/* send 8bit data to 74CH595 */
void send_data(uint8_t data){
    for(int i = 0; i < 8; i++ ) {
        /* set or reset DIO pin(PB0) */
        data & 1 ? GPIOB->BSRR |= GPIO_BSRR_BR0 : GPIOB->BSRR |= GPIO_BSRR_BS0;
        /* set(HIGH) PB1(CLK) */
        GPIOB->BSRR |= GPIO_BSRR_BS1;
        /* reset(low) PB1(CLK) */
        GPIOB->BSRR |= GPIO_BSRR_BR1;
        data >>= 1;
    }
}

/* send column position and digit data to 74HC795 */
/* arguments #1 unit8_t col:cloumn position */
/*           #2 uint8_t data:number for display 0..9 and '-[]' */
/*           #3 bool dot:true=show dot,false=no show dot */
void send_col_data(uint8_t col,uint8_t data,bool dot) {
    /* turn off the latch(turn off the LEDs) */
    GPIOB->BSRR = GPIO_BSRR_BR2;
    send_data(dot ? digit_code[data] | 1 : digit_code[data]);
    send_data(digit_pos[col]);
    /* turn on the latch(turn on the LEDs) */
    GPIOB->BSRR = GPIO_BSRR_BS2;
}

int main() {
    /* select clock source=HSI */
    RCC->CR = RCC_CR_HSION;
    /* wait for HSI stabirised */
    while(!(RCC->CR & RCC_CR_HSIRDY));
    /* initialize GPIO */
    GPIOinit();
    int i = 0,j = 0,k = 0,l = 0,m = 0,n = 0,o = 0,p = 0;
    while(1){
        i++;
        if(i >= 10){
            i = 0;
            j++;
        }
        if(j >= 10){
            j = 0;
            k++;
        }
        if(k >= 10){
            k = 0;
            l++;
        }
        if(l >= 10){
            l = 0;
            m++;
        }
        if(m >= 10){
            m = 0;
            n++;
        }
        if(n >= 10){
            n = 0;
            o++;
        }
        if(o >= 10){
            o = 0;
            p++;
        }
        if(p >= 10){
            p = 0;
        }
        send_col_data(7,p,false);
        send_col_data(6,o,false);
        send_col_data(5,n,false);
        send_col_data(4,m,false);
        send_col_data(3,l,false);
        send_col_data(2,k,false);
        send_col_data(1,j,false);
        send_col_data(0,i,false);
    }
}   