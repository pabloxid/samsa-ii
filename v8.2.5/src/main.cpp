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
// version: 0.8.2.5
// date: 10/4/2011
// authors: Pablo Gindel, Jorge Visca

// hay que pelar todo lo que sobra, rediseñar todo en torno al nuevo concepto de "conductas"
// idea: crear estructuras para todas las variables que andan sueltas por ahi
// la parte de control del hardware hay que pulirla un cacho
// ordenar las cosas, que haya un método estandar para habilitar o deshabilitar cosas, o para acceder a la info

#include "movimiento.h"
#include "cabeza.h"
#include "hardware.h"
#include "display.h"
#include "util.h"
#include "mov_bajo_nivel.h"
#include "HardwareSerial.h"
#include "bluetooth.h"
#include "conducta.h"


int main (void)
{
	// los objetos que usan Serial, hay que inicializarlos acá (en tiempo de ejecución)
	kbza = Cabeza();
	blue = Bluetooth();
	// termina inicialización de objetos que usan Serial. 
	
	setTimers();                /* configura los timers y arranca las interrupciones
	                                   sustituye a la antigua init() de Arduino */
																		 
	SPIconfig ();                                          			  // inicializa el SPI para el display
	pantalla.setColor (DEGRADE_H, RGB(1, 3, 0), RGB(1, 3, 3));     			  // setea el color para el display
	motor_setup ();                                         			// inicializa y testea los 18 motores AX12
	ADCconfig ();                                          			  // configura el ADC     
	ADCInitConv (0);																              				// esto equivale a un analogRead(0)...
	randomSeed (ADCReadConv());                               			  // ...que se usa para inicializar el Random
	kbza.set_pos (PAN, 0); 
	kbza.set_pos (TILT, 400);               	                      // posiciona la cabeza
	pantalla.conway (60, 10); delay (700);                        		// genera un efecto visual
	pantalla.scrollText (" SAMSA II (c) 2010, P.Gindel & J.Visca ", 73); 	 // presentación 
    
	// obtengo una posición inicial con hexagono()
	// nota: en 'vectores' está toda la aritmética que permite operar con estas posiciones antes de pasárselas a ningún movimiento
	// mov.goto_pos_ref (hexagono (7, 30, 35, 35).patas);
	mov.goto_pos_ref ();  // la posición por defecto ya está definida en la clase
	
	delay (300);                       // pequeño delay para que inicialice el módulo bluetooth 
	bool bt = blue.connect();            // inicializa e intenta conectar el bluetooth
	
	while (pantalla.isBusy()) {delay (200);}    // espera que termine el mensaje de bienvenida
	
	if (bt) {
		pantalla.scrollText (" BT conect. ", 65);
	} else {
		pantalla.scrollText (" BT desconect. ", 65);
	}
	
	conducta_init ();                          // inicializa las conductas (alto nivel)
	
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// NOTA: la inicialización debe insumir unos 10 segundos para darle tiempo a la Arduino Mini para bootear  //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// main
	
	
	while (1) {
		delay (10);
		conducta_main ();
	}
	
	return 0;

}