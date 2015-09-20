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

// util.cpp
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

// herramientas

#include "util.h"
#include "ax12.h"         // por la bin2sign()
#include "wiring.h"
#include "Print.h"

char sign (int numero) {                       // numero/abs(numero)
  char signo = 0;
  if (numero > 0) {signo = 1;}
  if (numero < 0) {signo = -1;}
  return signo;
}

void brownian_noise (float *ptr, float center, float amp, float damper) {          // suma un valor aleatorio a una variable #1
  *ptr += amp*damper*random(-1,1) + damper*(center - *ptr);
}

float sigmoide (float x) {
	return 1.0 / (1+exp(-x));
}

String float2string (float number) {
	int parte_entera = number;
	int decimos = number*10 - parte_entera*10;
	int centesimos = number*100 - parte_entera*100 - decimos*10;
	String signo;
	if (number<0 && number>-1) {signo = "-";}
	return signo + String (parte_entera, DEC) + "," + String (abs(decimos), DEC) + String (abs(centesimos), DEC);
}

/////////////// importadas de "WMath.h" (en realidad de Wiring) //////////////////////

void randomSeed(unsigned int seed)
{
  if(seed != 0){
    srand (seed);
  }
}

float random(float howbig)
{
  if (howbig == 0){
    return 0;
  }
  return howbig*rand()/RAND_MAX;
}

float random(float howsmall, float howbig)
{
  if(howsmall >= howbig){
    return howsmall;
  }
  float diff = howbig - howsmall;
  return random(diff) + howsmall;
}

float map(float value, float istart, float istop, float ostart, float ostop) {
  return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}

float semisuma (float a, float b) {
	return (a + b) / 2;
}