/********************************************************************************
//  MSP430FR59x Demo - LED BRIGHTNESS
//
//  Description: This program generates two PWM outputs on P1.0 using
//  Timer0_A configured for up mode. The value in CCR0, 100-1, defines the PWM
//  period and the values in CCR1 the PWM duty cycles. Using 32768Hz
//  ACLK as TACLK, the timer period is ~3ms.
//  Every 106,82 ms the Duty Cycle is incremented by 5 (STEP_DUTY_CYCLE) until reaches 100%(MAX_DUTY_CYCLE),
//  then the duty cycle is subtracted by 5 until reach 0%(MIN_DUTY_CYCLE),
//  then the duty cycle is incremented again.
//	Every time that Duty Cycle reaches MAX_DUTY_CYCLE LED1 (red LED)  blinks.
//
//  ACLK = TACLK = 32768Hz, SMCLK = MCLK = DCO = 8MHz
//
//
//          MSP430FR5969
//         ---------------
//     /|\|            XIN|-
//      | |               |  32KHz Crystal
//      --|RST        XOUT|-
//        |               |
//        |     P1.0/TA0.1|--> LED1 GREEN 	- CCR1 - PWM N.C
//        |     	  P4.6|--> LED2 RED		-
//        |               |
//
//   Claudionor Santos
//   Brazil   Campinas/SP
//   April 2017
//   Built with Code Composer Studio V6.1.1 */
//*******************************************************************************

#include "msp430.h"
#define MAX_DUTY_CYCLE 100
#define MIN_DUTY_CYCLE 0
#define STEP_DUTY_CYCLE 5

//*******************************************************************************
//************ GLOBAL VARIABLES   ***********************************************

int timerA_count = 0;
int timerA_count_AUX = 0;
short int timerA_tick = 0;

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;           // Stop WDT

  // Configure GPIO
  P1DIR |=  BIT0;                     // P1.0 output
  P1SEL0 |=  BIT0;                    // P1.0 option select
  P1SEL1 &= ~ BIT0;

  PJSEL0 |= BIT4 | BIT5;

  P4DIR |= BIT6;                     // P4.6 output
  P4OUT |= BIT6;					 // P4.6 High  turn On LED2 RED

  // Disable the GPIO power-on default high-impedance mode to activate
  // previously configured port settings
  PM5CTL0 &= ~LOCKLPM5;

  // XT1 Setup
  CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
  CSCTL1 = DCOFSEL_6;                       // Set DCO to 8MHz
  CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK; // Set ACLK = XT1; MCLK = DCO
  CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
  CSCTL4 &= ~LFXTOFF;
  do
  {
    CSCTL5 &= ~LFXTOFFG;                    // Clear XT1 fault flag
    SFRIFG1 &= ~OFIFG;
  }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
  CSCTL0_H = 0;                             // Lock CS registers

  // Configure Timer0_A
  TA0CCR0 = 100-1;                          // PWM Period = (100 * 30,52us) = 3052 us = 3.052 ms
  TA0CCTL0 |= CCIE;
  TA0CCTL1 |= OUTMOD_7 ;               		// CCR1 reset/set		**LED2 LAUNCHPAD

  TA0CCR1 = MAX_DUTY_CYCLE;                     // CCR1 PWM duty cycle	**LED2 GREEN LAUNCHPAD
  TA0CTL = TASSEL__ACLK | MC__UP | TACLR ;   	// ACLK =32,768KHz,
  	  	  	  	  	  	  	  	  	  	  	  	//up mode,
  	  	  	  	  	  	  	  	  	  	  	  	//clear TAR,
  	  	  	  	  	  	  	  	  	  	  	  	//Timer A counter interrupt enable

  __bis_SR_register(GIE);       				// General enable interrupt

  while(1){
	  if (timerA_tick){
		  timerA_tick= 0;
		  if(timerA_count_AUX <= 19){  //0 to 19 -> 20 steps -> 20 x STEP_DUTY_CYCLE = MAX_DUTY_CYCLE
		 		    		   TA0CCR1 -= STEP_DUTY_CYCLE;// Decrease Duty Cycle
		 		    		   P4OUT &= ~BIT6;// Turn off LED1  (red LED)
		 		    	   }
		 		    	   else
		 		    	   {
		 		    		if(timerA_count_AUX == 40){
		 		    			timerA_count_AUX =0;
		 		    			TA0CCR1 = MAX_DUTY_CYCLE;
		 		    			P4OUT |= BIT6;// Turn ON LED1  (red LED)
		 		    		}
		 		    		else{
		 		    			   TA0CCR1 += STEP_DUTY_CYCLE;// Increase Duty Cycle
		 		    		}
		 		    	   }
		 		    timerA_count_AUX ++;
	  }

  }
}

// Timer0_A0 interrupt service routine  every  T= 3052 us = 3.052 ms
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0_ISR (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer0_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
	if(timerA_count == (35-1)){
		timerA_count = 0;		//
		timerA_tick = 1;  		// 35 x 3,052ms = 106,82ms -> time base for change duty cycle
	}							//
	else{
		timerA_count ++;
	}
}
