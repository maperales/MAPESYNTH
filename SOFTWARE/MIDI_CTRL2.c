/*********************************************************************
 * MIDI_CTRL2: controladora para un teclado. Retransmite las notas que
 * recibe, pero modificadas por los siguientes controles:
 *      SL1: incrementa el valor de la nota (hasta 1 octava)
 *      SL2: fija el valor de Velocity
 *      Joystick-EjeY: control del Pitch
 *      Botón S1: Pedal de Sustain
 *********************************************************************/


#include <msp430.h>
#include <msp430.h>
#include <stdio.h>
#include "mapesynth.h"
#include "nokia5110.h"

char cadena[20];
volatile unsigned char nota=12, inst=0;
char Msg_Midi[3];


int main(int argc, char *argv[])
{
    char nota_act;
    HAL_conf_MC();
    initLCD();
    clearLCD();
    writeBlockToLCD(Logotipo, 504);
    HAL_habilita_ints();
    espera_boton();
    clearLCD();
    escribe(0,0,"MIDI_CTRL2");


    while(1)
    {
        /***************************************
         * Leer si hay mensajes MIDI pendientes
         * En caso de mensaje pendiente, actuar
         * y mostrar por pantalla
         ***************************************/
        if(Msg)
        {
            sprintf(cadena," IN:%02X-%02X-%02X " ,Buff[0],Buff[1],Buff[2]);
            escribe(0,1,cadena);
            Msg=0;
            if((Buff[0]==0x90) && (Buff[2]!=0))	//NOTE ON
            {
                Msg_Midi[0]=Buff[0];
                nota_act=Buff[1]+(SL1*13)/127;
                Msg_Midi[1]=nota_act;
                Msg_Midi[2]=SL2;
                manda_midi(Msg_Midi);
                sprintf(cadena,"OUT:%02X-%02X-%02X " ,Msg_Midi[0],Msg_Midi[1],Msg_Midi[2]);
            }
            else if((Buff[0]==0x80) || (Buff[0]==0x90 && Buff[2]==0))  //NOTE OFF
            {
                Msg_Midi[1]=Buff[1]+(SL1*13)/127;
                Msg_Midi[0]=0x80;
                manda_midi(Msg_Midi);
                sprintf(cadena,"OUT:%02X-%02X-%02X " ,Msg_Midi[0],Msg_Midi[1],Msg_Midi[2]);
            }
            else            //Ni note on ni note off...
            {
                manda_midi(Buff);
                sprintf(cadena,"OUT:%02X-%02X-%02X " ,Buff[0],Buff[1],Buff[2]);
            }
            escribe(0,2,cadena);
        }
        //Pitch en el Joystick:
        if(Cambio_y)
        {
            Cambio_y=0;
            Msg_Midi[0]=PITCH;
            Msg_Midi[1]=0;
            Msg_Midi[2]=JOY_Y;
            manda_midi(Msg_Midi);
            sprintf(cadena,"OUT:%02X-%02X-%02X " ,Msg_Midi[0],Msg_Midi[1],Msg_Midi[2]);
            escribe(0,3,cadena);
        }
        /*********************************
         * Pedal de Sustain en el boton 1.
         * Se manda un CC si se pulsa, y otro si se suelta
         *
         ************************************/
        if(TRIG[0])
        {
            Msg_Midi[0]=CC;
            Msg_Midi[1]=64;
            if(GATE[0])
            {
                Msg_Midi[2]=100;
                escribe(0,4,"PEDAL ON ");
            }
            else
            {
                Msg_Midi[2]=0;
                escribe(0,4,"PEDAL OFF");
            }
            TRIG[0]=0;
            manda_midi(Msg_Midi);
        }
    }
}


/************************************
 * Definición de funciones de LFO, VCO calcula_dac,
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

