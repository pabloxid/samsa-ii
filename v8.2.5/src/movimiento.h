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
#define CIRCLE_RES        24       // número de subdivisiones de la circunferencia, para movimientos oscilatorios

typedef struct {float amp; float freq; float phase; bool brown;} OSCILATOR;

float const angle_step = 2*PI/CIRCLE_RES;         // como su nombre lo dice

class Movimiento {

  public:
				
		Movimiento ();
		friend class RemoteControl;       // la RemoteControl es amiga de Movimiento
		
	  // general
		void set_pos_ref (COORD3D *pos_ref);
		void actual_pos_ref (); 
		void goto_pos_ref (COORD3D posicion[]); 
		void goto_pos_ref ();
		COORD3D* get_pos_ref ();
		void pasito (byte patas, COORD3D destino, bool absolute, float comienzo, int duracion, byte nsegmentos, COORD3D manejador);
		
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
		void set_vel (float velocidad);
						
		// movimientos del tronco
		void translation (COORD3D vector, int duracion, byte nsegmentos=0);
		void rotation (COORD3D centro, float angulox, float anguloy, float anguloz, int duracion, byte nsegmentos=0);
		void rotation (float angulox, float anguloy, float anguloz, int duracion, byte nsegmentos=0);
		void tronco (COORD3D *pos_ref, COORD3D traslacion, COORD3D centro, float angulox, float anguloy, float anguloz, int duracion, byte nsegmentos=0);
		void salto (float modulo, float angulo);

		
		// osciladores
		void set_oscilador (byte parametro, float amplitud, float frecuencia, float fase, bool brown); 
		OSCILATOR get_oscilador (byte parametro); 
		void set_amp (byte parametro, float value);
		void set_freq (byte parametro, float value);
		void set_phase (byte parametro, float value);
		void set_brown (byte parametro, bool value);
		void oscilador (float frecuencia_fund, int duracion);
		void osc_reset ();
		void osc_randomize (float amplitud, float frecuencia, byte parameters_flag=5, float curvature=2);
		
		// runtime component
		void update (unsigned long milis);
		
		/* monitores (para generar eventos o controlar cosas con la caminata)
		   son públicos, para evitar tener que hacer un setter para cada uno
		   no olvidarse de ponerlos a NULL cuando no se usan más */
		float *mon_angulo;
		COORD2D *mon_desplazamiento;
		// etc. los vamos creando a medida que se precisen
		
  private:
		
		// métodos
		void set_values (float velocidad, float desplazamiento, bool curva, COORD2D centro, float angulo, byte marcha, float largo_pasos);
		void caminata (bool compensate);  
		void posicion (COORD3D posicion[], int comienzo, int duracion);
		byte nseg (int duracion);
		byte nseg (int duracion_pasos, byte escala, float largo_pasos);
		void caminata_init ();
		void caminata_init2 ();
		COORD3D h_correct (COORD2D C);
		COORD3D h_correct (COORD2D C, byte pata);
	
		// esto de las estructuras está bien, pero hay que estudiar la posibilidad de 
		// implementarlo con clases anidadas, y usando herencia múltiple
		
		// variables exclusivas de los osciladores y el tronco
		struct {
			OSCILATOR oscilator [9];                            // contiene los parámetros de oscilación para cada variable
			float param_tronco [9];                             /* matriz de 9 parámetros para el movimiento del tronco
																		                                  contiene el valor actual de la variable   */
			COORD3D centro_ref;                                // centro de las rotaciones 
		} bd;	
			
		// variables exclusivas de la caminata
		struct {	
			// parámetros del movimiento
			struct {
				COORD2D vector;                                      // vector de traslación | centro de rotación
				float rotacion;                                      // ángulo de rotacion
				byte secuencia [MAX_FASES];                            // secuencia de patas
				byte fases, agrupamiento;                      
				int periodo_sub_ciclo, periodo_pasos, duracion_pasos;
				bool compensate;     
				float altura_pasito;
			} mv;
			// variables geométricas internas y de control (run-time)
			struct {
				float ciclo, coeficiente;
				COORD2D rotor1 [2], rotor2 [2];                      // talvez convendría liberar estos arrays mientras no se usan
				COORD3D comp [6], pos_ref [6];                        // para la compensación de posición de referencia
				byte index;                                       // índice de las fases 
				byte apoyadas;                                     // patas apoyadas 
				byte counter [MAX_FASES];                            // cuando vale 0, la pata se considera apoyada
				COORD3D normal; float d;                             // ecuación del plano del robot
			} rt;
		} wk;
		
		// variables compartidas (de la clase)
		struct {
			// de movimiento
			struct {
				COORD3D pos_ref [6];                                  // posición de referencia
				byte escala, nsegmentos;
				unsigned int ticks;                                   // duración del movimiento
			} mv;
			// de control
			struct {
				bool enable;
				byte mode;
				unsigned int tick;                 // contador principal
				int pausa;                       // pausa
			} cn;
		} sh;
   
};

extern Movimiento mov;

// defines
#define CW     0
#define CCW    1

// modos
#define CAMINATA        0
#define TRONCO          1
#define CAMINATA2	      2      // caminata distinta, sin secuencia fija

enum {TRASL_X, TRASL_Y, TRASL_Z, CENTRO_X, CENTRO_Y, CENTRO_Z, ROT_X, ROT_Y, ROT_Z};
enum {AMP, FREQ, PHASE, BROWN};

#endif