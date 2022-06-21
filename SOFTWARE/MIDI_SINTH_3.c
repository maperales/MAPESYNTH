/************************************************************
 * Segundo ejemplo de MAPE-Synth como sintetizador
 * MODO 1: forma de onda
 *      B1: p.inst-
 *      B2: p.inst+
 *      B3: quita inst
 *      B4: añade inst
 *      SL1: Amplitud inst (0..100)
 *      SL2: Fase inst (0..360)
 * MODO 2: ADSR
 *      B1: param-
 *      B2: param+
 *      B3: reset param
 *      B4: guarda param
 *      JOY_Y: sube o baja el param. 1 o 10
 * MODO 3: Guarda Forma Onda
 *      B1: puntero -
 *      B2  Puntero +
 *      B3: Guardar
 *      B4: Guardar
 *
 ************************************************************/

#include <msp430.h>
#include <stdio.h>
#include "formas.h"
#include "mapesynth.h"
#include "nokia5110.h"

unsigned int medida, num;
//unsigned char MSD=0, LSD=0;
unsigned char i=0, Espera=0;
char cadena[20];
volatile unsigned char nota=0, inst=0;
unsigned int fase=0;
unsigned int frec=0;


char Nom[]="01234567ABCD";

void normaliza(int *vector)
{
    int i, max=0;
    float factor=0.0;
    for(i=0;i<128;i++){
        if(abs(vector[i])>max) max=abs(vector[i]);
    }
    factor= 510.0/(float)max;
    for(i=0;i<128;i++) vector[i]=(int)(factor*(float)vector[i]);
}



char Mezcla[NUMINST+4]={0};
unsigned char Ampl[NUMINST+4]={0};
unsigned char Fase[NUMINST+4]={0};
int forma[128];


int t_nota, t_nota_off, tA, tD, tR, SUST, Amp_int;
float Amp, Amp_i, Amp_d, Amp_r;

char nota_act=0;
enum modos {FORMA, ADSR, SAVE, MIDI_SNIFF};

char modo=FORMA;
int param_adsr[4]={10,10,90,10};
int param_adsr_default[4]={10,10,90,10};
signed char valor_param, par_act=0;
char nombre_param[]="ADSR";

char Vib_on=0, Trem_on=0;
int Vel_vib=0, Amp_trem=0;
int NMAX=0;

void calcula_onda(void)
{
    char i,j;
    for(i=0;i<128;i++)
    {
        forma[i]=0;
        for(j=0;j<NMAX;j++)
        {
            if(Mezcla[j]) forma[i]+=((Ampl[j]>>1)*Instrum[j][(128+i+Fase[j])&0x7F])>>6;
        }
    }
    normaliza(forma);
    dibuja_onda(forma);
}

void LFO(void)  //LFO: Aplicación del ADSR y efectos
{
    if(nota==1)
    {
        t_nota++;
        if(t_nota<=param_adsr[0])   //A
        {
            Amp+=Amp_i;
            if(Amp>1000) Amp=1000;
        }else
        {
            if(t_nota<(param_adsr[0]+param_adsr[1]))  //D
            {
                Amp-=Amp_d;
                if(Amp<param_adsr[2]) Amp=param_adsr[2];
            }else
            {
                t_nota=param_adsr[0]+param_adsr[1];  //S

            }
        }
    }
    if(nota==2)
    {
        t_nota_off++;
        if(t_nota_off<param_adsr[3]) //R
        {
            Amp-=Amp_r;
            if(Amp<0)Amp=0;
        }
        else
        {
            nota=0;
        }
    }
    Amp_int=(int)(Amp/20.0);
}

int calcula_dac(void)
{
    int dac_calc= 2046;

    if(nota)
    {
        dac_calc = (forma[(fase>>8)&0x7f]);
        fase += frec;
        //P1OUT &=~BIT0;
        dac_calc=(dac_calc*(Amp_int))/50;
        //P1OUT |=BIT0;
        dac_calc+=2046;

    }
//    dac_calc+=2046;
    return dac_calc;
}

void VCO(void)
{
    int dac;
    dac=calcula_dac();
    saca_dac(dac);
}




