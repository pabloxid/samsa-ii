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

// util.h
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

#ifndef UTIL_H
#define UTIL_H

// aca va toda la matemática
// ando con ganas de borrar este archivo
// o por lo menos repartir los defines a donde realmente se usan
// la razón de que los #defines y los 'const' estén acá es que son usados por más de un archivo (algunos, pero no todos)

#include "wiring.h"
#include <math.h>
#include "WString.h"
//#include <stdarg.h>                        // funciones con número variable de argumentos

// propiedades del robot
#define BAUDRATE   1000000                // velocidad de comunicación de los AX12
#define COXA       5.0                    // distancia entre el eje del primer motor y el segundo (en cm)
#define FEMUR      8.2                    // longitud del femur (en cm)
#define TIBIA      11.7                   // longitud de la tibia (en cm)
#define SEPARA     8.75                   // separación entre las inserciones de las patas (en cm) 
#define ORIGEN     4.6                    // distancia entre el origen de la x y el centro del tronco (en cm)
#define ANG_SCALE  614                    // unidades angulares, equivalente a PI o 180º
#define ANG_ZERO   518                    // offset angular (teóricamente tendría que ser 512, pero ya ves...)
#define TICK       .004                   // duración del tick en segundos
#define VEL_MAX    684                    // velocidad máxima de la escala en grados/segundo. 

/*  Nota: 119rpm según la info actualizada de Dynamixel, lo que equivale a 684º/s.
    No obstante, el motor no pasa de los 300º/s, por lo que adquiere su velocidad máxima por la mitad de la escala.
    En consecuencia, para velocidades mayores a 300º/s, el tiempo previsto no se va a cumplir, o los movimientos van a quedar incompletos.    */

#define DEFAULT_POSITION  hexagono (7.85, 30.4, 36, 35.7).patas	

#define IDLE_THRESH  237           // tiempo mínimo de inactividad en ticks      
																					 
// macros
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(x) ((x)>=0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))																					 
																					 
// constantes para acelerar el cálculo
float const doublea = 2 * FEMUR;
float const doubleab = 2 * FEMUR * TIBIA;
float const sqaplussqb = sq (FEMUR) + sq (TIBIA);
float const sqaminussqb = sq (FEMUR) - sq (TIBIA);
float const vel_scale = 184200.0 / (TICK*VEL_MAX*ANG_SCALE);         // velocidad(AX12) = unidades_angulares*vel_scale/ticks   
byte const overflow = 256 - (TICK*14000);                        // valor inicial del contador para el timer2 (no se usaría si el timer estuviera en CTC)

// funciones
char sign (int numero);
void brownian_noise (float *ptr, float center, float amp, float damper);
float random (float);
float random (float, float);
void randomSeed (unsigned int);
float map (float, float, float, float, float);
float sigmoide (double x);
float semisuma (float a, float b);
String float2string (float number);
float log_random (float curvature);

#endif

/* explicación del subsistema que controla la velocidad de los motores:

int diferencia = abs (angulo_destino - angulo_actual);                    // en unidades angulares AX12 (0-1023)
velocidad = constrain (vel_scale*diferencia/duracion, 1, 1023);           // en unidades de velocidad AX12 (0-1023)

*/