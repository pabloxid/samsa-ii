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

// hardware.h
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

#ifndef HARDWARE_H
#define HARDWARE_H

#include "pins_arduino.h"

void setTimers ();                    // configura los timers
void ADCconfig ();                    // configura el ADC
void ADCInitConv (byte pin);
int ADCReadConv ();
void SPIconfig ();                    // configura el SPI
byte SPItransfer (byte _data);

// importadas de wiring.h
unsigned long millis (void);
unsigned long micros (void);
void delay (unsigned long);
void delayMicroseconds (unsigned int us);

extern volatile byte timer0_int_flag;                // permite registrar y desregistrar tareas en la interrupción del timer0

void inline enable_ovf2() {TIMSK2 |= _BV(TOIE2);}                 // habilita el Overflow Interrupt (timer2)
void inline disable_ovf2() {TIMSK2 &= ~_BV(TOIE2);}               // deshabilita el Overflow Interrupt (timer2)
void inline enable_ovf0() {TIMSK0 |= _BV(TOIE0);}                 // habilita el Overflow Interrupt (timer0)
void inline disable_ovf0() {TIMSK0 &= ~_BV(TOIE0);}               // deshabilita el Overflow Interrupt (timer0)

#define RESTORE true
void all_timers_off (bool restore = false);


// defines para las rutinas de temporización

/* the prescaler is set so that timer0 ticks every 64 clock cycles, and the
   the overflow handler is called every 256 ticks.       */
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))

// the whole number of milliseconds per timer0 overflow
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)

/* the fractional number of milliseconds per timer0 overflow. we shift right
   by three to fit these numbers into a byte. (for the clock speeds we care
   about - 8 and 16 MHz - this doesn't lose precision.)                   */
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)


#endif