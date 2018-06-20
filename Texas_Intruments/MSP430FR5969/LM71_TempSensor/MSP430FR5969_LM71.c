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
//		The LM71 has 13-bit plus sign temperature resolution (0.03125°C per LSB) while operating over a temperature range of −40°C to +150°C.
//
//             -----------------
//            |                 |
//            |           VCC   |--Red wire
//            |           GND   |--Purple wire
//            |           SI/O  |--White wire
//            |           SCLK  |--Orange wire
//            |           SC    |--Yelow wire
//            |                 |


#include <msp430.h>

//FUNCTIONs PROTOTYPEs
void delay_us(unsigned int delay);
void send_message(int port, int len);


unsigned char MST_Data, SLV_Data1, SLV_Data2, secondbyte_received=0;
unsigned int SLV_DataTT , SLV_DataTTaux;
unsigned long Temp_Celsius;
unsigned char num_10k, num_1k, num_100 , num_10 , num_1;
unsigned char count_ISR_onesec, flag_send_string;

float Valor_temp;


typedef struct
{
    char buf[40];
    char ptr;
    char len;
} serial_msg_buf_t;

serial_msg_buf_t tx_msg;


int main(void)
{
  volatile unsigned int i;

  WDTCTL = WDTPW + WDTHOLD;			// Stop watchdog timer
  // Configure GPIO
  P1DIR |= BIT0;                    // Set P1.0 as output -> Green LED
  P1SEL1 |= BIT6 | BIT7;            // Configure SOMI and MISO
  P2SEL1 |= BIT0 | BIT1;            // USCI_A0 UART operation
  P2SEL1 |= BIT2;                   // Configure and clk
  PJSEL0 |= BIT4 | BIT5;            // For XT1

  P2DIR |= BIT4; // LM71 -> sLAVE cHIP SELECT
  P2OUT |= BIT4; // LM71 -> SLAVE CS DISABLED

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // XT1 Setup
    CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
    CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
    CSCTL4 &= ~LFXTOFF;
    do
    {
      CSCTL5 &= ~LFXTOFFG;                    // Clear XT1 fault flag
      SFRIFG1 &= ~OFIFG;
    }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
    CSCTL0_H = 0;                             // Lock CS registers

    // Configure USCI_A0 for UART mode -> 9600 bps
    UCA0CTLW0 = UCSWRST;                      // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__ACLK;                // CLK = ACLK
    UCA0BR0 = 3;                              // 9600 baud
    UCA0MCTLW |= 0x5300;                      // 32768/9600 - INT(32768/9600)=0.41
                                              // UCBRSx value = 0x53 (See UG)
    UCA0BR1 = 0;
    UCA0CTL1 &= ~UCSWRST;                     // Initialize eUSCI

    // Configure USCI_B0 for SPI operation -> LM71 
    UCB0CTLW0 = UCSWRST;                      // **Put state machine in reset**
                                              // 3-pin, 8-bit SPI master
    UCB0CTLW0 |= UCMST | UCSYNC |  UCMSB | UCMODE_1;
                                              // Clock polarity high, MSB
    UCB0CTLW0 |= UCSSEL__SMCLK;                // SMCLK
    UCB0BR0 = 0x02;                           // /2
    UCB0BR1 = 0;                              //
    UCB0CTLW0 &= ~UCSWRST;                    // **Initialize USCI state machine**
	
    UCB0IE &= ~UCRXIE;                         // Enable USCI_B0 -> SPI RX interrupt
	__bis_SR_register( GIE);       // CPU off, enable interrupts

    // Configure RTC_C
    RTCCTL01 = RTCTEVIE | RTCRDYIE | RTCBCD | RTCHOLD;
                                            // RTC enable, BCD mode, RTC hold
                                            // enable RTC read ready interrupt
                                            // enable RTC time event interrupt

    RTCYEAR = 0x2016;                       // Year = 0x2016
    RTCMON = 0x7;                           // Month = 0x07 = July
    RTCDAY = 0x31;                          // Day = 0x05 = 5th
    RTCDOW = 0x07;                          // Day of week = 0x01 = Monday
    RTCHOUR = 0x10;                         // Hour = 0x13 -> 1 PM
    RTCMIN = 0x00;                          // Minute = 0x00
    RTCSEC = 0x00;                          // Seconds = 0x00

    RTCADOWDAY = 0x2;                       // RTC Day of week alarm = 0x2
    RTCADAY = 0x20;                         // RTC Day Alarm = 0x20
    RTCAHOUR = 0x10;                        // RTC Hour Alarm
    RTCAMIN = 0x23;                         // RTC Minute Alarm

    RTCCTL01 &= ~(RTCHOLD);                 // Start RTC


  P2OUT |= BIT4;                            // Now with SPI signals initialized,
  MST_Data = 0x01;                          // Initialize data values (any value)
  SLV_Data1 = 0x00;							//data slave clear
  count_ISR_onesec =0 ;						//1 second periodic counter clear
  flag_send_string =0 ;						//flag 2 seconds clear
  //***	Send string TO pc TERMINAL -> "LM71 TEMPERATURE SENSOR"	***
  tx_msg.buf[0]='L';tx_msg.buf[1]='M';tx_msg.buf[2]='7';tx_msg.buf[3]='1';  tx_msg.buf[4]=' ';
  tx_msg.buf[5]='T';tx_msg.buf[6]='E';tx_msg.buf[7]='M';tx_msg.buf[8]='P'; tx_msg.buf[9]='E'; tx_msg.buf[10]='R';
  tx_msg.buf[11]='A';tx_msg.buf[12]='T';tx_msg.buf[13]='U';tx_msg.buf[14]='R';tx_msg.buf[15]='E';tx_msg.buf[16]=' ';
  tx_msg.buf[17]='S';tx_msg.buf[18]='E';tx_msg.buf[19]='N';tx_msg.buf[20]='S';tx_msg.buf[21]='O';tx_msg.buf[22]='R';
  tx_msg.buf[23]='\r';
  tx_msg.buf[24]='\n';
  tx_msg.len = 25;
  send_message(0,tx_msg.len);	// Send array (TX_MSG) to uart_A0  
  while (tx_msg.ptr != 0);		//  
  
   while(1){
	   // SEND STRING TO PC  -> "LM71 TEMP SENSOR"
			while(flag_send_string == 0); // waiting for 2 second period -> RTC ISR
			flag_send_string = 0;
			//***	Send string TO pc TERMINAL -> "LM71 TEMP SENSOR"	***
	   	    tx_msg.buf[0]='L';tx_msg.buf[1]='M';tx_msg.buf[2]='7';tx_msg.buf[3]='1';tx_msg.buf[4]=' ';
	   	   	tx_msg.buf[5]='T';tx_msg.buf[6]='E';tx_msg.buf[7]='M';tx_msg.buf[8]='P';tx_msg.buf[9]=' ';
	   	   	tx_msg.buf[10]='S';tx_msg.buf[11]='E';tx_msg.buf[12]='N';tx_msg.buf[13]='S';tx_msg.buf[14]='O';tx_msg.buf[15]='R';
	   	   	tx_msg.buf[16]='\r';
	   	   	tx_msg.buf[17]='\n';
	   	   	tx_msg.len = 18;
	   	 send_message(0,tx_msg.len);
	   	while (tx_msg.ptr != 0);
	  	UCB0IE |= UCRXIE;                          // Enable USCI0 RX interrupt
	    P2OUT &= ~BIT4;                            // Clear P2.4 -> LM71 Chip select Activated
	    __delay_cycles(10);		
	    UCB0TXBUF = MST_Data;                     // TX SPI to LM71
	    while(! secondbyte_received);			  // Wainting for a second byte received from the LM71
	  	Valor_temp = SLV_DataTT * 0.03125f;		  // Translating the data from de LM71 to Celcius
	    Temp_Celsius = Valor_temp *1000;		  	
	    num_10k = Temp_Celsius / 10000 ;
	    num_1k =  (Temp_Celsius % 10000) / 1000 ;
	    num_100 = ((Temp_Celsius % 10000) - (num_1k * 1000)) / 100;
	    num_10 =  ((Temp_Celsius % 10000) - (num_1k * 1000 + num_100 *100)) / 10;
	    num_1 =      ((Temp_Celsius % 10000) - (num_1k * 1000 + num_100 *100)) % 10;
	    //***	Send string TO pc TERMINAL -> "TEMP XX,YYY Celcius  HH:MM:SS"	***
	   	tx_msg.buf[0]='T';tx_msg.buf[1]='E';tx_msg.buf[2]='M';tx_msg.buf[3]='P';tx_msg.buf[4]=' ';
	   	tx_msg.buf[5]= 0X30 |num_10k;
	   	tx_msg.buf[6]= 0X30 |num_1k;
	   	tx_msg.buf[7]=',';
	   	tx_msg.buf[8]= 0X30 |num_100;
	   	tx_msg.buf[9]= 0X30 |num_10;
	   	tx_msg.buf[10]= 0X30 |num_1;
	   	tx_msg.buf[11]=' ';
	   	tx_msg.buf[12]= 248;	//see ASCII Table
	   	tx_msg.buf[13]='C';
	   	tx_msg.buf[14]=' ';
	   	tx_msg.buf[15]='T';tx_msg.buf[16]='I';tx_msg.buf[17]='M';tx_msg.buf[18]='E';tx_msg.buf[19]=' ';
	   	tx_msg.buf[20]=0X30 | (RTCHOUR >> 4);
	   	tx_msg.buf[21]=0X30 | (RTCHOUR & 0X0F);
	   	tx_msg.buf[22]=':';
	   	tx_msg.buf[23]=0X30 | (RTCMIN >> 4);
	   	tx_msg.buf[24]=0X30 | (RTCMIN & 0X0F);
	   	tx_msg.buf[25]=':';
	   	tx_msg.buf[26]=0X30 | (RTCSEC >> 4);
	   	tx_msg.buf[27]=0X30 | (RTCSEC & 0X0F);
	   	tx_msg.buf[28]='\r';
	   	tx_msg.buf[29]='\n';
	   	tx_msg.buf[30]='\n';
	   	tx_msg.len= 31;
	   	send_message(0,tx_msg.len);
  }
}

