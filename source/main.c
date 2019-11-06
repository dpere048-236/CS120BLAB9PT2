/*	Author: dpere048
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include <avr/interrupt.h>
unsigned char buttonPress = 0x00;
unsigned char note = 0x00;
const double notes[8] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};
//0.954 hz is lowest frequency possible with this function,
//based on settings in PWM_on()
//Passing in 0 as the frequency will stop the speaker from generating sound
void set_PWM(double frequency) {
	static double current_frequency; // Keeps track of the currently set frequency
	//Will only update the registers when the frequency changes, otherwise allows
	//music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; } //stops timer/counter
		else { TCCR3B |= 0x03;} //resumes continues timer counter
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		//prevents OCR0A from underflowing, using prescaler 64
		else if (frequency > 31250) { OCR3A = 0x0000; }
		//set OCR3A based on desired frequency
		else { OCR3A = (short)(8000000 / (128*frequency)) -1; }
		TCNT3 = 0; //resets counter
		current_frequency = frequency; //update current frequency
	}
}
void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB3 on compare match between counter and OCR0A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	//WGM02: When counter (TCNT0) matches OCR0A, reset counter
	//CS01 & CS30: Set a prescaler of 64
	set_PWM(0);
}
void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

enum States{on, off, up, down, compute, waitOff, waitUp, waitDown}state;
void button_tick(){
	buttonPress = ~PINA &0x07; //PA0,1,2
	switch(state){
		case off:
		if(buttonPress == 1){
			state = on;
		}
		else{
			state = off;
		}
		break;
		case on:
		if(buttonPress == 1){
			state = on;
		}
		else{
			state = compute;
		}
		break;
		case compute:
		if(buttonPress == 2){
			if(note < 7)
				note++;
			state = up;
		}
		else if(buttonPress == 4){
			if(note > 0)
				note--;
			state = down;
		}
		else if(buttonPress == 1){
			state = waitOff;
		}
		else{
			state = compute;
		}
		break;
		case up:
			state = waitUp;
		break;
		case down:
			state = waitDown;
		break;
		case waitUp:
			if(buttonPress == 2){
				state = waitUp;
			}
			else{
				state = compute;
			}
			break;
		case waitDown:
			if(buttonPress == 4){
				state = waitDown;
			}
			else{
				state = compute;
			}
			break;
		case waitOff:
			if(!(buttonPress == 1)){
				state = on;
		}	else{
				state = waitOff;
		}	break;
	}
	switch(state){ //actions
		case off:
			PWM_off();
			break;
		case on:
			PWM_on();
			break;
		case compute:
			break;
		case up:
			set_PWM(notes[note]);
			break;
		case down:
			set_PWM(notes[note]);
			break;
		case waitUp:
			break;
		case waitDown:
			break;
		case waitOff:
			break;
	}
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF; //a input
	DDRB = 0xFF; PORTB = 0x00; //output is from port B
	PWM_on();
	state = off;
    /* Insert your solution below */
    while (1) {
	button_tick();
    }
    return 0;
}
