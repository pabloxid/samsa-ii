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

// movimiento.h
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

#ifndef MOVIMIENTO_H
#define MOVIMIENTO_H

#include "vectores.h"
#include "util.h"

#define MAX_FASES         6        // máximo de fases de una secuencia de pasos en la caminata
#define TPS               14       // ticks-per-segment, constante para las curvas bezier
#define CIRCLE_RES        50       // número de subdivisiones de la circunferencia, para movimientos oscilatorios

typedef struct {float amp; float freq; float phase; bool brown;} OSCILATOR;

float const angle_step = 2*PI/CIRCLE_RES;

class Movimiento {

  public:
		
	  Movimiento ();
		
	  // general
		void set_pos_ref (COORD3D *pos_ref);
		void actual_pos_ref (); 
		void goto_pos_ref (COORD3D posicion[]); 
		void goto_pos_ref ();
		
		// caminatas
		void start ();
		void stop ();
		void cont ();
		void caminata (float velocidad, float desplazamiento, bool curva, COORD2D centro, float angulo, byte marcha=1, float largo_pasos=0);
		void recta (float velocidad, float desplazamiento, float angulo, byte marcha=1, float largo_pasos=0);
		void curva (float velocidad, float desplazamiento, COORD2D centro, bool sentido, byte marcha=1, float largo_pasos=0);
		
		// cambio de parámetros de la caminata "on the fly"
		byte inc_escala ();     
		byte dec_escala ();
						
		// movimientos del tronco
		void translation (COORD3D vector, int duracion, byte nsegmentos=0);
		void rotation (COORD3D centro, float angulox, float anguloy, float anguloz, int duracion, byte nsegmentos=0);
		void set_oscilador (byte parametro, float amplitud, float frecuencia, float fase, bool brown); 
		void oscilador (float frecuencia_fund, int duracion); 
		
		// rutina universal para que camine la cosa
		void update (unsigned long milis);
		
		/* monitores (para generar eventos o controlar cosas con la caminata)
		   son públicos, para evitar tener que hacer un setter para cada uno
		   no olvidarse de ponerlos a NULL cuando no se usan más */
		float *mon_angulo;
		COORD2D *mon_desplazamiento;
		// etc. los vamos creando a medida que se precisen

  private:
		void pasito (byte patas, COORD3D destino, bool absolute, float comienzo, int duracion, byte nsegmentos, COORD3D manejador);
		void tronco (COORD3D *pos_ref, COORD3D traslacion, COORD3D centro, float angulox, float anguloy, float anguloz, int duracion, byte nsegmentos=0);
		void caminata (COORD2D vector, float rotacion, unsigned int ticks, byte *secuencia, byte fases, byte agrupamiento, byte escala, int periodo_sub_ciclo, int periodo_pasos, int duracion_pasos, float altura_pasito, byte nsegmentos, bool compensate);  
		void posicion (COORD3D posicion[], int comienzo, int duracion);
		byte nseg (int duracion);
		byte nseg (int duracion_pasos, byte escala, float largo_pasos);
		
		// parámetros
		COORD3D pos_ref_ [6];                                  // posición de referencia
		COORD2D vector_;                                      // vector de traslación | centro de rotación
		float rotacion_;                                      // ángulo de rotacion
		unsigned int ticks_;                                   // duración del movimiento
		byte secuencia_ [MAX_FASES];                            // secuencia de patas
		byte fases_, agrupamiento_, escala_;                      
		int periodo_sub_ciclo_, periodo_pasos_, duracion_pasos_;
		bool compensate_;     
		float altura_pasito_;                                  
		byte nsegmentos_;
		OSCILATOR oscilator [9];
							
	 // variables de control de la clase
	 bool init;
	 bool enable;
	 byte mode;
	 unsigned int tick;                    // contador principal
	 int pausa;                           // pausa
   
};

extern Movimiento mov;

// defines
#define CW     0
#define CCW    1

// modos
#define CAMINATA        0
#define TRONCO          1

enum {TRASL_X, TRASL_Y, TRASL_Z, CENTRO_X, CENTRO_Y, CENTRO_Z, ROT_X, ROT_Y, ROT_Z};

#endif