/*
 * _7seg_LED.c
 *
 * Created: 30.05.2020 21:58:37
 *  Author: igork
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "7seg_LED.h"

const uint8_t digit_tab[] PROGMEM = {
	~(a_seg|b_seg|c_seg|d_seg|e_seg|f_seg),			// 0
	~(b_seg|c_seg),									// 1
	~(a_seg|b_seg|d_seg|e_seg|g_seg),				// 2
	~(a_seg|b_seg|c_seg|d_seg|g_seg),				// 3
	~(b_seg|c_seg|f_seg|g_seg),						// 4
	~(a_seg|c_seg|d_seg|f_seg|g_seg),				// 5
	~(a_seg|c_seg|d_seg|e_seg|f_seg|g_seg),			// 6
	~(a_seg|b_seg|c_seg|f_seg),						// 7
	~(a_seg|b_seg|c_seg|d_seg|e_seg|f_seg|g_seg),	// 8
	~(a_seg|b_seg|c_seg|d_seg|f_seg|g_seg),			// 9
	0xFF											// Null
};

void seg_init(void){
	Number_segDDR |= maska_seg;
	Numbers_DDR |= maska_numx;
	
	Number_seg_PORT &= ~(maska_seg);				// Cathodes low
	Numbers_PORT |= (maska_numx);					// Anodes low
}

void change_digit(void){
	which_digit <<= 1;
	if(which_digit > 32) which_digit = 1;
	
	Numbers_PORT |= (maska_numx);
	Number_seg_PORT &= ~(maska_seg);
	
	if(which_digit == 1){
		Number_seg_PORT |= pgm_read_byte(&digit_tab[digit_0]);
		Numbers_PORT &= ~(Num0);
	}
	else if(which_digit == 2){
		Number_seg_PORT |= pgm_read_byte(&digit_tab[digit_1]);
		Numbers_PORT &= ~(Num1);
	}
	else if(which_digit == 4){
		Number_seg_PORT |= pgm_read_byte(&digit_tab[digit_2]);
		Numbers_PORT &= ~(Num2);
	}
	else if(which_digit == 8){
		Number_seg_PORT |= pgm_read_byte(&digit_tab[digit_3]);
		Numbers_PORT &= ~(Num3);
	}
	else if(which_digit == 16){
		Number_seg_PORT |= pgm_read_byte(&digit_tab[digit_4]);
		Numbers_PORT &= ~(Num4);
	}
	else if(which_digit == 32){
		Number_seg_PORT |= pgm_read_byte(&digit_tab[digit_5]);
		Numbers_PORT &= ~(Num5);
	}
	
	
}