/********************************************************************************
//  MSP430FR5969 Demo - LED BRIGHTNESS
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