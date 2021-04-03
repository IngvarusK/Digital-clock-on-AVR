/*
 * projekt_Kaminski_Konwewko.c
 *
 * Created: 30.05.2020 21:42:22
 * Author : Igor Kamiñski
 *			Konrad Konewko
 *	
 *	PORTA	|7	|6	|5	|4	|3	|2	|1	|0
 *	--------------------------------------
 *			|S	|g	|f	|e	|d	|c	|b	|a
 *  ______________________________________
 *	PORTB	|7	|6	|5	|4	|3	|2	|1	|0
 *	--------------------------------------	
 *			|-	|+	|5	|4	|3	|2	|1	|0
 *  ______________________________________
 *	PORTC	|7	|6	|5	|4	|3	|2	|1	|0
 *	--------------------------------------
 *			|Al	|B	|	|	|	|	|	|
 *	Buzer aktywowany 1, Uzbrojenie alarmu stanem 0
 *
 *
 * Za pomoc¹ S zwiêkszamy status:
 *	1) zegar ustaw - godziny (+ zwiêkszamy, - zmniejszamy)
 *	2) zegar ustaw - minuty (+ zwiêkszamy, - zmniejszamy)
 *	4) zegar ustaw - sekundy (+ zwiêkszamy, - zmniejszamy)
 *			ustawienie godziny 1 sekunda wciœniêcia przycisku "S"
 *	8) zegar - pokazanie godziny
 *	16) budzik ustaw - godziny (+ zwiêkszamy, - zmniejszamy)
 *	32) budzik ustaw - minuty (+ zwiêkszamy, - zmniejszamy)
 *			ustawienie godziny 1 sekunda wciœniêcia przycisku "S"
 *	64) budzik - pokazanie ustawieñ
 *	128) stoper - pokazanie (+ w³¹czamy lub stopujemy, - reset)
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "7seg_LED.h"

// DEBUGGING
#include "mkuart.h"

//#define F_CPU 3686400UL
#define F_CPU 11059200UL

#define switch_S_DDR DDRA
#define switch_plus_minus_DDR DDRB
#define switch_S_PORT PORTA
#define switch_plus_minus_PORT PORTB
#define switch_S_key (1<<PA7)
#define switch_S_plus (1<<PB6)
#define switch_S_minus (1<<PB7)

void switch_operate(uint8_t volatile *switch_PIN, uint8_t switch_mask, uint16_t *switch_time_present, uint8_t switch_time_goal,
	uint8_t *switch_flag_Firststep, uint8_t *switch_flag, uint8_t q_continue, uint8_t switch_time_goal_continue,
	uint16_t *switch_time_absolute);

volatile uint8_t interrupt_flag = 0;

uint8_t which_digit = 1;
uint8_t digit_0 = 0;
uint8_t digit_1 = 0;
uint8_t digit_2 = 0;
uint8_t digit_3 = 0;
uint8_t digit_4 = 0;
uint8_t digit_5 = 0;

int main(void)
{
	
	// TIMER0
	TCCR0 |= (1<<WGM01);				// CTC
	TCCR0 |= (1<<CS02)|(1<<CS00);		// prescaler 1024
	OCR0 = 44;							// interrupt 240Hz 11.059.200Hz
	TIMSK |= (1<<OCIE0);				// permission for interrupts
	
	// DEBUGGING
	USART_Init( __UBRR );			// inicjalizacja UART
	
	sei();
	
	uint8_t status = 8;
	uint8_t onlyone = 0;
	
	uint8_t clock_one_fivehund = 0;
	uint8_t stopwatch_one_fivehund = 0;
	
	uint8_t clock_hours = 0;
	uint8_t clock_minutes = 0;
	uint8_t clock_seconds = 0;
	
	uint8_t alarm_hours = 0;
	uint8_t alarm_minutes = 0;
	uint8_t alarm_set = 0;
	uint8_t alarm_on = 0;
	
	uint8_t change_hours = 0;
	uint8_t change_minutes = 0;
	uint8_t change_seconds = 0;
	
	uint8_t stopwatch_hours = 0;
	uint8_t stopwatch_minutes = 0;
	uint8_t stopwatch_seconds = 0;
	uint8_t stopwatch_on = 0;
	
	uint8_t switch_flag_S_Firststep = 0;
	uint8_t switch_flag_S = 0;
	uint8_t switch_mask_S = switch_S_key;
	uint16_t switch_time_present_S = 0;
	uint16_t switch_time_absolute_S = 0;
	
	uint8_t switch_flag_plus_Firststep = 0;
	uint8_t switch_flag_plus = 0;
	uint8_t switch_mask_plus = switch_S_plus;
	uint16_t switch_time_present_plus = 0;
	uint16_t switch_time_absolute_plus = 0;
	
	uint8_t switch_flag_minus_Firststep = 0;
	uint8_t switch_flag_minus = 0;
	uint8_t switch_mask_minus = switch_S_minus;
	uint16_t switch_time_present_minus = 0;
	uint16_t switch_time_absolute_minus = 0;
	
	//uint16_t test = 0; // do testu obciazenia mikroprocesora
	
	//------------------------------Ports Set-------------------------------
	seg_init();
	
	switch_S_DDR &= ~(switch_mask_S);
	switch_plus_minus_DDR &= ~(switch_mask_plus | switch_mask_minus);
	
	switch_S_PORT |= switch_mask_S;
	switch_plus_minus_PORT |= (switch_mask_plus | switch_mask_minus);
	
	DDRC |= (1<<PC6) | (1<<PC7);
	PORTC |= (1<<PC7);// Alarm off
	PORTC &= ~(1<<PC6);// Buzzer off
	    
    while(1){
		//test++;
		//----------------------------Changing positions-----------------------------
		if((switch_flag_S) && (!alarm_on)){
			status <<= 1;
			onlyone = 1;
			if(status == 0) status = 1;
			switch_flag_S = 0;
		}	
		//--------------------------------Digits set--------------------------------
		if(status < 8){
			if(clock_one_fivehund < 150){
				digit_0 = change_hours/10;
				digit_1 = change_hours%10;
				digit_2 = change_minutes/10;
				digit_3 = change_minutes%10;
				digit_4 = change_seconds/10;
				digit_5 = change_seconds%10;
			}
			else{
				if(status == 1){
					digit_0 = 10;
					digit_1 = 10;
					digit_2 = change_minutes/10;
					digit_3 = change_minutes%10;
					digit_4 = change_seconds/10;
					digit_5 = change_seconds%10;
				}
				else if(status == 2){
					digit_0 = change_hours/10;
					digit_1 = change_hours%10;
					digit_2 = 10;
					digit_3 = 10;
					digit_4 = change_seconds/10;
					digit_5 = change_seconds%10;
				}
				else{
					digit_0 = change_hours/10;
					digit_1 = change_hours%10;
					digit_2 = change_minutes/10;
					digit_3 = change_minutes%10;
					digit_4 = 10;
					digit_5 = 10;
				}
			}
		}
		if(status == 8){
			switch_flag_plus = 0;
			switch_flag_minus = 0;
			if(onlyone && (switch_time_absolute_S > 240)){
				clock_hours = change_hours;
				clock_minutes = change_minutes;
				clock_seconds = change_seconds;
				clock_one_fivehund = 0;
				onlyone = 0;
			}
			digit_0 = clock_hours/10;
			digit_1 = clock_hours%10;
			digit_2 = clock_minutes/10;
			digit_3 = clock_minutes%10;
			digit_4 = clock_seconds/10;
			digit_5 = clock_seconds%10;
		}
		if((status > 8) && (status < 64)){
			digit_0 = 10;
			digit_1 = 10;
			if(clock_one_fivehund < 150){
				digit_2 = change_hours/10;
				digit_3 = change_hours%10;
				digit_4 = change_minutes/10;
				digit_5 = change_minutes%10;
			}
			else{
				if(status == 16){
					digit_2 = 10;
					digit_3 = 10;
					digit_4 = change_minutes/10;
					digit_5 = change_minutes%10;
				}
				else{
					digit_2 = change_hours/10;
					digit_3 = change_hours%10;
					digit_4 = 10;
					digit_5 = 10;
				}
			}
			
		}
		if(status == 64){
			if(onlyone && (switch_time_absolute_S > 240)){
				alarm_hours = change_hours;
				alarm_minutes = change_minutes;
				if(alarm_set){
					alarm_set = 0;
					PORTC |= (1<<PC7);// Alarm off
				}
				else{
					alarm_set = 1;
					PORTC &= ~(1<<PC7);// Alarm on
				}
				onlyone = 0;
			}
			digit_0 = 10;
			digit_1 = 10;
			digit_2 = alarm_hours/10;
			digit_3 = alarm_hours%10;
			digit_4 = alarm_minutes/10;
			digit_5 = alarm_minutes%10;
		}
		if(status == 128){
			digit_0 = stopwatch_hours/10;
			digit_1 = stopwatch_hours%10;
			digit_2 = stopwatch_minutes/10;
			digit_3 = stopwatch_minutes%10;
			digit_4 = stopwatch_seconds/10;
			digit_5 = stopwatch_seconds%10;
			if(switch_flag_plus){
				if(stopwatch_on) stopwatch_on = 0;
				else{
					stopwatch_on = 1;
					stopwatch_one_fivehund = 0;
				}
				switch_flag_plus = 0;
			}
			if(switch_flag_minus){
				stopwatch_hours = 0;
				stopwatch_minutes = 0;
				stopwatch_seconds = 0;
				switch_flag_minus = 0;
			}
		}
		//-----------------------------Settings operation------------------------------
		if(status == 1){
			if(onlyone){
				change_hours = clock_hours;
				change_minutes = clock_minutes;
				change_seconds = clock_seconds;
				onlyone = 0;
			}
			if(switch_flag_plus){
				change_hours++;
				if(change_hours > 23) change_hours = 0;
				switch_flag_plus = 0;
			}
			if(switch_flag_minus){
				change_hours--;
				switch_flag_minus = 0;
				if(change_hours > 23) change_hours = 23;
			}
		}
		else if(status == 2){
			if(switch_flag_plus){
				change_minutes++;
				if(change_minutes > 59) change_minutes = 0;
				switch_flag_plus = 0;
			}
			if(switch_flag_minus){
				change_minutes--;
				switch_flag_minus = 0;
				if(change_minutes > 59) change_minutes = 59;
			}
		}
		else if(status == 4){
			if(switch_flag_plus){
				change_seconds++;
				if(change_seconds > 59) change_seconds = 0;
				switch_flag_plus = 0;
			}
			if(switch_flag_minus){
				change_seconds--;
				switch_flag_minus = 0;
				if(change_seconds > 59) change_seconds = 59;
			}
		}
		if(status == 16){
			if(onlyone){
				change_hours = alarm_hours;
				change_minutes = alarm_minutes;
				onlyone = 0;
			}
			if(switch_flag_plus){
				change_hours++;
				if(change_hours > 23) change_hours = 0;
				switch_flag_plus = 0;
			}
			if(switch_flag_minus){
				change_hours--;
				switch_flag_minus = 0;
				if(change_hours > 23) change_hours = 23;
			}
		}
		else if(status == 32){
			if(switch_flag_plus){
				change_minutes++;
				if(change_minutes > 59) change_minutes = 0;
				switch_flag_plus = 0;
			}
			if(switch_flag_minus){
				change_minutes--;
				switch_flag_minus = 0;
				if(change_minutes > 59) change_minutes = 59;
			}
		}
		//--------------------------------Alarm gears--------------------------------
		if(alarm_set){
			if((alarm_hours == clock_hours) && (alarm_minutes == clock_minutes)){
				status = 8;
				alarm_on = 1;
				PORTC |= (1<<PC6);// Buzzer on
			}
		}
		if(alarm_on){
			if((!(PINB & switch_mask_plus)) && (!(PINB & switch_mask_minus))){
				alarm_set = 0;
				PORTC |= (1<<PC7);// Alarm off
				alarm_on = 0;
				PORTC &= ~(1<<PC6);// Buzzer off
				switch_flag_S = 0;
			}
		}
		//--------------------------------Timer gears--------------------------------
		if(interrupt_flag){
			clock_one_fivehund++;
			if(stopwatch_on) stopwatch_one_fivehund++;
			change_digit();
			interrupt_flag = 0;
			// Switch time operate BEGIN
			if(!switch_flag_S) switch_time_present_S++;
			if(switch_flag_S_Firststep) switch_time_absolute_S++;
			if(!switch_flag_plus)switch_time_present_plus++;
			if(!switch_flag_minus)switch_time_present_minus++;
			// Switch time operate END
						
			//------------------------------Clock&Stopwatch gears------------------------------
			if(clock_one_fivehund == 240){
				//uart_putint(test, 10);
				//uart_putc('\r');			// wyœlij znak cr (enter)
				//uart_putc('\n');			// wyœlij znak lf (nowa linia)
				
				//uart_puts("status ");
				//uart_putint(status, 10);
				//uart_puts(" | godzina: ");
				//uart_putint(clock_hours, 10);
				//uart_puts(":");
				//uart_putint(clock_minutes, 10);
				//uart_puts(":");
				//uart_putint(clock_seconds, 10);
				//uart_puts(" | last switch_time_absolute_s: ");
				//uart_putint(switch_time_absolute_S, 10);
				//uart_puts(" | alarm: ");
				//uart_putint(alarm_hours, 10);
				//uart_puts(":");
				//uart_putint(alarm_minutes, 10);
				//uart_puts(" | alarm set&on: ");
				//uart_putint(alarm_set, 10);
				//uart_putint(alarm_on, 10);
				//uart_putc('\r');			// wyœlij znak cr (enter)
				//uart_putc('\n');			// wyœlij znak lf (nowa linia)
				
				clock_seconds++;
				if(clock_seconds == 60){
					clock_minutes++;
					clock_seconds = 0;
					if(clock_minutes == 60){
						clock_hours++;
						clock_minutes = 0;
						if(clock_hours == 24){
							clock_hours = 0;
						}
					}
				}
				clock_one_fivehund = 0;
			}
			if(stopwatch_one_fivehund == 240){
				stopwatch_seconds++;
				if(stopwatch_seconds == 60){
					stopwatch_minutes++;
					stopwatch_seconds = 0;
					if(stopwatch_minutes == 60){
						stopwatch_hours++;
						stopwatch_minutes = 0;
						if(stopwatch_hours == 24){
							stopwatch_hours = 0;
						}
					}
				}
				stopwatch_one_fivehund = 0;
			}
			
			
		//---------------------------------------------------------------------------		
		}
		// Switch operate
		switch_operate(&PINA, switch_mask_S, &switch_time_present_S, 8, &switch_flag_S_Firststep,
			&switch_flag_S, 0, 0, &switch_time_absolute_S);
		switch_operate(&PINB, switch_mask_plus, &switch_time_present_plus, 20, &switch_flag_plus_Firststep,
			&switch_flag_plus, 1, 40, &switch_time_absolute_plus);
		switch_operate(&PINB, switch_mask_minus, &switch_time_present_minus, 20, &switch_flag_minus_Firststep,
			&switch_flag_minus, 1, 40, &switch_time_absolute_minus);
			
    }
}

ISR(TIMER0_COMP_vect){	
	cli();
	interrupt_flag = 1;
	sei();
}

void switch_operate(uint8_t volatile *switch_PIN, uint8_t switch_mask, uint16_t *switch_time_present, uint8_t switch_time_goal,
	uint8_t *switch_flag_Firststep, uint8_t *switch_flag, uint8_t q_continue, uint8_t switch_time_goal_continue,
	uint16_t *switch_time_absolute){
		
	if((*switch_flag_Firststep) && (!(*switch_flag))){
		if(!(*switch_PIN & switch_mask)){
			if((*switch_time_present > switch_time_goal_continue) && (q_continue)){
				*switch_flag = 1;
				*switch_time_present = 0;
			}
		}
		else{
			*switch_time_present = 0;
			*switch_flag_Firststep = 0;
		}
	}
	else{
		if(!(*switch_PIN & switch_mask)){
			if(*switch_time_present > switch_time_goal){
				*switch_flag_Firststep = 1;
				*switch_flag = 1;
				*switch_time_absolute = *switch_time_present;
				*switch_time_present = 0;
			}
		}
		else *switch_time_present = 0;
	}
}