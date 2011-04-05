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

char sign (int numero) {                       // numero/abs(numero)
  char signo = 0;
  if (numero > 0) {signo = 1;}
  if (numero < 0) {signo = -1;}
  return signo;
}

void brownian_noise (float *ptr, float center, float amp, float damper) {          // suma un valor aleatorio a una variable #1
  *ptr += amp*damper*random(-1,1) + damper*(center - *ptr);
}

void curved_noise (float *ptr, float center, float amp, float curvature) {           // suma un valor aleatorio a una variable #2                                   
  char sign = bin2sign (*ptr > center);                                       // revisar que esté correcto el signo
  *ptr += amp * sign * log_random (curvature);
}

float log_random (float curvature) {              // curvature=1, random normal; curvature>1, valores próximos a 0; curvature<1, valores próximos a 1                      
  return pow(random(1), curvature); 
}

float sigmoide (float x) {
	return 1.0 / (1+exp(-x));
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


//===================================================================================================================================
//======================================= otras utils que se pueden llegar a necesitar ==============================================
//===================================================================================================================================

/*

unsigned int makeWord(unsigned int w) { return w; }
unsigned int makeWord(unsigned char h, unsigned char l) { return (h << 8) | l; }

int mcd (int a, int b) {                        // máximo común divisor, algoritmo recursivo clásico
  if (a % b == 0) {return b;}
  else {return mcd (b, a%b);}
}

byte maximo (float* array, byte largo) {            // rutina clásica para sacar el máximo (devuelve la posición del máximo en un array)
  byte maxx = 0;
  for (byte i=1; i<largo; i++) {                                                                   
    if (array [i] > array [maxx]) {maxx = i;}
  }
  return maxx;
}

float fourier (float ampl[], byte fase[], int index) {               // da el valor instantáneo de la suma de sinusoides con amplitudes ampl[] y fases fase[]
  float y = 0;
  for (byte freq=0; freq<9; freq++) {
    y += ampl[freq] * seno [(index*freq + fase[freq])%16];
  }
  return y;
}

void decrement_p (volatile byte *p) {                                                // decrementa la posicion en buffers circulares de tamaño=32
  *p = (*p - 1) & 31;  
}

byte ring (byte posicion, byte p) {                                                  // devuelve la posición relativa en buffers circulares de tamaño=32
  return (p + posicion) & 31;
}

byte markov (byte largo, ...) {             // devuelve un valor aleatorio según las probabilidades de transferencia
                                            // ejemplo: markov (7,1,3,1,0,2,0,5); --> devuelve valores de entre 0 y 6; 0 con 1/12, 1 con 3/12, 2 con 1/12, etc. de probabilidad
  int sum = 0;
  byte prob [largo];
  
	va_list valores;
  va_start(valores,largo);
  for (byte f=0; f<largo; f++) {
    prob[f] = va_arg(valores,int);
    sum += prob[f];
  }
  va_end(valores);
	
	float num = random (sum);
  byte i;
  for (i=largo-1; i>=0; i--) {                   
    sum -= prob[i];
    if (num >= sum) {break;}
  }
  return i;  
} 

COORD3D semisuma (COORD3D punto1, COORD3D punto2) {
  return (COORD3D) {(punto1.x+punto2.x)/2, (punto1.y+punto2.y)/2, (punto1.z+punto2.z)/2}; 
}

*/