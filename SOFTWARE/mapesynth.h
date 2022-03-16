/*
 * midisynth.h
 *
 *  Created on: 12 mar. 2018
 *      Author: ManoloP
 */

#ifndef MAPESYNTH_H_
#define MAPESYNTH_H_

/*******************************************************************
 * parte del fichero de cabecera relativo al manejo del sintetizador
 * Basado en midilib.h, con inclusión de las nuevas funciones necesarias
 * para el manejo del encoder y el joystick
 *
 */

#define do1     60
#define reb1    61
#define re1     62
#define mib1    63
#define mi1     64
#define fa1     65
#define solb1   66
#define sol1    67
#define lab1    68
#define la1     69
#define sib1    70
#define si1     71
#define do2     72
#define reb2    73
#define re2     74
#define mib2    75
#define mi2     76
#define fa2     77
#define solb2   78
#define sol2    79
#define lab2    80
#define la2     81
#define sib2    82
#define si2     83

#define SIL 250


#define NOTE_OFF 0x80
#define NOTE_ON  0x90
#define CC 0xB0
#define PITCH 0xE0
#define MOD_W 0x01

void manda_midi(char *p);

void manda_char( char a);
void Note_on(char i);
void Note_off(char i);

extern int increm;
extern int relat;
extern signed char cambia_encoder;

int abs(int k);
void normaliza(int *vector);



#define B1 !(P7IN&BIT1)
#define B2 !(P7IN&BIT0)
#define B3 !(P8IN&BIT0)
#define B4 !(P4IN&BIT7)

#define _B1 (P7IN&BIT0)
#define _B2 (P7IN&BIT1)
#define _B3 (P8IN&BIT0)
#define _B4 (P4IN&BIT7)

#define BL_ON  P4OUT |= BIT4
#define BL_OFF P4OUT &= ~BIT4



void manda_midi(char *p);

void manda_char( char a);
void Note_on(char i);
void Note_off(char i);


void espera(int T);
void HAL_conf_MC(void);


extern char Cambio_SL1,Cambio_SL2,Cambio_POT1,Cambio_POT2,Cambio_x,Cambio_y;
extern unsigned int JOY_X, JOY_Y, SL1, SL2, ROT1, ROT2;
extern char GATE[], TRIG[];
extern int t;
extern char Buff[6];
extern char Msg;
extern volatile int Enc_val;


void espera_boton(void);
void HAL_habilita_ints(void);
void HAL_conf_VCO(void);
void HAL_conf_LFO(void);
int calcula_dac(void);
void LFO(void);


#endif /* MAPESYNTH_H_ */
