/********************************************************************************
Includes
********************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#ifdef	__cplusplus
extern "C" {
#endif
#include "usart.h"
#include "atmega128.h"

#ifdef	__cplusplus
}
#endif

//(OC3B/INT4) PE4

/********************************************************************************
Global Variables
********************************************************************************/
volatile uint8_t ir_data[3072];
volatile uint16_t ir_data_count = 0;
volatile uint8_t ovf_count = 0;
volatile uint8_t send_data_flag = 0;

/********************************************************************************
Interrupt Service
********************************************************************************/

ISR(INT4_vect)
{
	uint16_t t = TCNT1;
	ir_data[ir_data_count++] = ovf_count;
	TCNT1 = 0;
	ovf_count = 0;

	ir_data[ir_data_count++] = (uint8_t) (t >> 8);
	ir_data[ir_data_count++] = (uint8_t) (t);
}

ISR(TIMER1_OVF_vect)
{
	ovf_count++;
}

ISR(USART0_RX_vect)
{
	send_data_flag = 1;
	uint8_t data = UDR0;
}

/********************************************************************************
Function Prototypes
********************************************************************************/

/********************************************************************************
Main
********************************************************************************/
int main(void) {
    // initialize code
	usart_init();

	// initialize input port for ir receiver
	_in(DDE4,   DDRE);
	_on(PE4,    PORTE);

	// initialize input port interrupt
	_on(ISC40,  EICRB);
	_off(ISC41, EICRB);
	_on(INT4,   EIMSK);

	// initialize 16 bit timer 1
	TCCR1B = (0<<CS12)|(1<<CS11)|(1<<CS10);
	_on(TOIE1, TIMSK);

    // enable interrupts
    sei();

    printf("INIT AFTER RESET");

    // main loop
    while (true) {
    	if (send_data_flag && ir_data_count > 0) {

    		printf("START PACKET");

    		for (uint16_t i = 0; i < ir_data_count; i++) {
    			usart_putchar(ir_data[i]);
    		}

    		printf("END PACKET");

    		ir_data_count = 0;
    		send_data_flag = 0;
    	}
    }
}
