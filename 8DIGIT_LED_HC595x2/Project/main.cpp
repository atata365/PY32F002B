/* test program for 8digit LED display with 74HC595x2*/
/* used GPIOs PB0(DIO), PB1(CLK), PB2(LATCH) */
/* PIN assign PB0:#5, PB1:#6, PB2:#7 (SOP14 package) */
#include "RTE_Components.h"
#include CMSIS_device_header

/* counter, incremented every counter reset */
volatile uint32_t COUNT;

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

/* TIM14 Interrupt Handler (IRQn=14) */
extern "C" 
    __attribute__((interrupt)) void TIM14_IRQHandler(void) {
    COUNT++;
    /* clear update interrupt flag */
    TIM14->SR &= ~TIM_SR_UIF;
}

void TIM14init() {
    /* clear counter */
    TIM14->CNT = 0;
    /* activate TIM14 */
    RCC->APBENR2 |= RCC_APBENR2_TIM14EN;
    /* prescaler (make counter freq) */
    /* 0..65535(16bit) */
    /* (24000000(24MHz) / 6) = 4MHz */
    TIM14->PSC = 6;
    /* ARR(Auto Reload Resister) = 0..65535(16bit) */ 
    /* 4MHz/5=800kHz = 1.25us cycle*/
    TIM14->ARR = TIM_ARR_ARR_Msk & 4;
    /* activate TIM14 counter*/
    TIM14->CR1 = TIM_CR1_CEN;
    /* activate TIM14 BRK_UP_TRG_COM */
    TIM14->DIER |= TIM_DIER_UIE;
    /* enable interrupts in NVIC */
    NVIC->ISER[0] |= (1 << TIM14_IRQn);
}

void GPIOinit(){
    /* activate GPIOA */
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    /* set PA0 snd PA1 to General purpose output mode */
    GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1))
     | GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0;
    /* set PA0 and PA1 to push-pull mode */
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1);
    /* set PA0 and PA1 to high */
    GPIOA->BSRR |= GPIO_BSRR_BS0 | GPIO_BSRR_BS1;
}

void wait() {
    /* reset counter */
    COUNT = 0;
    TIM14->CNT = 0;
    /* wait 1 cycle */
    while(COUNT < 1);
}

/* send 8 bit data */
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
        /* wait for 100ns(Setup time) */
        wait();
        /* set(HIGH) PA0(clock) */
        GPIOA->BSRR |= GPIO_BSRR_BS0;
        /* wait for 100ms(HOLD time) */
        wait();
        data >>= 1;
    }
    /* reset PA0(clock) */
    GPIOA->BSRR |= GPIO_BSRR_BR0;
    /* reset_PA1(data)*/
    GPIOA->BSRR |= GPIO_BSRR_BR1;
    /* wait 100ns for data transfer end */
    wait();
    /* wait for ACK(discare ACK) */
    /* set PA0(clock) */
    GPIOA->BSRR |= GPIO_BSRR_BS0;
    /* wait for 200ns(discared ACK) */
    wait();
    /* reset PA0(clock) */
    GPIOA->BSRR |= GPIO_BSRR_BR0;
}

int main() {
    /* select clock source=HSI */
    RCC->CR = RCC_CR_HSION;
    /* wait for HSI stabirised */
    while(!(RCC->CR & RCC_CR_HSIRDY));
    /* initialize GPIO */
    GPIOinit();
    /* initialize TIM14*/
    TIM14init();
    int i = 0,j = 0,k = 0,l = 0;
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
        }


        send_data(digit_code[l]); 
        send_data(digit_code[k]); // with colon, 2nd digit & 0x80
        send_data(digit_code[j]); 
        send_data(digit_code[i]); 
    }
}   