///////////////////////////////////////////////////////
// Interrupt Service Routine for the USCI_A0 UART module
///////////////////////////////////////////////////////

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
	#pragma vector=USCI_A0_VECTOR
	__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
	void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
	#error Compiler not supported!
#endif
{
	    char ch;
	    switch(__even_in_range(UCA0IV,USCI_UART_UCTXCPTIFG))
	    {
	    case USCI_NONE:
	    break;
	    case USCI_UART_UCRXIFG:
	    break;
	    case USCI_UART_UCTXIFG:
	    UCA0TXBUF = tx_msg.buf[tx_msg.ptr++];
	    if (tx_msg.ptr >= tx_msg.len)
	    {
	        /* Stop transmission */
	        UCA0IE &= ~UCTXIE;
	        tx_msg.ptr = 0;
	        tx_msg.len = 0;
	    }
	    break;
	    case USCI_UART_UCSTTIFG:
	    break;
	    case USCI_UART_UCTXCPTIFG:
	    break;
	    }
}

///////////////////////////////////////////////////////
// Interrupt Service Routine for the USCI_B0 SPI module
///////////////////////////////////////////////////////
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCI_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  volatile unsigned int i;

  while (!(UCB0IFG & UCRXIFG));              // USCI_A0 TX buffer ready?

  if(MST_Data == 1){
	  SLV_Data1 = UCB0RXBUF;	//First the Most Signigficant Byte of the Temp. Sensor
  	  MST_Data++;  }
  else{
	  SLV_Data2 = UCB0RXBUF;	// Least Signigficant Byte of the Temp. Sensor
  	  SLV_DataTTaux = (SLV_Data1 <<8)|SLV_Data2; // 16-bit data Temp. Sensor
  	  SLV_DataTT = SLV_DataTTaux >> 2;	// Discard the two least significant bits
  	  MST_Data = 1;						//MSByte received
  	  P2OUT |= BIT4;                    // Slave SPI Chip Select Disabled
  	  secondbyte_received = 1;			// Flag for the data read from Temp. Sensor
  	  UCB0IE &= UCRXIE;                 // Enable USCI0 RX interrupt
  	  }

