/***********************************************************************
 * MIDI_CTRL1: Controladora MIDI básica. Genera una secuencia constante
 * Se puede elegir la secuencia entre 4 posibles. Se pueden cambiar parámetros
 *      SL1: cambia la velocidad de la ejecución
 *      SL2: cambia la fuerza de la nota (Velocity)
 *      ROT1: cambia el canal midi por el que salen los mensajes
 *      S1..S4: eligen la canción
 *
 **************************************************************************/


#include <mapesynth.h>
#include <msp430.h>
#include <stdio.h>
#include "nokia5110.h"


const char* nombres[]={"Indiana  ", "Vader    ", "Shire    ", "Simpsons", "Tetris  "};

const char indiana[]={mi1,3,fa1,1,sol1,4,
        do2,8,
        re1,3,mi1,1,fa1,12,
        sol1,3,la1,1,si1,4,
        fa2,8,
        la1,3,si1,1,do2,4,
        re2,4,mi2,4,
        mi1,3,fa1,1,sol1,4,
        do2,8,
        re2,3,mi2,1,
        fa2,12,
        sol1,3,sol1,1,mi2,4, re2,3,
        sol1,1, mi2,4,  re2,3,
        sol1,1, fa2,4,  mi2,3,
        re2,1,do2,12,'$','$'
};


const char darthvader[]={
        sol1,3,SIL,1,sol1,3,SIL,1,sol1,3,SIL,1,mib1,3,sib1,1,
        sol1,4,mib1,3,sib1,1,sol1,8,
        re2,3,SIL,1,re2,3,SIL,1,re2,3,SIL,1,mib2,3,sib1,1,
        solb1,4,mib1,3,sib1,1,sol1,8,
        sol2,4,sol1,2,SIL,1, sol1,1,sol2,4,solb2,3,fa2,1,
        mi2,1,mib2,1,mi2,1,SIL,2,lab1,2,reb2,4,do2,3,si1,1,
        sib1,1,la1,1,sib1,2,SIL,2,mib1,2,solb1,4,mib1,3,solb1,1,
        sib1,4,sol1,3,sib1,1,re2,8,
        sol2,4,sol1,2,SIL,1, sol1,1,sol2,4,solb2,3,fa2,1,
        mi2,1,mib2,1,mi2,1,SIL,2,lab1,2,reb2,4,do2,3,si1,1,
        sib1,1,la1,1,sib1,2,SIL,2,mib1,2,solb1,4,mib1,3,sib1,1,
        sol1,4,mib1,3,sib1,1,sol1,7,SIL,1,

'$','$'};

const char hobbit[]={do1,2,re1,2,mi1,4,sol1,4,mi1,2,re1,2,
        do1,8,mi1,2,sol1,2,
        la1,6,do2,2,si1,2,sol1,2,mi1,6,
        fa1,1,mi1,1,re1,4,do1,2,re1,2,mi1,4,sol1,4,
        re1,2,mi1,2,do1,8,
        mi1,2,sol1,2,la1,4,sol1,4,mi1,4,
        re1,8, '$','$'};

const char Simps[]={do1,8,solb1,8,sol1,16,
        do2,6,mi2,4,solb2,4,la2,2,
        sol2,6,mi2,4,do2,4,la1,2,
        solb1,1,SIL,1,solb1,1,SIL,1,solb1,1,SIL,1,sol1,8,SIL,2,
        SIL,2,solb1,1,SIL,1,solb1,1,SIL,1,solb1,1,SIL,1,sol1,2,sib1,6,
        si1,8,SIL,8,
        do2,6,mi2,4,solb2,4,la2,2,
        sol2,6,mi2,4,do2,4,la1,2,
        SIL,8,solb2,1,SIL,1,solb2,1,SIL,1,solb2,1,SIL,1,sol2,2,
        SIL,2,solb1,1,SIL,1,solb1,1,SIL,1,solb1,1,SIL,1,sol1,2,sib1,6,
        do2,1,SIL,1,do2,1,SIL,1,do2,1,SIL,1,do2,6,SIL,4,
'$','$'};

const char Tetris[]={la1,2,mi1,1,fa1,1,sol1,2,fa1,1,mi1,1,
        re1,1,SIL,1,re1,1,fa1,1,la1,2,sol1,1,fa1,1,
        mi1,1,SIL,1,mi1,1,fa1,1,sol1,2,la1,2,
        fa1,2,re1,1,SIL,1,re1,4,
        SIL,1,sol1,2,sib1,1,
        re2,2,do2,1,sib1,1,la1,3,fa1,1,
        la1,2,sol1,1,fa1,1,mi1,1,SIL,1,mi1,1,fa1,1,
        sol1,2,la1,2,fa1,2,re1,1,SIL,1,re1,4,
'$','$'};