void cambia_param(char param)
{
    switch(param)
    {
    case 0:
        Amp_i=1000.0/(float)param_adsr[0];
        break;
    case 1:
        Amp_d=(float)(10*(100-param_adsr[2]))/param_adsr[1];
        break;
    case 2:
        Amp_d=(float)(10*(100-param_adsr[2]))/param_adsr[1];
        Amp_r=(float)(10*param_adsr[2])/(float)param_adsr[3];
        break;
    case 3:
        Amp_r=(float)(10*param_adsr[2])/(float)param_adsr[3];
        break;
    }
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

int Save=0, Guardando=0;

int main(void)
{
    char i,j;
    HAL_conf_MC();
    initLCD();
    clearLCD();
    writeBlockToLCD(Logotipo, 504);
    HAL_habilita_ints();
    espera_boton();
    NMAX=copia_ondas();
    clearLCD();
    HAL_conf_VCO();         //Interrupción a 20kHz
    HAL_conf_LFO();         //Interrupción a 100Hz

     Amp_i=1000.0/(float)param_adsr_default[0];
     Amp_d=(float)(10*(100-param_adsr_default[2]))/param_adsr_default[1];
     Amp_r=(float)(10*param_adsr_default[2])/(float)param_adsr_default[3];

    while(1)
    {
        if(Msg)
        {
            manda_midi(Buff);
            Msg=0;
            if(Buff[0]==0x80 || (Buff[0]==0x90 && Buff[2]==0))
            {
                if(Buff[1]==nota_act)
                {
                    nota=2;
                    t_nota_off=0;
                }
            }
            else if(Buff[0]==0x90)
            {
                nota_act=Buff[1];
                frec=Freq[nota_act-OFFSET_NOTA];
                fase=0;
                nota=1;
                t_nota=0;
                Amp=0;
            }
        }
        calcula_modo();

        if(modo==FORMA)     //Modo selección de forma de onda
        {
            if(GATE[1] && TRIG[1])
            {
                TRIG[1]=0;
                inst++;
                if(inst>=NMAX)inst=0;
            }
            if(GATE[0] && TRIG[0])
            {
                TRIG[0]=0;
                if(inst!=0){
                    inst--;
                }
                else
                {
                    inst=NMAX-1;
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

            sprintf(cadena,"I:%c A:%d F:%d   ",Nom[inst],(SL1*100)>>7,(SL2*360)>>7);
            escribe(0,4,cadena);
            sprintf(cadena, "Mix:         ");
            j=0;
            for(i=0;i<NMAX;i++)
            {
                if(Mezcla[i]){
                    cadena[4+j]=Nom[i];
                    j++;
                }
            }
            escribe(0,5,cadena);
        }
        if(modo==ADSR)      //Modo envolvente
        {
            if(GATE[0] && TRIG[0])  //decrementa selección de parámetro
            {
                TRIG[0]=0;
                if(par_act>0){
                    par_act--;
                }else par_act=3;
                valor_param=param_adsr[par_act];
            }
            if(GATE[1] && TRIG[1]) //incrementa selección de parámetro
            {
                TRIG[1]=0;
                if(par_act<3){
                    par_act++;
                }else par_act=0;
                valor_param=param_adsr[par_act];
            }
            if(GATE[2] && TRIG[2]) //Da valor por defecto al parámetro seleccionado
            {
                TRIG[2]=0;
                param_adsr[par_act]=param_adsr_default[par_act];
                cambia_param(par_act);
            }
            if(GATE[3] && TRIG[3]) //Asigna nuevo valor al parámetro seleccionado
            {
                TRIG[3]=0;
                param_adsr[par_act]=valor_param;
                cambia_param(par_act);
            }
            if(JOY_Y>80)    //Incrementa parámetro
            {
                valor_param++;
                if(JOY_Y>110) valor_param+=3;
                if(valor_param>100) valor_param=100;
            }
            if(JOY_Y<48)    //Decrementa parámetro
            {
                valor_param--;
                if(JOY_Y<14) valor_param-=3;
                if(valor_param<1) valor_param=1;
            }
            sprintf(cadena,"A:%d0ms     ",param_adsr[0]);
            escribe(0, 0, cadena);
            sprintf(cadena,"D:%d0ms     ",param_adsr[1]);
            escribe(0, 1, cadena);
            sprintf(cadena,"S:%d%%     ",param_adsr[2]);
            escribe(0, 2, cadena);
            sprintf(cadena,"R:%d0ms     ",param_adsr[3]);
            escribe(0, 3, cadena);
            if(par_act==2)            
            {
                sprintf(cadena,"S:%d%%     ",valor_param);
            }
            else
            {
                sprintf(cadena,"%c:%d0ms     ",nombre_param[par_act],valor_param);
            }
            escribe(0, 5, cadena);
        }
        if(modo==SAVE)        //Por desarrollar
        {
            escribe(0, 1, "Modo SAVE");
            if(GATE[1] && TRIG[1])
            {
                TRIG[1]=0;
                Save++;
                if(Save==4) Save=0;
            }
            if(GATE[0] && TRIG[0])
            {
                TRIG[0]=0;

                if(Save==0)
                {
                    Save=3;
                }
                else
                {
                    Save--;
                }
            }
            if(GATE[2] && TRIG[2])
            {
                TRIG[2]=0;
                if(Save_Wave[Save*128]!=0xffff)
                {
                    escribe(0,2,"Sobreescribir?");
                    escribe(0,3,"S4: SI");
                    Guardando=1;
                }
                else
                {
                    for(i=0;i<128;i++) Save_Wave[Save*128+i]=forma[i];
                    sprintf(cadena, "Guardado en S%d", Save);
                    escribe(0,3,cadena);
                    NMAX=copia_ondas();

                }
            }
            if(GATE[3] && TRIG[3])
            {
                TRIG[3]=0;
                if(Guardando){
                for(i=0;i<128;i++) Save_Wave[Save*128+i]=forma[i];
                sprintf(cadena, "Guardado en S%d", Save);
                escribe(0,3,cadena);
                Guardando=0;
                NMAX=copia_ondas();
                }
            }
            sprintf(cadena,"Sel: %d ",Save);
            escribe(0,5,cadena);

        }





    }
}

//unsigned int ciclos=0;

