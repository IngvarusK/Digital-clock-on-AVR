/*
 * _7seg_LED.h
 *
 * Created: 30.05.2020 21:58:49
 *  Author: igork
 */ 


#ifndef _7SEG_LED_H_
#define _7SEG_LED_H_

//-----------------------7 Segments 6 numbers display settings-----------------------

#define Number_seg_PORT PORTA
#define Numbers_PORT PORTB
#define Number_segDDR DDRA
#define Numbers_DDR DDRB

#define Num0 (1<<PB0)
#define Num1 (1<<PB1)
#define Num2 (1<<PB2)
#define Num3 (1<<PB3)
#define Num4 (1<<PB4)
#define Num5 (1<<PB5)

#define a_seg (1<<PA0)
#define b_seg (1<<PA1)
#define c_seg (1<<PA2)
#define d_seg (1<<PA3)
#define e_seg (1<<PA4)
#define f_seg (1<<PA5)
#define g_seg (1<<PA6)

#define maska_numx (Num0|Num1|Num2|Num3|Num4|Num5)
#define maska_seg (a_seg|b_seg|c_seg|d_seg|e_seg|f_seg|g_seg)

//----------------------------------------------------------------------------------

extern uint8_t which_digit;
extern uint8_t digit_0;
extern uint8_t digit_1;
extern uint8_t digit_2;
extern uint8_t digit_3;
extern uint8_t digit_4;
extern uint8_t digit_5;
extern uint8_t clock_one_fivehund;

void seg_init(void);
void change_digit(void);

#endif /* _7SEG_LED_H_ */