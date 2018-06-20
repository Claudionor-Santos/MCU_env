/********************************************************************************<br/>
//  MSP430FR5969 Demo - LED BRIGHTNESS<br/>
//<br/>
//  Description: This program generates two PWM outputs on P1.0 using<br/>
//  Timer0_A configured for up mode. The value in CCR0, 100-1, defines the PWM<br/>
//  period and the values in CCR1 the PWM duty cycles. Using 32768Hz<br/>
//  ACLK as TACLK, the timer period is ~3ms.<br/>
//  Every 106,82 ms the Duty Cycle is incremented by 5 (STEP_DUTY_CYCLE) until reaches 100%(MAX_DUTY_CYCLE),<br/>
//  then the duty cycle is subtracted by 5 until reach 0%(MIN_DUTY_CYCLE),<br/>
//  then the duty cycle is incremented again.<br/>
//	Every time that Duty Cycle reaches MAX_DUTY_CYCLE LED1 (red LED)  blinks.<br/>
//<br/>
//  ACLK = TACLK = 32768Hz, SMCLK = MCLK = DCO = 8MHz<br/>
//<br/>
//<br/>
//          MSP430FR5969<br/>
//         ---------------<br/>
//     /|\|            XIN|-<br/>
//      | |               |  32KHz Crystal<br/>
//      --|RST        XOUT|-<br/>
//        |               |<br/>
//        |     P1.0/TA0.1|--> LED1 GREEN 	- CCR1 - PWM N.C<br/>
//        |     	  P4.6|--> LED2 RED		-<br/>
//        |               |<br/>
//<br/>
//   Claudionor Santos<br/>
//   Brazil   Campinas/SP<br/>
//   April 2017<br/>
//   Built with Code Composer Studio V6.1.1 */<br/>
//*******************************************************************************<br/>