/********************************************************************************<br/>
//  MSP430FR5969 Demo - LM71 Temperature Sensor<br/>
//<br/>
//  Description: This program reads the LM71 (SPI) and shows the readings in the PC terminal(UART).<br/> 
//<br/>
//  ACLK = 32768Hz, MCLK =  SMCLK = default DCO 1 MHz<br/>
//<br/>
//                MSP430FR5969<br/>
//             -----------------<br/>
//            |                 |--LFXTIN (32768Hz reqd.)<br/>
//            |                 |--LFXTOUT<br/>
//            |                 |<br/>
//       RST -|     P2.0/UCA0TXD|----> PC<br/>
//            |                 |<br/>
//            |     P2.1/UCA0RXD|<---- PC<br/>
//            |                 |<br/>
//            |    P1.7/UCB0MISO|<---- (SPI MISO)LM71 TEMP. SENSOR<br/>
//            |                 |<br/>
//            |        P2.2/SCLK|<---- (SPI CLOCK)LM71 TEMP. SENSOR<br/>
//            |                 |<br/>
//            |          P2.4/SC|<---- (SPI SC)LM71 TEMP. SENSOR<br/>
//            |                 |<br/>
//            |            P1.0|<---- Green LED<br/>
//            |                 |<br/>
//<br/>
//<br/>
//		LM71 SPI/MICROWIRE 13-Bit Plus Sign Temperature Sensor<br/>
//		The LM71 has 13-bit plus sign temperature resolution (0.03125°C per LSB) while operating<br/> 
//		over a temperature range of −40°C to +150°C.<br/>
//<br/>
//<br/>
//   Claudionor Santos<br/>
//   Brazil   Campinas/SP<br/>
//   April 2017<br/>
//   Built with Code Composer Studio V6.1.1 */<br/>