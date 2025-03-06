#include <stdint.h>
#include "SysTick.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"


struct State {
        uint32_t Out;  // output
        uint32_t Time; // how long to wait
        const struct State *Next[4];
}; // points to next state

typedef const struct State STyp;

#define goS     &FSM[0]
#define waitS   &FSM[1]
#define goW     &FSM[2]
#define waitW   &FSM[3]
STyp FSM[4]={
             {0x21,300,{goS,waitS,goS,waitS}},
             {0x22, 50,{goW,goW,goW,goW}},
             {0x0C,300,{goW,goW,waitW,waitW}},
             {0x14, 50,{goS,goS,goS,goS}}
}; // {output, how long, order of states}


void GPIOPortB_Init() {
    SYSCTL_RCGC2_R |= 0b10;        // 1) turns on clock B
    GPIO_PORTB_AMSEL_R &= ~(0xFF); // 3) disable analog function on PB5-0
    GPIO_PORTB_PCTL_R &= ~(0xFF);  // 4) enable regular GPIO
    GPIO_PORTB_DIR_R |= 0xFF;      // 5) outputs on PB5-0
    GPIO_PORTB_AFSEL_R &= ~(0xFF); // 6) regular function on PB5-0
    GPIO_PORTB_DEN_R |= 0xFF;      // 7) enable digital on PB5-0
}

void GPIOPortE_Init() {
    SYSCTL_RCGC2_R |= 0b10000;     // 1) turns on clock for E
    GPIO_PORTE_AMSEL_R &= ~(0xFF); // 3) disable analog function on PE1-0
    GPIO_PORTE_PCTL_R &= ~(0xFF);  // 4) enable regular GPIO
    GPIO_PORTE_DIR_R &= ~(0xFF);   // 5) inputs on PE1-0
    GPIO_PORTE_AFSEL_R &= ~(0xFF); // 6) regular function on PE1-0
    GPIO_PORTE_DEN_R |= 0xFF;      // 7) enable digital on PE1-0
}

//  if pb2  or the data reg with 0b0100 then wait then use and to clear it
void srclk() {
    GPIO_PORTB_DATA_R |= 0b0010;
    SysTick_Wait10ms(1);
    GPIO_PORTB_DATA_R &= ~(0b0010); // PB1
}

void rclk() {
    GPIO_PORTB_DATA_R |= 0b1000;
    SysTick_Wait10ms(1);
    GPIO_PORTB_DATA_R &= ~(0b1000); // PB3
}

// make a clk to pulse pins, put on ser
void LedOn (uint32_t Light) {
    int i;
    uint32_t Light2;
    for (i = 0; i <= 7; i++) {
        if (i == 6 || i == 7) {     // loads 0 into QA and QB
            Light = 0b0;
            SysTick_Wait10ms(1);
            srclk();
        } else {                    // loads new bit into QA and pushes prev bit to next Q#
            GPIO_PORTB_DATA_R &= ~(0b1);
            Light2 = Light >> i;
            Light2 &= 0x01;
            GPIO_PORTB_DATA_R |= Light2;
            SysTick_Wait10ms(1);
            srclk();
        }
    }
    rclk();
}


void main(void){

    STyp *Pt; // state pointer
    uint32_t Input;

    PLL_Init(Bus80MHz); // 50 MHz, Prog 4.6
    SysTick_Init(); // Program 4.7
    GPIOPortE_Init();
    GPIOPortB_Init();

    Pt = goS; // set to first initial state

    while(1){
        LedOn(Pt->Out); // set lights
        SysTick_Wait10ms(Pt->Time);
        uint32_t SENSOR = GPIO_PORTE_DATA_R &= 0b11;
        Input = SENSOR; // read sensors (buttons); put data reg here
        Pt = Pt->Next[Input];
    }

}
