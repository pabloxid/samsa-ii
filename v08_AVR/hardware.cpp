/*
  Part of the SAMSA II project
	http://www.pablogindel.com/trabajos/samsa-ii-2010/
  
	Copyright (c) 2010 Pablo Gindel & Jorge Visca, Montevideo - Uruguay. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

// hardware.cpp
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

#include "util.h"
#include "hardware.h"
#include "events.h"
#include "movimiento.h"
#include "display.h"
#include "cabeza.h"
#include "wiring.h"

//==================================================================================================================//
//                           inicialización de timers y recursos del microcontrolador                               //
//==================================================================================================================//

// seteo del timer y las interrupciones
// nota: esto está todo hardcodeado, hay que acualizarlo a mano si se cambia el valor de TICK
// nota2: emprolijar esto y sacar todos los if defined, dado que este código no podría correr en otros micros

void setTimers () {               // setea el timer2 del Atmega1280 para generar una interrupción periódica
	
	sei();    // habilita las interrupciones (yo lo pondría al final)
	
	// on the ATmega168, timer 0 is also used for fast hardware pwm
	// (using phase-correct PWM would mean that timer 0 overflowed half as often
	// resulting in different millis() behavior on the ATmega8 and ATmega168)
	sbi(TCCR0A, WGM01);
	sbi(TCCR0A, WGM00);
 
	// set timer 0 prescale factor to 64
	// this combination is for the standard 168/328/1280/2560
	sbi(TCCR0B, CS01);
	sbi(TCCR0B, CS00);

	// enable timer 0 overflow interrupt
  enable_ovf0();

	// timers 1 and 2 are used for phase-correct hardware pwm
	// this is better for motors as it ensures an even waveform
	// note, however, that fast pwm mode can achieve a frequency of up
	// 8 MHz (with a 16 MHz clock) at 50% duty cycle
	TCCR1B = 0;

	// set timer 1 prescale factor to 64
	sbi(TCCR1B, CS11);
	sbi(TCCR1B, CS10);
	
	// put timer 1 in 8-bit phase correct pwm mode
	sbi(TCCR1A, WGM10);

	// timer 2
	
	/* First disable the timer overflow interrupt while we're configuring */  
  disable_ovf2();   
  
  /* Configure timer2 in normal mode (pure counting, no PWM etc.) */  
  TCCR2A &= ~((1<<WGM21) | (1<<WGM20));   
  TCCR2B &= ~(1<<WGM22);   
  
  /* Select clock source: internal I/O clock */  
  ASSR &= ~(1<<AS2);   
  
  /* Disable Compare Match A interrupt enable (only want overflow) */  
  TIMSK2 &= ~(1<<OCIE2A);   
  
  /* Now configure the prescaler to CPU clock divided by 1024, lo cual nos da un ciclo de 16ms */  
  TCCR2B |= _BV(CS22) | _BV(CS21) | _BV(CS20);      
  
  /* Finally load end enable the timer */  
  enable_ovf2();   
	
	// timer 3

	sbi(TCCR3B, CS31);		// set timer 3 prescale factor to 64
	sbi(TCCR3B, CS30);
	sbi(TCCR3A, WGM30);		// put timer 3 in 8-bit phase correct pwm mode

	// timer 4

	sbi(TCCR4B, CS41);		// set timer 4 prescale factor to 64
	sbi(TCCR4B, CS40);
	sbi(TCCR4A, WGM40);		// put timer 4 in 8-bit phase correct pwm mode

  // timer 5

  // lo que sigue es para setear el timer5 para generar PWM para los 2 servos de la cabeza
  // fast PWM, 16 bits, prescaler CPU/8, TOP = 40000 (el resto de los seteos quedan como los hizo el core de Arduino)
  TCCR5A |= _BV(WGM51); TCCR5A &= ~_BV(WGM50);
  TCCR5B |= _BV(WGM52) | _BV(WGM53) | _BV(CS51);
  TCCR5B &= ~(_BV(CS50) | _BV(CS52));	
  ICR5 = 40000;
  
	// the bootloader connects pins 0 and 1 to the USART; disconnect them
	// here so they can be used as normal digital i/o; they will be
	// reconnected in Serial.begin()
	UCSR0B = 0;
	
}  


// rutinas del ADC manual

void ADCconfig () {
  // por ahora usamos la inicialización de Arduino, pero hay que revisarla y reemplazarla por lo de abajo
	
	// set a2d prescale factor to 128
	// 16 MHz / 128 = 125 KHz, inside the desired 50-200 KHz range.
	// XXX: this will not work properly for other clock speeds, and
	// this code should use F_CPU to determine the prescale factor.
	sbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);
	sbi(ADCSRA, ADPS0);

	// enable a2d conversions
	sbi(ADCSRA, ADEN);
	
  /* configureta del analog, esto va a servir 
  ADCSRA = _BV(ADEN)|_BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0);        // prende el ADC, modo manual, no int, no inicia nada, prescaler a 1/128 (125KHz, 104us cada conversión)
  ADMUX = _BV(REFS0);                                         // setea el Vref
  */

}

void ADCInitConv (byte pin) {
  ADMUX = (ADMUX & 0xe0) | (pin & 0x07);             // setea el canal ADC
  delayMicroseconds(2);
  ADCSRA |= _BV(ADSC);                            // inicia una nueva conversión manual
}

int ADCReadConv () {
  while (ADCSRA & _BV(ADSC)) ;                   // espera hasta que se apague ADSC (está pronta la conversión)  
  return ADC;                                    // equivale al analogRead
}


