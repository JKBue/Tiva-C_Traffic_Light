// Timer0A.c
// Runs on Tiva-C

// Adapted from SysTick.c from the book:
/* "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 4.7
*/

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "Timer0A.h"

uint32_t sysClkFreq = 80000000; // Assume 80 MHz clock by default

// Set clock freq. so Timer0A_Wait10ms delays for exactly 10 ms if clock is not 80 MHz
void Timer0A_Init( uint32_t clkFreq ){
  sysClkFreq = clkFreq;
  return;
}

// Time delay using busy wait
// The delay parameter is in units of the core clock (units of 12.5 nsec for 80 MHz clock)
//   Adapted from Program 9.8 from the book:
/*   "Embedded Systems: Introduction to ARM Cortex-M Microcontrollers",
     ISBN: 978-1477508992, Jonathan Valvano, copyright (c) 2013
     Volume 1, Program 9.8
*/
void Timer0A_Wait( uint32_t delay ){

  if(delay <= 1){ return; } // Immediately return if requested delay less than one clock

  SYSCTL_RCGCTIMER_R |= 0x00000001;  // 0) Activate Timer0
  TIMER0_CTL_R &= ~0x00000001;       // 1) Disable Timer0A during setup
  TIMER0_CFG_R = 0;                  // 2) Configure for 32-bit timer mode
  TIMER0_TAMR_R = 1;                 // 3) Configure for one-shot mode
  TIMER0_TAILR_R = delay - 1;        // 4) Specify reload value
  TIMER0_TAPR_R = 0;                 // 5) No prescale
  TIMER0_IMR_R = 0;                  // 6-9) No interrupts
  TIMER0_CTL_R |= 0x00000001;        // 10) Enable Timer0A

  //while( TIMER0_TAR_R ){} // Doesn't work; Wait until timer expires (value equals 0)
  // Or, clear interrupt and wait for raw interrupt flag to be set
  TIMER0_ICR_R = 1;
  while( !(TIMER0_RIS_R & 0x1) ){}
  return;
}

// Time delay using busy wait
// This assumes 80 MHz system clock
void Timer0A_Wait1ms( uint32_t delay ){
  uint32_t i;
  for( i = 0; i < delay; i++ ){
    Timer0A_Wait(sysClkFreq/1000);  // wait 1ms
  }
  return;
}
