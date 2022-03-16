
#include <mapesynth.h>
#include <msp430.h>
#include "nokia5110.h"


int n, N_MAX, N_M[8]={3,3,3,3,2,2,3,3};
char Buff[6];
char Msg=0;

int Slider1, Slider2, Pot1, Pot2;
char Cambio_SL1,Cambio_SL2,Cambio_POT1,Cambio_POT2,Cambio_x,Cambio_y;
unsigned int JOY_X=0, JOY_Y=0, SL1=0, SL2=0, ROT1=0, ROT2=0;
int t=0;
char GATE[]={0,0,0,0};
char TRIG[]={0,0,0,0};

volatile int Enc_val=0;

void manda_midi( char *msg)
{
	int i;
	for(i=0;i<3;i++)
	{
	    manda_char(msg[i]);
	}
}


void manda_char( char a)
{
		 while (!(UCA3IFG & UCTXIFG)); // Poll TXIFG to until set
		    UCA3TXBUF = a;       // TX -> RXed character
}

void HAL_clk_16(void)
{
    // Configure one FRAM waitstate as required by the device datasheet for MCLK
      // operation beyond 8MHz _before_ configuring the clock system.
      FRCTL0 = FRCTLPW | NWAITS_1;

      // Clock System Setup
      CSCTL0_H = CSKEY_H;                     // Unlock CS registers
      CSCTL1 = DCOFSEL_0;                     // Set DCO to 1MHz
      // Set SMCLK = MCLK = DCO, ACLK = VLOCLK
      CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
      // Per Device Errata set divider to 4 before changing frequency to
      // prevent out of spec operation from overshoot transient
      CSCTL3 = DIVA__4 | DIVS__4 | DIVM__4;   // Set all corresponding clk sources to divide by 4 for errata
      CSCTL1 = DCOFSEL_4 | DCORSEL;           // Set DCO to 16MHz
      // Delay by ~10us to let DCO settle. 60 cycles = 20 cycles buffer + (10us / (1/4MHz))
      __delay_cycles(60);
      CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers to 1 for 16MHz operation
      CSCTL0_H = 0;                           // Lock CS registers                      // Lock CS registers

}