// rutinas del SPI

void SPIconfig () {
  pinMode (SCK, OUTPUT);
  pinMode (MOSI, OUTPUT);
  pinMode (SS, OUTPUT);
  digitalWrite (SCK, LOW);
  digitalWrite (MOSI, LOW);
  digitalWrite (SS, HIGH);
  /* Warning: if the SS pin ever becomes a LOW INPUT then SPI automatically switches to Slave,  
     so the data direction of the SS pin MUST be kept as OUTPUT.  */
  SPCR |= _BV(MSTR);
  SPCR |= _BV(SPE);
}

byte SPItransfer (byte _data) {
  SPDR = _data;
  while (!(SPSR & _BV(SPIF))) ;     // por alguna razón esto no anda cuando está adentro de una ISR
  delayMicroseconds (5);           // por otra misteriosa razón hay que poner este molesto delay acá
  return SPDR;
}

//=================================================================================================================//
//                                   interrupt service routines & timers                                           //
//=================================================================================================================//

// variables para la delay(), etc.
volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;

volatile byte timer0_int_flag = 255;        // permite registrar y desregistrar tareas en la interrupción del timer0

ISR (TIMER0_OVF_vect, ISR_BLOCK)   
{
	// copy these to local variables so they can be stored in registers
	// (volatile variables must be read from memory on every access)
	unsigned long m = timer0_millis;
	unsigned char f = timer0_fract;

	m += MILLIS_INC;
	f += FRACT_INC;
	if (f >= FRACT_MAX) {
		f -= FRACT_MAX;
		m += 1;
	}
  
	// comienza zona de tareas automáticas
	if (timer0_int_flag & 1) {mov.update (m);}                    // caminata; esto puede durar 4,5 ms
	if (timer0_int_flag & 2) {pantalla.update (m);}                // display
	if (timer0_int_flag & 4) {kbza.comm.leer_serial();}             // polling del sensor
	// termina zona de tareas automáticas
	
	timer0_fract = f;
	timer0_millis = m;
	timer0_overflow_count++;
}

ISR (TIMER2_OVF_vect, ISR_BLOCK) {         
  TCNT2 = overflow;                // cada vez que se produce un overflow, el timer arranca en este valor y cuenta hasta 255.
  eventos.process ();              // se ejecuta cada TICK. Devuelve la cantidad de motores actualizados, dato que se podría usar
  // etc., etc., etc.
}

//=============================================================================================================================//
//                 implementación de millis(), delay(), etc., basada en el timer0, proveniente de wiring.c                     //
//=============================================================================================================================//

unsigned long millis()
{
	unsigned long m;
	uint8_t oldSREG = SREG;

	// disable interrupts while we read timer0_millis or we might get an
	// inconsistent value (e.g. in the middle of a write to timer0_millis)
	cli();
	m = timer0_millis;
	SREG = oldSREG;

	return m;
}

unsigned long micros() {             // esta rutina sólo se usaría para diagnóstico
	unsigned long m;
	uint8_t oldSREG = SREG, t;
	
	cli();
	m = timer0_overflow_count;
#if defined(TCNT0)
	t = TCNT0;
#elif defined(TCNT0L)
	t = TCNT0L;
#else
	#error TIMER 0 not defined
#endif

#ifdef TIFR0
	if ((TIFR0 & _BV(TOV0)) && (t < 255))
		m++;
#else
	if ((TIFR & _BV(TOV0)) && (t < 255))
		m++;
#endif

	SREG = oldSREG;
	
	return ((m << 8) + t) * (64 / clockCyclesPerMicrosecond());
}

void delay (unsigned long ms)    /* volvemos a la delay() de la 0018, 
																		  porque la de la 0022 no es recomendable en un entorno con interrupciones */ 
{
	unsigned long start = millis();
	
	while (millis() - start < ms)   // antes era <=
	      ;
}

/* Delay for the given number of microseconds.  Assumes a 8 or 16 MHz clock. */
void delayMicroseconds (unsigned int us)
{
	// calling avrlib's delay_us() function with low values (e.g. 1 or
	// 2 microseconds) gives delays longer than desired.
	//delay_us(us);

#if F_CPU >= 16000000L
	// for the 16 MHz clock on most Arduino boards

	// for a one-microsecond delay, simply return.  the overhead
	// of the function call yields a delay of approximately 1 1/8 us.
	if (--us == 0)
		return;

	// the following loop takes a quarter of a microsecond (4 cycles)
	// per iteration, so execute it four times for each microsecond of
	// delay requested.
	us <<= 2;

	// account for the time taken in the preceeding commands.
	us -= 2;
#else
	// for the 8 MHz internal clock on the ATmega168

	// for a one- or two-microsecond delay, simply return.  the overhead of
	// the function calls takes more than two microseconds.  can't just
	// subtract two, since us is unsigned; we'd overflow.
	if (--us == 0)
		return;
	if (--us == 0)
		return;

	// the following loop takes half of a microsecond (4 cycles)
	// per iteration, so execute it twice for each microsecond of
	// delay requested.
	us <<= 1;
    
	// partially compensate for the time taken by the preceeding commands.
	// we can't subtract any more than this or we'd overflow w/ small delays.
	us--;
#endif

	// busy wait
	__asm__ __volatile__ (
		"1: sbiw %0,1" "\n\t" // 2 cycles
		"brne 1b" : "=w" (us) : "0" (us) // 2 cycles
	);
}