if(! secondbyte_received){					//
  UCB0TXBUF = MST_Data;                     // Tx SPI for read the least signficant
	}
}

///////////////////////////////////////////////////////
// Interrupt Service Routine for the Real Time Clock module
///////////////////////////////////////////////////////
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=RTC_VECTOR
__interrupt void RTC_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(RTC_VECTOR))) RTC_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(RTCIV, RTCIV_RT1PSIFG))
    {
        case RTCIV_NONE:      break;        // No interrupts
        case RTCIV_RTCOFIFG:  break;        // RTCOFIFG
        case RTCIV_RTCRDYIFG:               // RTCRDYIFG -> 1 second Interrupt Flag
            P1OUT ^= 0x01;                  // Toggles green LED every second
			if(count_ISR_onesec == 2)
			{
				count_ISR_onesec = 1;
				flag_send_string = 1;
			}
			else
				count_ISR_onesec ++;
            break;
        case RTCIV_RTCTEVIFG:               // RTCEVIFG
            __no_operation();               // Interrupts every minute
            break;
        case RTCIV_RTCAIFG:   break;        // RTCAIFG
        case RTCIV_RT0PSIFG:  break;        // RT0PSIFG
        case RTCIV_RT1PSIFG:  break;        // RT1PSIFG
        default: break;
    }
}

void send_message(int port, int len)
{
    tx_msg.ptr = 0;
    tx_msg.len = len;
    switch (port)
    {
          case 0:
                UCA0IE |= UCTXIE;
          break;
          case 1:
               UCA1IE |= UCTXIE;
          break;
    }
}

void delay_us(unsigned int delay)
{
    while ( delay--)
    {
        __delay_cycles(1);
    }
}