void HAL_conf_MC(void){

	/*Parar el WatchDog*/

	WDTCTL = WDTPW | WDTHOLD;

    //Configurar pines E/S botones y Encoder

    P7REN |= BIT0+BIT1; //B1, B2
    P7OUT |= BIT0+BIT1;
    P8REN |=BIT0+BIT2;  //B4, ENC_P
    P8OUT |=BIT0+BIT2;
    P4REN |=BIT7;       //B3
    P4OUT |=BIT7;

    /****************************
     * INterrupción del encoder en P4.1
     *
     *****************************/
    P4IFG =0;
    P4IES = BIT1;  // Flanco bajada
    P4IE  = BIT1;   // habilitar int. P4.1



    //Configurar entradas analógicas:

    P3SEL1 |= BIT0+BIT1+BIT2+BIT3;    //JoyX, JoyY, SL1, SL2
    P3SEL0 |= BIT0+BIT1+BIT2+BIT3;
    P1SEL0 |= BIT5+BIT4;    //Rot1, Rot2
    P1SEL0 |= BIT5+BIT4;


    P6SEL1 &= ~(BIT0 | BIT1);
    P6SEL0 |= (BIT0 | BIT1);                // USCI_A3 UART operation

    PJSEL0 |= BIT4 | BIT5;

    P5REN |= BIT5+BIT6;
    P5OUT |= BIT5+BIT6;

    /**********
     * USCIB1 como SPI:
     * P5.0 SIMO: SEL1=0, SEL0=1
     * P5.2 CLK:  SEL1=0, SEL0=1
     * P5.1 CS (I/O)
     ***********/

    P5SEL1 &= ~(BIT0+BIT2);
    P5SEL0 |= BIT0+BIT2;
    P5OUT |= BIT1;
    P5DIR |= BIT1;

    /********************************************
     * Configurar pantalla:
     *******************************/

    P3OUT |= LCD5110_SCE_PIN + LCD5110_DC_PIN;
    P3OUT &= ~LCD5110_SCLK_PIN;
    P3DIR |= LCD5110_SCE_PIN + LCD5110_DC_PIN+LCD5110_SCLK_PIN + LCD5110_DN_PIN;
    BL_ON;  //Backlight on
    P4DIR |= BIT4;

    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    ADC12CTL0 = ADC12SHT0_2 | ADC12ON |ADC12MSC;      // Sampling time, S&H=16, ADC12 on

      ADC12CTL1 = ADC12SHP |ADC12CONSEQ_1 |ADC12SSEL_3 |ADC12DIV_0 ;                   // Use sampling timer
      ADC12CTL2 |= ADC12RES_2;                // 12-bit conversion results
      ADC12CTL3 |= ADC12CSTARTADD_0;
      ADC12MCTL0 |= ADC12INCH_12;
      ADC12MCTL1 |= ADC12INCH_13;
      ADC12MCTL2 |= ADC12INCH_14;
      ADC12MCTL3 |= ADC12INCH_15;
      ADC12MCTL4 |= ADC12INCH_4;
      ADC12MCTL5 |= ADC12INCH_5 | ADC12EOS;

      ADC12IER0 |= ADC12IE5  ;                  // Enable ADC conv complete interrupt
      HAL_clk_16();

    // ACLK, contmode, clear TAR, enable overflow interrupt
    TA0CTL = TASSEL__SMCLK | MC__UP | ID__8| TACLR  ; //2MHz
//    TA0CTL = TASSEL__ACLK | MC__UP | TACLR  ;

    TA0CCTL0 = CCIE;
    TA0CCR0 = 39999;  //2M/40000 /50Hz

   // TA0CCR0 = 3200;  //32kHz /3200: 10Hz

    /***************************
     * CONFIG. UCA3 para MIDI
     * 31250bps
     **********/
     UCA3CTLW0 = UCSWRST;                    // Put eUSCI in reset
     UCA3CTLW0 |= UCSSEL__SMCLK;             // CLK = SMCLK (16MHz)
     // Baud Rate calculation
     // 16000000/(16*31250) = 32
     // Fractional portion = 0
     // User's Guide Table 21-4: UCBRSx = 0x04
     // UCBRFx = int ( (52.083-52)*16) = 1 0.021*13=0.33 (0)

     UCA3BRW = 32;
     UCA3MCTLW |= UCOS16;   // OVERSAMPLING con modulation=0
     UCA3CTLW0 &= ~UCSWRST;                  // Initialize eUSCI
     UCA3IE |= UCRXIE;                       // Enable USCI_A3 RX interrupt


     /*****************
      * CONF. UCB1 como SPI:
      *
      */
        UCB1CTLW0 = UCSWRST;                    // **Put state machine in reset**
        UCB1CTLW0 |= UCMST | UCSYNC | UCCKPL | UCMSB; // 3-pin, 8-bit SPI master
                                                // Clock polarity high, MSB
        UCB1CTLW0 |= UCSSEL__SMCLK;              // ACLK
        UCB1BRW = 0x01;                         // /2
        //UCB1MCTLW = 0;                          // No modulation
        UCB1CTLW0 &= ~UCSWRST;                  // **Initialize USCI state machine**
}


#pragma vector=EUSCI_A3_VECTOR
__interrupt void INterrupcion_RX_A3(void)
{

    char Buf_RX;
    Buf_RX=UCA3RXBUF;
    if(Buf_RX>=0x80)
    {
        n=0;
        N_MAX=N_M[((Buf_RX>>4)&0x07)];
    }
    Buff[n]=Buf_RX;
    n++;
    if(n==N_MAX)Msg=1;
}

void lee_botones(void)
{
    char i, bot[4];
    bot[0]=B1;
    bot[1]=B2;
    bot[2]=B3;
    bot[3]=B4;

    for(i=0;i<4;i++)
    {
        if(bot[i] !=GATE[i])
        {
                TRIG[i]=1;
        }
        GATE[i]=bot[i];
    }
}

