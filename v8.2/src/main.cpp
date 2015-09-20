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
// version: 0.8.2
// date: 10/4/2011
// authors: Pablo Gindel, Jorge Visca

// hay que pelar todo lo que sobra, rediseñar todo en torno al nuevo concepto de "conductas"


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
	
	// comunicación con el ATtiny85
	pinMode (3, INPUT);          // request int1
	pinMode (4, INPUT);          // sensor_id
	pinMode (5, INPUT);          // sign_id
	
	setTimers();                /* configura los timers y arranca las interrupciones
	                                   sustituye a la antigua init() de Arduino */
																		 
	SPIconfig ();                                          			// inicializa el SPI para el display
	pantalla.setColor (DEGRADE_H, RGB(1, 3, 0), RGB(1, 3, 3));     			// setea el color para el display
	motor_setup ();                                         			// inicializa y testea los 18 motores AX12
	ADCconfig ();                                          			// configura el ADC     
	ADCInitConv (0);																              				// esto equivale a un analogRead(0)...
	randomSeed (ADCReadConv());                               			// ...que se usa para inicializar el Random
	kbza.posicion (PAN, 0); kbza.posicion (TILT, 400);               	// posiciona la cabeza
	pantalla.conway (60, 10); delay (700);                        		// genera un efecto visual
	pantalla.scrollText (" SAMSA II (c) 2010, P.Gindel & J.Visca ", 73); 	// presentación 
    
	// obtengo una posición inicial con hexagono()
	// nota: en 'vectores' está toda la aritmética que permite operar con estas posiciones antes de pasárselas a ningún movimiento
	// mov.goto_pos_ref (hexagono (7, 30, 35, 35).patas);
	mov.goto_pos_ref ();  // la posición por defecto ya está definida en la clase
	
	delay (300);                       // pequeño delay para que inicialice el módulo bluetooth 
	bool bt = blue.connect();            // inicializa e intenta conectar el bluetooth
	
	while (pantalla.isBusy()) {delay (200);}    // espera que termine el mensaje de bienvenida
	
	if (bt) {
		pantalla.scrollText (" BT Conectado ", 68);
	} else {
		pantalla.scrollText (" BT desconectado ", 68);
	}
	
	attachInterrupt(1, sensor_cuerdita, RISING);   // define la interrupción del sensor de cuerdita (ATtiny85)
	//timer0_int_flag |= _BV(3);                 // prende el polling analógico (sensor de cuerdita)
	
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// NOTA: la inicialización debe insumir unos 10 segundos para darle tiempo a la Arduino Mini para bootear  //
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
		// empieza zona de prueba
	
	/* esto en combinación con lo provisorio que está en cabeza::callback() 
	   para probar rápidamente el sensor de adelante
	
  kbza.enable_send_all();
	
	// tranca
	while (1) {
		delay (80);
	}
	*/
		
		
	extern volatile bool cuerdita;
	cuerdita = false;
	bool is_moving = false;
	conducta_init ();
	
	while (1) {                                                       // este es el loop prueba-tutti
	  main_ ();
		
		/*
	  static byte shape = 0;
	  static int timeout = 0;
		// asocia el movimiento de cabeza al sensor de atrás
		float vel = (1023-sns_fuerza) * .03;
		float ang = -HALF_PI + sns_angulo*PI/512;
		if (is_moving) {
			if (vel < 7) {
				mov.stop();
			  mov.goto_pos_ref ();
				delay (200);
				is_moving = false;
			}
		} else {
			if (cuerdita != cuerdita) {
				if (vel>=8) {
					if (abs (ang-HALF_PI)<.15) {mov.recta (vel, -1, HALF_PI);}
					else {mov.curva (vel, abs (ang-HALF_PI)*60, (COORD2D){0,0}, (ang<HALF_PI));}
					is_moving = true;
				}
				cuerdita = false;
			} else {
				kbza.posicion(PAN, (sns_angulo-512)*3);
				kbza.posicion(TILT, 1324-sns_fuerza);
			}
		}
		// prueba el sensor de distancia y la "medidor"
		kbza.request (CM_DIST);
		char cercania = 13-2.5*log(1+kbza.cm_dist);
		if (!pantalla.isBusy()) {pantalla.medidor (shape, cercania);}
		if (cercania < 1) {timeout++;}
		if (cercania >= 5) {mov.stop();}
		delay(10);
		
		// randomiza el shape y el color
		if (timeout > 250) {
			byte r1 = random(4);
			byte r2 = random(4);
			byte g1 = random(1,4);
			byte g2 = random(1,4);
			byte b1 = random(1,4);
			byte b2 = random(1,4);
			if (!pantalla.isBusy()) {pantalla.setColor (random(3), RGB(r1,g1,b1), RGB(r2,g2,b2));}
			shape = random(OSC_H, ALEATORIO);
			timeout = 0;
		}
				
		for (byte pata=0; pata<6; pata++) {
			blue.send_load (pata);			
		}
		*/
		
	 
	  delay (25);
	}
	
	
	return 0;

}