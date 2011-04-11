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

// main.cpp
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca


/* acá hay mucha desprolijidad, también, cosas que se repiten, etc.
   implementar los cambios de parámetros "on the fly"                */

#include "movimiento.h"
#include "cabeza.h"
#include "hardware.h"
#include "display.h"
#include "util.h"
#include "mov_bajo_nivel.h"
#include "HardwareSerial.h"


int main (void)
{
  setTimers();                /* configura los timers y arranca con las interrupciones periódicas.
	                                   sustituye a la antigua init() de Arduino */
																		 
  SPIconfig ();                                          // inicializa el SPI para el display
  pantalla.setColor (DEGRADE_H, RGB(1, 3, 0), RGB(1, 3, 3));     // setea el color para el display
  motor_setup ();                                         // inicializa y testea los 18 motores AX12
  ADCconfig ();                                          // configura el ADC     
  randomSeed (analogRead(0));                               // esto se va a ir, porque no va a existir más analogRead()
  kbza.posicion (PAN, 0); kbza.posicion (TILT, 400);            // posiciona la cabeza
  pantalla.conway (60, 10);                                 // genera un efecto visual
  delay (700);
  pantalla.scrollText (" SAMSA II (c) 2010, P.Gindel & J.Visca ", 75);    // presentación estilo 'cutcsa'
    
	
	// empieza zona de prueba
	
	//delay (9000);   // sería necesario este delay, a menos que pelemos el bootloader de la mini
								   // pero se compensa con todo el tiempo de inicialización
  
	
	// obtengo una posición inicial con hexagono()
	// nota: en 'vectores' está toda la aritmética que permite operar con estas posiciones antes de pasárselas a ningún movimiento
  // mov.goto_pos_ref (hexagono (7, 30, 35, 35).patas);
  mov.goto_pos_ref ();  // la posición por defecto ya está definida en la clase
	
	delay (2500);
	
	Serial.begin (115200);
	
	while (1) { 
		
		//Serial.print (analogRead(6)); Serial.print ("   ");
		//Serial.println (analogRead(7));
	  			
		delay (13);
	}
	
	// implementar un timeout para que vuelva lentamente a la posición
	// boton para recobrar la posición
	// modo para editar la posición
	// modo para oscilaciones
	// corregir los mensajes cuando se apreta muchas veces una tecla
	// colores en los mensajes (probablemente en la display)
	// "auto" en los parámetros que lo tienen
	// rotaciones que rotan el ángulo
	// emprolijar código: "cabeza" (clase, sistema de comandos) y movimiento (todo)
	
	return 0;

}

//========================================================================================================================================================
//*************************************************************  COMENTARIOS  ****************************************************************************
//========================================================================================================================================================

/* ejemplo de oscilador

    mov.set_oscilador (ROT_Z, .6, .05, 0, true); 
		mov.set_oscilador (ROT_Y, .6, .05, 0, true); 
		mov.oscilador (2, 1000);
		delay (5000);
		
*/


// método 1 para leer sensor (manual, sin polling, no recomendado)									
	
	/* 
	
	//Serial.begin (115200);
	
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
  } */ 
  
	/*
	
	// método 2 para leer sensor (con polling, recomendado)
	
	Serial.begin (115200);
	
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