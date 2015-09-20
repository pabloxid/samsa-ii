/*
  Part of the SAMSA II project
	http://www.pablogindel.com/trabajos/samsa-ii-2010/
  
	Copyright (c) 2010 Pablo Gindel & Jorge Visca, Montevideo - Uruguay. All rights reserved.

*/

// main.cpp
// version: 0.8.3
// date: 1/1/2012
// authors: Pablo Gindel, Jorge Visca

// usar con firmware_cabeza 04a 

// hay que pelar todo lo que sobra, rediseñar todo en torno al nuevo concepto de "conductas"
// idea: crear estructuras para todas las variables que andan sueltas por ahi
// la parte de control del hardware hay que pulirla un cacho
// ordenar las cosas, que haya un método estandar para habilitar o deshabilitar cosas, o para acceder a la info
// faltan #defines y comentarios a bocha
// la parte de caminata hay que seguirla puliendo

#include "movimiento.h"
#include "cabeza.h"
#include "hardware.h"
#include "display.h"
#include "util.h"
#include "mov_bajo_nivel.h"
#include "HardwareSerial.h"
#include "bluetooth.h"
#include "conducta.h"
#include "settings.h"

int main (void)
{
	// los objetos que usan Serial, hay que inicializarlos acá (en tiempo de ejecución)
	kbza = Cabeza();
	blue = Bluetooth();
	// termina inicialización de objetos que usan Serial. 
	
	init_filters ();             // necesario para el poll_load
	
	setTimers();                /* configura los timers y arranca las interrupciones
	                                   sustituye a la antigua init() de Arduino */
	SPIconfig ();                                          			  // inicializa el SPI para el display
	pantalla.setColor (DEGRADE_H, RGB(1, 3, 0), RGB(1, 3, 3));     	  // setea el color para el display
	motor_setup ();                 								  // inicializa y testea los 18 motores AX12
	ADCconfig ();                                          			  // configura el ADC     
	ADCInitConv (0);										          // esto equivale a un analogRead(0)...
	randomSeed (ADCReadConv());                               	      // ...que se usa para inicializar el Random
 //	kbza.set_pos (PAN, 0); 
 //	kbza.set_pos (TILT, 400);               	                              // posiciona la cabeza
	pantalla.conway (60, 10); delay (700);                        		// genera un efecto visual
	if (BOOT_MSG) {
		pantalla.scrollText (" SAMSA II (c) 2010, P.Gindel & J.Visca ", 73); 	 // presentación 
	}
	
	// obtengo una posición inicial con hexagono()
	// nota: en 'vectores' está toda la aritmética que permite operar con estas posiciones antes de pasárselas a ningún movimiento
	// mov.goto_pos_ref (hexagono (7, 30, 35, 35).patas);
	mov.goto_pos_ref ();  // la posición por defecto ya está definida en la clase
	
	// inicialización del módulo bluetooth
	while (millis() < 1800) ;       // garantiza que pasaron 1,8 segundos para que inicie el hardware
	bool bt = blue.connect();       // inicializa e intenta conectar el bluetooth
	
	while (pantalla.isBusy()) ;    // espera que termine el mensaje de bienvenida
	
	if (bt) {
		pantalla.scrollText (" BT conect. ", 65);
	} else {
		pantalla.scrollText (" BT desconect. ", 65);
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// NOTA: la inicialización debe insumir unos 10 segundos para darle tiempo a la Arduino Mini para bootear  //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// while (millis() < 10000) ;       // garantiza que pasaron esos 10 segundos (ya no es necesario porque fue actualizado el bootloader de la Arduino Mini)
	
	// firmwares alternativos
	if (TEST_SENSOR) {
		kbza.autoMode();
		while (1) ;
	}
	
	if (SIMULACION) {
		while (1) {
			for (byte pata=0; pata<6; pata++) {
				blue.send_load (pata);			
			}
			delay (5);
		}
	}
	// fin firmwares alternativos
	// prosigue firmware normal con conductas	
	
	conducta_init ();                          // inicializa las conductas (alto nivel)
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                  main loop                                                        //
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
	while (1) {
		delay (10);          // !!
		conducta_main ();
	}
	
	return 0;

}