unsigned int medida, num;
char cadena[20];
volatile unsigned char nota=12, inst=0;
char Msg_Midi[3];

int tempo=10, Dur=1;
const char *coplita;
char nota_act=SIL;
int Idx=0;
char Velocity = 70;


char Canal=0;

int main(void)

{
    //Configurar pines E/S botones y Encoder
    HAL_conf_MC();
    initLCD();
    clearLCD();
    coplita=hobbit;
    Idx=0;
    nota_act=SIL;
    HAL_habilita_ints();
    writeBlockToLCD(Logotipo, 504);
    espera_boton();
    clearLCD();

    // Valores iniciales
    tempo=2+(SL1>>3);
    Velocity=SL2;
    Canal=ROT1>>3;
    t=0;

    escribe(0,0,"MIDI_CTRL1");
    while(1)
    {
        /**********************
         * Lectura de variables analógicas
         * y modificación de parámetros
         * Se muestra por la pantalla el valor
         ***************************/
        if(Cambio_SL1)
        {
            Cambio_SL1=0;
            tempo=2+(SL1>>3);
            sprintf(cadena," SL1:%d  ", SL1);
            escribe(0, 1, cadena);
        }
        if(Cambio_SL2)
        {
            Cambio_SL2=0;
            Velocity=SL2;
            sprintf(cadena," SL2:%d  ", SL2);
            escribe(0, 2, cadena);
        }
        if(Cambio_POT1)
        {
            Cambio_POT1=0;
            Canal=ROT1>>3;
            sprintf(cadena,"ROT1:%d    ", ROT1);
            escribe(0, 3, cadena);
        }
        /**********************************
         * Lectura de botones y selección de canción
         * Se muestra la canción elegida
         **********************************/


        if(GATE[0] && TRIG[0]){
            TRIG[0]=0;
            coplita=indiana;
            Idx=0;
            Dur=0;
            escribe(0, 4, nombres[0]);
        }
        if(GATE[1] && TRIG[1]){
            TRIG[1]=0;
            coplita=darthvader;
            Idx=0;
            Dur=0;
            escribe(0, 4, nombres[1]);
        }
        if(GATE[2] && TRIG[2]){
            TRIG[2]=0;
            coplita=hobbit;
            Idx=0;
            Dur=0;
            escribe(0, 4, nombres[2]);
        }
        if(GATE[3] && TRIG[3]){
            TRIG[3]=0;
            coplita=Tetris;
            Idx=0;
            Dur=0;
            escribe(0, 4, nombres[4]);
        }
        /*******************************
         * Actualización de la nota:
         * Si ha pasado el tiempo de la nota
         * se manda un Note Off de la anterior
         * Se manda un Note On de la nueva
         * Se informa en la pantalla
         *********************************/

        if(t>=tempo*Dur)
        {
            t=0;
            if(nota_act!=SIL){
                Msg_Midi[0]=NOTE_OFF+Canal;
                Msg_Midi[1]=nota_act;
                Msg_Midi[2]=Velocity;
                manda_midi(Msg_Midi);
                sprintf(cadena,"OUT:%02X-%02X-%02X  " ,Msg_Midi[0],Msg_Midi[1],Msg_Midi[2]);
                escribe(0, 5, cadena);
           }
            if(coplita[Idx]=='$')
            {
                Idx=0;
                Dur=0;
            }
            else
            {
                nota_act=coplita[Idx];
                Idx++;
                Dur=coplita[Idx];
                Idx++;
                if(nota_act!=SIL)
                {
                    Msg_Midi[0]=NOTE_ON+Canal;
                    Msg_Midi[1]=nota_act;
                    Msg_Midi[2]=Velocity;
                    manda_midi(Msg_Midi);
                    sprintf(cadena,"OUT:%02X-%02X-%02X  " ,Msg_Midi[0],Msg_Midi[1],Msg_Midi[2]);
                    escribe(0, 5, cadena);
                }
            }
        }
    }
}

/************************************
 * Definición de funciones de LFO, calcula_dac,
 * vacías en este caso, por compatibilidad
 * con el resto de casos
 ***********************************/
void LFO(void)
{

}
void VCO(void)
{

}

int calcula_dac(void)
{
return 0;
}
