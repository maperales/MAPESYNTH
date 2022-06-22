/************************************************************
 * Primer ejemplo de MAPE-Synth como sintetizador
 * MODO 1: forma de onda
 *      B1: p.inst-
 *      B2: p.inst+
 *      B3: quita inst
 *      B4: añade inst
 *      SL1: Amplitud inst (0..100)
 *      SL2: Fase inst (0..360)
 *
 ************************************************************/

#include <formas.h>
#include <msp430.h>
#include <stdio.h>
#include "mapesynth.h"
#include "nokia5110.h"

unsigned int medida, num;
unsigned char MSD=0, LSD=0;
unsigned char i=0, Espera=0;
char cadena[20];
volatile unsigned char nota=12, inst=0;
unsigned int fase=0;
unsigned int frec=0;

int t_nota, t_nota_off, tA, tD, tR, SUST, Amp_int;
float Amp, Amp_i, Amp_d, Amp_r;


char Mezcla[NUMINST]={0};
unsigned char Ampl[NUMINST]={0};
unsigned char Fase[NUMINST]={0};
int forma[128];


char nota_act=0;
enum modos {FORMA, ADSR, EFECTO, OTROS};

char modo=FORMA;
int param_adsr[4]={10,10,90,10};
int param_adsr_default[4]={10,10,90,10};
signed char valor_param, par_act=0;
char nombre_param[]="ADSR";



void normaliza(int *vector) //Normaliza las formas de onda a un máximo de (510, -510) para evitar saturaciones
{
    int i, max=0;
    float factor=0.0;
    for(i=0;i<128;i++){
        if(abs(vector[i])>max) max=abs(vector[i]);
    }
    factor= 510.0/(float)max;
    for(i=0;i<128;i++) vector[i]=(int)(factor*(float)vector[i]);
}



void calcula_onda(void) //Si ha cambiado la mezcla, recalcular la forma
{
    char i,j;
    for(i=0;i<128;i++)
    {
        forma[i]=0;
        for(j=0;j<NUMINST;j++)
        {
            if(Mezcla[j]) forma[i]+=((Ampl[j]>>1)*SAMPLES[j][(128+i+Fase[j])&0x7F])>>6;
        }
    }
    normaliza(forma);
    dibuja_onda(forma);
}

void LFO(void)
{

}

int calcula_dac(void)   //Cálculo de la salida del DAC
{
    int dac_calc= 0;

    if(nota)
    {
        dac_calc =  2 * forma[(fase>>8)&0x7f];
        fase += frec;
    }
    dac_calc+=2046;     //Se le suman 2046 para centrarlo
    return dac_calc;
}




void VCO(void)
{
    int dac;
    dac=calcula_dac();
    saca_dac(dac);
}


void calcula_modo(void)
{
    char nuevo_modo;
    if(Enc_val>5) Enc_val=-2;
    if(Enc_val<-2) Enc_val=5;

    if(Enc_val>-1 && Enc_val<3) nuevo_modo=0;
    if(Enc_val>-3 && Enc_val<0) nuevo_modo=2;
    if(Enc_val<6 && Enc_val>2) nuevo_modo=1;
    if(nuevo_modo !=modo)
    {
        clearLCD();
        modo=nuevo_modo;
        if(modo==FORMA) dibuja_onda(forma);
    }

}

int main(void)
{
    char i,j, Comm;
    HAL_conf_MC();
    initLCD();
    clearLCD();
    writeBlockToLCD(Logotipo, 504);
    HAL_habilita_ints();
    espera_boton();
    clearLCD();
    HAL_conf_VCO();
    HAL_conf_LFO();     //No necesario en este primer caso
   // habilita_ints();


    while(1)
    {
        if(Msg)
        {
            Msg=0;
            Comm=Buff[0]&0xF0;
            if(Comm==0x80 || (Comm==0x90 && Buff[2]==0))
            {
                if(Buff[1]==nota_act)
                {
                    nota=0;
                    t_nota_off=0;
                }
            }
            else if(Comm==0x90)
            {
                nota_act=Buff[1];
                frec=Freq[nota_act-OFFSET_NOTA];
                fase=0;
                nota=1;
                t_nota=0;
                Amp=0;
            }
            else if(Comm==PITCH)
            {
                if(Buff[2]>64) //Pitch up
                {
                    frec=Freq[nota_act-OFFSET_NOTA]+(((Freq[nota_act+2-OFFSET_NOTA]-Freq[nota_act-OFFSET_NOTA])*(Buff[2]-64))>>6);
                }
                else    //Pitch Down
                {
                    frec=Freq[nota_act-OFFSET_NOTA]+(((Freq[nota_act-OFFSET_NOTA]-Freq[nota_act-2-OFFSET_NOTA])*(Buff[2]-64))>>6);
                }
            }
        }
        calcula_modo();

        if(modo==FORMA)     //Único modo disponible en este ejemplo
        {
            if(GATE[1] && TRIG[1])
            {
                TRIG[1]=0;
                inst++;
                if(inst>=NUMINST)inst=0;
            }
            if(GATE[0] && TRIG[0])
            {
                TRIG[0]=0;
                if(inst!=0){
                    inst--;
                }
                else
                {
                    inst=NUMINST-1;
                }

            }
            if(GATE[3] && TRIG[3])
            {
                TRIG[3]=0;
                Mezcla[inst]=1;
                Ampl[inst]=SL1;
                Fase[inst]=SL2;
                calcula_onda();
            }
            if(GATE[2] && TRIG[2])
            {
                TRIG[2]=0;
                Mezcla[inst]=0;
                calcula_onda();
            }

            sprintf(cadena,"I:%d A:%d F:%d   ",inst,(SL1*100)>>7,(SL2*360)>>7);
            escribe(0,4,cadena);
            sprintf(cadena, "Mix:         ");
            j=0;
            for(i=0;i<NUMINST;i++)
            {
                if(Mezcla[i]){
                    cadena[4+j]='0'+i;
                    j++;
                }
            }
            escribe(0,5,cadena);
        }
        if(modo==ADSR)  //Desarrollado en MIDI_SYNTH_2
        {
            escribe(0, 1, "Modo ADSR");

            escribe(0, 3, "T.B.D.");
        }
        if(modo==EFECTO)    //FUTUROS DESARROLLOS
        {
            escribe(0, 1, "Modo Efecto");

            escribe(0, 3, "T.B.D.");
        }





    }
}

//unsigned int ciclos=0;