char pant[6][64];
void dibuja_onda(int *forma)
{
    char i,j,k;
    int Ypunt;
    char Yf, Yant;
    for(i=0;i<6;i++)
    {
        for (j=0;j<64;j++) pant[i][j]=0;
    }
    for(i=0;i<64;i++)
    {
        Ypunt=512+(forma[2*i]+forma[2*i+1])/2;
        Yf=31-(Ypunt/32);
        if(i>1)
        {
            if(Yf>(Yant+4))
            {
                for(k=Yf;k>Yant;k--)
                {
                    pant[(k&0x18)>>3][i] |= (1<<((k&0x07)));
                }
            }
            if(Yf<(Yant-4))
                        {
                            for(k=Yf;k<Yant;k++){
                                pant[(k&0x18)>>3][i] |= (1<<((k&0x07)));
                            }
                        }

        }
        Yant=Yf;
        pant[(Yf&0x18)>>3][i] |= (1<<((Yf&0x07)));
    }
    for(i=0;i<4;i++)
    {

        setAddr(10,i);

        writeBlockToLCD(pant[i],64);
}
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Int_T0(void)
{
        ADC12CTL0 |= ADC12ENC | ADC12SC;    // Lee pots
        lee_botones();                      //Lee botones
        t++;                                //Incrementa el tiempo de ejecucion
}

#pragma vector = PORT4_VECTOR
__interrupt void Encoder(void)
{
    if(P8IN&BIT1)
    {
        Enc_val++;
    }
    else
    {
        Enc_val--;
    }
    P4IFG =0;

}

#pragma vector = ADC12_B_VECTOR
__interrupt void ADC12_ISR(void)
{
    unsigned int sl1, sl2,rot1, rot2, jx, jy;

    switch(__even_in_range(ADC12IV, ADC12IV__ADC12RDYIFG))
    {
    case ADC12IV__NONE:        break;   // Vector  0:  No interrupt
    case ADC12IV__ADC12OVIFG:  break;   // Vector  2:  ADC12MEMx Overflow
    case ADC12IV__ADC12TOVIFG: break;   // Vector  4:  Conversion time overflow
    case ADC12IV__ADC12HIIFG:  break;   // Vector  6:  ADC12BHI
    case ADC12IV__ADC12LOIFG:  break;   // Vector  8:  ADC12BLO
    case ADC12IV__ADC12INIFG:  break;   // Vector 10:  ADC12BIN
    case ADC12IV__ADC12IFG0:   break;   // Vector 12:  ADC12MEM0 Interrupt
    case ADC12IV__ADC12IFG1:            // Vector 14:  ADC12MEM1
        break;
    case ADC12IV__ADC12IFG2:   break;   // Vector 16:  ADC12MEM2
    case ADC12IV__ADC12IFG3:   break;   // Vector 18:  ADC12MEM3
    case ADC12IV__ADC12IFG4:   break;   // Vector 20:  ADC12MEM4
    case ADC12IV__ADC12IFG5:      // Vector 22:  ADC12MEM5
        ADC12IV=0;

//        JOY_X=ADC12MEM0>>5;
//        JOY_Y=ADC12MEM1>>5;

        jx=ADC12MEM0>>5;
        if(JOY_X!=jx)
        {
            JOY_X=jx;
            Cambio_x=1;
        }
        jy=ADC12MEM1>>5;
        if(JOY_Y!=jy)
                {
                    JOY_Y=jy;
                    Cambio_y=1;
                }
        sl2=ADC12MEM2>>5;
        if(SL2!=sl2)
        {
            SL2=sl2;
            Cambio_SL2=1;
        }
//        else
//        {
//            Cambio_SL2=0;
//        }
        sl1=ADC12MEM3>>5;
        if(SL1!=sl1)
                {
                    SL1=sl1;
                    Cambio_SL1=1;
                }
//                else
//                {
//                    Cambio_SL1=0;
//                }
        rot2=ADC12MEM4>>5;
        if(ROT2!=rot2)
                {
                    ROT2=rot2;
                    Cambio_POT2=1;
                }
//                else
//                {
//                    Cambio_POT2=0;
//                }
        rot1=ADC12MEM5>>5;
        if(ROT1!=rot1)
                        {
                            ROT1=rot1;
                            Cambio_POT1=1;
                        }
//                        else
//                        {
//                            Cambio_POT1=0;
//                        }
        P6OUT &=~BIT0;
        LPM0_EXIT;

        break;
    case ADC12IV__ADC12IFG6:   break;   // Vector 24:  ADC12MEM6
    case ADC12IV__ADC12IFG7:   break;   // Vector 26:  ADC12MEM7
    case ADC12IV__ADC12IFG8:   break;   // Vector 28:  ADC12MEM8
    case ADC12IV__ADC12IFG9:   break;   // Vector 30:  ADC12MEM9
    case ADC12IV__ADC12IFG10:  break;   // Vector 32:  ADC12MEM10
    case ADC12IV__ADC12IFG11:  break;   // Vector 34:  ADC12MEM11
    case ADC12IV__ADC12IFG12:  break;   // Vector 36:  ADC12MEM12
    case ADC12IV__ADC12IFG13:  break;   // Vector 38:  ADC12MEM13
    case ADC12IV__ADC12IFG14:  break;   // Vector 40:  ADC12MEM14
    case ADC12IV__ADC12IFG15:  break;   // Vector 42:  ADC12MEM15
    case ADC12IV__ADC12IFG16:  break;   // Vector 44:  ADC12MEM16
    case ADC12IV__ADC12IFG17:  break;   // Vector 46:  ADC12MEM17
    case ADC12IV__ADC12IFG18:  break;   // Vector 48:  ADC12MEM18
    case ADC12IV__ADC12IFG19:  break;   // Vector 50:  ADC12MEM19
    case ADC12IV__ADC12IFG20:  break;   // Vector 52:  ADC12MEM20
    case ADC12IV__ADC12IFG21:  break;   // Vector 54:  ADC12MEM21
    case ADC12IV__ADC12IFG22:  break;   // Vector 56:  ADC12MEM22
    case ADC12IV__ADC12IFG23:  break;   // Vector 58:  ADC12MEM23
    case ADC12IV__ADC12IFG24:  break;   // Vector 60:  ADC12MEM24
    case ADC12IV__ADC12IFG25:  break;   // Vector 62:  ADC12MEM25
    case ADC12IV__ADC12IFG26:  break;   // Vector 64:  ADC12MEM26
    case ADC12IV__ADC12IFG27:  break;   // Vector 66:  ADC12MEM27
    case ADC12IV__ADC12IFG28:  break;   // Vector 68:  ADC12MEM28
    case ADC12IV__ADC12IFG29:  break;   // Vector 70:  ADC12MEM29
    case ADC12IV__ADC12IFG30:  break;   // Vector 72:  ADC12MEM30
    case ADC12IV__ADC12IFG31:  break;   // Vector 74:  ADC12MEM31
    case ADC12IV__ADC12RDYIFG: break;   // Vector 76:  ADC12RDY
    default: break;
    }
}

int dac;

#pragma vector=TIMER1_A0_VECTOR
__interrupt void VCO(void)
{
    P1OUT|=BIT0;
    dac=calcula_dac();
    P5OUT &= ~BIT1;
                      UCB1TXBUF = 0x70+(dac>>8);                   // Transmit characters
                      UCB1TXBUF = dac&0x00ff;                   // Transmit characters
                      while(!(UCB1IFG&UCTXIFG));
                      P5OUT |=BIT1;
                      P1OUT &=~BIT0;
}


#pragma vector=TIMER3_A0_VECTOR
__interrupt void LFO_Int(void)
{
 LFO();
}


void HAL_conf_VCO(void)
{
    TA1CTL = TASSEL__SMCLK | MC__UP | TACLR  ;
    TA1CCTL0 = CCIE;
    TA1CCR0 = 799;  //16M/800 = 20kHz
}


void HAL_conf_LFO(void)
{
    TA3CTL = TASSEL__SMCLK | ID_3 | MC__UP | TACLR  ;
    TA3CCTL0 = CCIE;
    TA3CCR0 = 19999;  //2M/20000 = 100Hz
}


void HAL_habilita_ints(void)
{
    __bis_SR_register( GIE);
}

void espera(int MS)
{
    while(MS)
    {
        MS--;
        __delay_cycles(16000);
    }
}
void espera_boton(void)
{
    while(!(GATE[0]+GATE[1]+GATE[2]+GATE[3]));
}
