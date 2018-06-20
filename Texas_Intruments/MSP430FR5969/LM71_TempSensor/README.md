/********************************************************************************
//  MSP430FR5969 Demo - LM71 Temperature Sensor
//
//  Description: This program reads the LM71 (SPI) and shows the readings in the PC terminal(UART). 
//
//  ACLK = 32768Hz, MCLK =  SMCLK = default DCO 1 MHz
//
//                MSP430FR5969
//             -----------------
//            |                 |--LFXTIN (32768Hz reqd.)
//            |                 |--LFXTOUT
//            |                 |
//       RST -|     P2.0/UCA0TXD|----> PC
//            |                 |
//            |     P2.1/UCA0RXD|<---- PC
//            |                 |
//            |    P1.7/UCB0MISO|<---- (SPI MISO)LM71 TEMP. SENSOR
//            |                 |
//            |        P2.2/SCLK|<---- (SPI CLOCK)LM71 TEMP. SENSOR
//            |                 |
//            |          P2.4/SC|<---- (SPI SC)LM71 TEMP. SENSOR
//            |                 |
//            |            P1.0|<---- Green LED
//            |                 |
//
//
//		LM71 SPI/MICROWIRE 13-Bit Plus Sign Temperature Sensor
//		The LM71 has 13-bit plus sign temperature resolution (0.03125°C per LSB) while operating 
//		over a temperature range of −40°C to +150°C.