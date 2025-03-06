#include <stdint.h>
#include "SysTick.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"


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

void GPIOPortF_Init(void) {
    SYSCTL_RCGCGPIO_R |= 0x20;          // 1) Activate clock for Port F; set bit 5 to turn on clock for Port F
    SysTick_Wait10ms(1);                // Allow time for clock to stabilize
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;  // (2) Unlock GPIO Port F Commit Register
    GPIO_PORTF_CR_R = 0xFF;             // Enable commit for Port F
    GPIO_PORTF_AMSEL_R = 0x00;          // (3) Disable analog functionality
    GPIO_PORTF_PCTL_R = 0x00000000;     // (4) Configure Port F as GPIO
    GPIO_PORTF_DIR_R = 0x0E;            // (5) PF0 and PF4 input, PF3-1 output
    GPIO_PORTF_AFSEL_R = 0x00;          // 6) Regular port function; Disable alternate function
    GPIO_PORTF_PUR_R = 0x11;            // Enable weak pull-up on PF0 and PF4
    GPIO_PORTF_DEN_R = 0xFF;            // (7) Enable digital I/O on Port F
}

/* Seven-segment display counter
 *
 * This program counts number 0-3 on the seven segment display.
 * The seven segment display is driven by a shift register which is
 * connected to SSI2 in SPI mode.
 *
 * Built and tested with Keil MDK-ARM v5.28 and TM4C_DFP v1.1.0
 */

// #include "TM4C123.h" // use diff header files

void delayMs(int n);
void sevenseg_init(void);
void SSI2_write(unsigned char data);

int main(void) {
    const static unsigned char digitPattern[] = {0x90, 0x80, 0xF8, 0x82, 0x92, 0x99, 0xB0, 0xA4, 0xF9, 0xC0};
    int i = 0;
    int x = 0;

    sevenseg_init();    // initialize SSI2 that connects to the shift registers

    while(1) {
        SSI2_write(digitPattern[i]);    // write digit pattern to the seven segments
        SSI2_write((1 << x));           // select digit
        if (++i > 9) {
            i = 0;
        }
        if (++x > 3){
            x = 0;
        }
        delayMs(2);                     // 1000 / 60 / 4 = 4.17
    }
}

// enable SSI2 and associated GPIO pins (need to change to fit our board)
void sevenseg_init(void) {
    SYSCTL_RCGCGPIO_R |= 0x02;   // enable clock to GPIOB (change first part to SYSCTC_RCGCGPIO_R)
    SYSCTL_RCGCGPIO_R |= 0x04;   // enable clock to GPIOC
    SYSCTL_RCGCSSI_R |= 0x04;    // enable clock to SSI2

    // PORTB 7, 4 for SSI2 TX and SCLK
    GPIO_PORTB_AMSEL_R &= ~0x90;      // turn off analog of PORTB 7, 4
    GPIO_PORTB_AFSEL_R |= 0x90;       // PORTB 7, 4 for alternate function
    GPIO_PORTB_PCTL_R &= ~0xF00F0000; // clear functions for PORTB 7, 4
    GPIO_PORTB_PCTL_R |= 0x20020000;  // PORTB 7, 4 for SSI2 function
    GPIO_PORTB_DEN_R |= 0x90;         // PORTB 7, 4 as digital pins (change first part to GPIO_PORTB_DEN_R)

    // PORTC 7 for SSI2 slave select
    GPIO_PORTC_AMSEL_R &= ~0x80;      // disable analog of PORTC 7
    GPIO_PORTC_DATA_R |= 0x80;        // set PORTC 7 idle high
    GPIO_PORTC_DIR_R |= 0x80;         // set PORTC 7 as output for SS
    GPIO_PORTC_DEN_R |= 0x80;         // set PORTC 7 as digital pin

    SSI2_CR1_R = 0;              // turn off SSI2 during configuration
    SSI2_CC_R = 0;               // use system clock
    SSI2_CPSR_R = 16;            // clock prescaler divide by 16 gets 1 MHz clock
    SSI2_CR0_R = 0x0007;         // clock rate div by 1, phase/polarity 0 0, mode freescale, data size 8
    SSI2_CR1_R = 2;              // enable SSI2 as master
}


// This function enables slave select, writes one byte to SSI2,
// wait for transmit complete and deassert slave select.
void SSI2_write(unsigned char data) {
    GPIO_PORTC_DATA_R &= ~0x80;       // assert slave select
    SSI2_DR_R = data;            // write data
    while (SSI2_SR_R & 0x10) {}  // wait for transmit done
    GPIO_PORTC_DATA_R |= 0x80;        // deassert slave select
}

// get rid of below
/* delay n milliseconds (50 MHz CPU clock) */
void delayMs(int n) {
    int i, j;
    for(i = 0 ; i< n; i++)
        for(j = 0; j < 6265; j++)
            {}  /* do nothing for 1 ms */
}

