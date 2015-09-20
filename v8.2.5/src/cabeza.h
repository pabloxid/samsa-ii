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

// cabeza.h
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

/* contiene las rutinas de:
  - el sensor inteligente de distancia
	- los servos de la cabeza
	- el control remoto IR                 */

#ifndef CABEZA_H
#define CABEZA_H

#include "serialcomm.h"

// servos de la cabeza
#define S_1_PIN      46
#define S_2_PIN      45
#define S_CENTER     3300
#define PAN          0
#define TILT         1

// parámetros del sensor inteligente de distancia (se puede hacer un enum)
#define CM_DIST      0
#define VEL          1
#define LAT          2
#define EVNT_VEL     3
#define EVNT_LAT     4
#define MM_DIST      5
#define ERROR        -255

typedef unsigned char byte;

#define disable_polling() {bitClear (timer0_int_flag, 2);}           // deshabilita el polling del sensor
#define enable_polling() {bitSet (timer0_int_flag, 2);}              // rehabilita el polling del sensor

// NOTA: para habilitar el polling hay que hacer un enable_send_all() y un set_time()
// o de lo contrario, hacer periódicamente un 'request()' 

class Cabeza {
	
	public:
		Cabeza ();                                               // construtor
		void set_pos (byte servo, int angulo, bool absolute=true);        // setea la posición de los servos
		int get_pos (byte servo);                                   // devuelve la posición de los servos
		int read_value (byte parametro);                             // leer valor (DEPRECATED: se recomienda usar 'request' en su lugar)
		void request (byte parametro);                               // solicita un valor al sensor inteligente
		void set_threshold (byte parametro, unsigned int valor);          // setea un umbral del sensor inteligente
		void set_time (byte ms);                                    // setea el período de muestreo del sensor inteligente
		void enable_send_all ();                                    // habilita el muestreo del sensor inteligente
		void disable_send_all ();                                   // deshabilita el muestreo del sensor inteligente
		static void callback (byte instruccion, byte largo, byte* data);   // procesa la info que llega del sensor inteligente 
		 
		// variables que contienen los datos del sensor (públicas)
		static volatile byte cm_dist;
		static volatile char vel, lat, evnt_vel, evnt_lat;
		static volatile int mm_dist;
		static volatile byte flag;         // notifica la llegada de información
		
		// objeto Serialcomm, necesario para la comunicación con el sensor inteligente
		// es público porque se accede a él desde la ISR del timer0
		Serialcomm comm;           
		
	private:
		static volatile char* values[4];                // array de punteros a las variables, para indexarlas
		static int twobyte2int (byte* data); 
		int angulo_act [2];                          // posición actual de los servos (en unidades servísticas, que son de -750 a 750, más o menos)
		
};

extern Cabeza kbza;      // preinstanciado del objeto Cabeza 

#endif

/*  RESUMEN DEL LENGUAJE USADO PARA EL SENSOR 'INTELIGENTE' DE DISTANCIA
  
  data > 224 --> cabecera
  data - 224 --> instruccion * largo
  
  instruccion 0-7   --> sin parámetros
  instruccion 8-15  --> 1 parámetro
  instruccion 16-23 --> 2 parámetros
  instruccion 24-31 --> 3 parámetros
  
  * SEND:
  
      inst 0 --> request cm dist
      inst 1 --> request vel
      inst 2 --> request lat
      inst 3 --> request event_vel
      inst 4 --> request event_lat
      inst 5 --> request mm dist
      
      inst 6 --> enable send all
      inst 7 --> disable send all
      
      inst-1 0 --> set thresh cm dist
      inst-1 1 --> set thresh vel
      inst-1 2 --> set thresh lat
      inst-1 3 --> set thresh event_vel
      inst-1 4 --> set thresh event_lat
      
      inst-1 5 --> set time (pause)
      
      inst-2 0 --> set thresh mm dist
 
  * RECEIVE:
    
      inst-1 0 --> cm dist value
      inst-1 1 --> vel value
      inst-1 2 --> lat value
      inst-1 3 --> event_vel value
      inst-1 4 --> event_lat value
			
			inst-1 7 --> IR command
      
      inst-2 0 --> mm dist value
      
      inst-3 7 --> send_all packet
       
 */
 
 //////////////////////////////////////////////////////////////////////////////////////////////////
 ///////////////////////////// EJEMPLOS DE UTILIZACION DE LA BIBLIOTECA ///////////////////////////
 //////////////////////////////////////////////////////////////////////////////////////////////////
  
 /* método 1 para leer sensor (manual, sin polling, NO recomendado)									
	
	disable_polling();
										
  while (1) {
    
    int mm = read_value (MM_DIST);
    int vel = read_value (VEL);
    int lat = read_value (LAT);
   
    Serial.print (mm, DEC);
		Serial.print ("    ");
		Serial.print (vel, DEC);
		Serial.print ("    ");
		Serial.println (lat, DEC);
      
    delay (10);
		
  }  
  
	
	// método 2 para leer sensor (con polling, Recomendado)
	
	while (1) {
    
    kbza.request (MM_DIST);
    kbza.request (VEL);
    kbza.request (LAT);
         
		Serial.print (kbza.mm_dist, DEC);
		Serial.print ("    ");
		Serial.print (kbza.vel, DEC);
		Serial.print ("    ");
		Serial.println (kbza.lat, DEC);
				
	  delay (10);
  
	}
	
*/