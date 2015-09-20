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

// movimiento.cpp
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

/* acá hay mucha desprolijidad, también, cosas que se repiten, etc.
   implementar los cambios de parámetros "on the fly"                
	 corregir los nombres de las variables que son cualquiera          */

#include "movimiento.h"
#include "events.h"
#include <string.h>
#include "mov_bajo_nivel.h"      // para acceder a 'pos_des[]'

Movimiento mov;        // preinstanciado

///////////////////////////////////////// CONSTRUCTOR /////////////////////////////////////////

Movimiento::Movimiento () {           
	set_pos_ref (DEFAULT_POSITION);                      // posición de referencia (por las dudas que se olviden de setear)
	osc_reset ();                                    // inicializa los osciladores 
	bd.centro_ref = (COORD3D) {0, 0, 0};                 // centro de referencia de las rotaciones
	// inicializa los monitores a NULL
	mon_angulo = NULL;
	mon_desplazamiento = NULL;
	// inicia el estado de movimiento
	sh.cn.enable = false;
}

///////////////////////////////////////// ROTACIONES Y TRASLACIONES /////////////////////////////////////////

// traslación del tronco
void Movimiento::translation (COORD3D vector, int duracion, byte nsegmentos) {
  tronco (pos_des, vector, (COORD3D){0,0,0}, 0, 0, 0, duracion, nsegmentos);
}

// rotación del tronco
void Movimiento::rotation (COORD3D centro, float angulox, float anguloy, float anguloz, int duracion, byte nsegmentos) {
	tronco (pos_des, (COORD3D){0,0,0}, centro, angulox, anguloy, anguloz, duracion, nsegmentos);
}

// rotación usando el centro de referencia
void Movimiento::rotation (float angulox, float anguloy, float anguloz, int duracion, byte nsegmentos) {
	tronco (pos_des, (COORD3D){0,0,0}, bd.centro_ref, angulox, anguloy, anguloz, duracion, nsegmentos);
}

// rotaciones y traslaciones del tronco 
// las dos rutinas anteriores mueven desde la posición actual, 
// en cambio esta mueve desde la posición de referencia que le pasen
void Movimiento::tronco (COORD3D *pos_ref, COORD3D traslacion, COORD3D centro, float angulox, float anguloy, float anguloz, int duracion, byte nsegmentos) {
  
  if (nsegmentos == 0) {nsegmentos = nseg (duracion);}
	
  POSICION pos;
  memcpy (pos.patas, pos_ref, 6*sizeof(COORD3D));
  COORD3D *matrix;
  float coeficiente = 1.0 / nsegmentos;  // modulo vector
  float t = 0;
  int tps = duracion * coeficiente;    // ticks-per-segment
  
  bool do_traslate = (traslacion.x!=0 || traslacion.y!=0 || traslacion.z!=0);
  if (do_traslate) {traslacion = producto (traslacion, -coeficiente);}
  bool do_rotate = (angulox!=0 || anguloy!=0 || anguloz!=0);
  if (do_rotate) {
    matrix = (COORD3D*) malloc (3*sizeof(COORD3D));
    getRotationMatrix (matrix, angulox*-coeficiente, anguloy*-coeficiente, anguloz*-coeficiente);
  }
  
  for (byte i=0; i<nsegmentos; i++) {
    if (do_traslate) {pos = traslate (pos.patas, traslacion);}
    if (do_rotate) {pos = rotate (pos.patas, centro, matrix);} 
    posicion (pos.patas, t+.5, tps);
    t += coeficiente;
  }
    
  if (do_rotate) {
    free (matrix);
  }
}

void Movimiento::salto (float modulo, float angulo) {
	COORD3D vector = (COORD3D) {-modulo*cos(angulo), -modulo, -modulo*sin(angulo)};  
	mov.pasito (63, (COORD3D){0,0,0}, false, 0, 15, 5, vector);
}

///////////////////////////////////////// OSCILADORES /////////////////////////////////////////

void Movimiento::set_oscilador (byte parametro, float amplitud, float frecuencia, float fase, bool brown) {
	bd.oscilator [parametro] = (OSCILATOR) {amplitud, frecuencia, fase, brown};
}

OSCILATOR Movimiento::get_oscilador (byte parametro) {
	return bd.oscilator [parametro];
}

void Movimiento::set_amp (byte parametro, float value) {
	bd.oscilator [parametro].amp = value;
}

void Movimiento::set_freq (byte parametro, float value) {
	bd.oscilator [parametro].freq = value;
}

void Movimiento::set_phase (byte parametro, float value) {
	bd.oscilator [parametro].phase = value;
}

void Movimiento::set_brown (byte parametro, bool value) {
	bd.oscilator [parametro].brown = value;
}

void Movimiento::oscilador (float frecuencia_fund, int duracion) {
  // duración en TICKS (4ms), igual que en las rutinas del tronco, en la 'event', en la 'pasito'
	
	sh.mv.escala = 1.0/(CIRCLE_RES*TICK*frecuencia_fund);                   // período en TICKS para una subdivisión del círculo
	sh.mv.nsegmentos = nseg (sh.mv.escala);                     
	sh.mv.ticks = duracion/sh.mv.escala;
	sh.cn.mode = TRONCO;
	start ();
	
}

// restea los osciladores
void Movimiento::osc_reset () {
	for (byte i=0; i<9; i++) {
	  bd.oscilator[i] = (OSCILATOR) {0, 0, 0, false};             // inicializa los osciladores
	  bd.param_tronco[i] = 0;
	}
}

// prueba de randomizador
// nota: el centro de rotación tendría que ser una referencia seteable en la clase "movimiento", y su oscilación en torno a ella --> hecho
void Movimiento::osc_randomize (float amplitud, float frecuencia, byte parameters_flag, float curvature) { 
	/* parameters_flag = bit0 traslaciones, bit1 centro, bit2 rotaciones; default 7 o sea todo.
	   curvature afecta a las amplitudes, default = 3 o sea, tienden a 0   */
	for (byte index=0; index<3; index++) {
		bool active = parameters_flag & (1<<index);
		for (byte subindex=0; subindex<3; subindex++) {
			float amp=0, freq=0, fase=0;
			if (active) {
				amp	= amplitud * log_random (curvature);
				if (index==1) {amp *= 2;}                           // para el centro, más efecto       
				else if (index == 2) {amp /= 15;}                    // para los ángulos, radio=15
				freq = random (frecuencia/3, frecuencia*3);
				fase = random (-PI, PI);
			}
			mov.set_oscilador (3*index+subindex, amp, freq, fase, false);
		}
	}
}


///////////////////////////////////////// MISC. & PRIVATE /////////////////////////////////////////

void Movimiento::set_pos_ref (COORD3D *pos_ref) {
  memcpy (sh.mv.pos_ref, pos_ref, 6*sizeof(COORD3D));
}

void Movimiento::actual_pos_ref () { 
  memcpy (sh.mv.pos_ref, pos_des, 6*sizeof(COORD3D));
}

// adopta una posición directamente
void Movimiento::posicion (COORD3D posicion[], int comienzo, int duracion) {
  for (byte pata=0; pata<6; pata++) {
    eventos.add ((MOVDATA) {1<<pata, posicion[pata], duracion, true}, comienzo); 
  }
}

// adopta una posición indirectamente, dando "pasitos"
void Movimiento::goto_pos_ref (COORD3D posicion[]) {
	set_pos_ref (posicion);
	goto_pos_ref ();
}
	
void Movimiento::goto_pos_ref () {
  // esto por ahora está harcodeado, y muy probablemente se mantenga asi
	wk.mv.vector = (COORD2D) {0, 0};
	wk.mv.rotacion = 0;
	sh.mv.ticks = 90;
	memcpy (wk.mv.secuencia, (byte[]) {8, 2, 32, 1, 16, 4}, 6);
	wk.mv.fases = 6;
	wk.mv.agrupamiento = 1;
	sh.mv.escala = 1;
	wk.mv.periodo_sub_ciclo = 15;
	wk.mv.periodo_pasos = 15;
	wk.mv.duracion_pasos = 60;
	wk.mv.altura_pasito = 10;
	sh.mv.nsegmentos = 7;
	caminata (false);
}

COORD3D* Movimiento::get_pos_ref () {
	return sh.mv.pos_ref;
}

// curvas bezier (usada para dar "pasitos" en la caminata, pero se puede usar para muchas más cosas)
void Movimiento::pasito (byte patas, COORD3D destino, bool absolute, float comienzo, int duracion, byte nsegmentos, COORD3D manejador) { 
  
  float t = 0;
  float t_inc = 1.0 / nsegmentos;
  float tps = duracion * t_inc;         // ticks-per-segment
  COORD3D punto, last, origen;
  if (absolute) {
    origen = eventos.search (patas2pata(patas), comienzo);   // calcula la posición futura de una pata, en base a los eventos agendados
  } else { 
    origen = (COORD3D) {0, 0, 0};
    punto = origen;
    last = punto;
  }
  for (byte i=0; i<nsegmentos; i++) {
    t += t_inc;
    if (!absolute) {last = punto;}
    punto.x = bezier (origen.x, manejador.x, destino.x, t);
    punto.y = bezier (origen.y, manejador.y, destino.y, t);
    punto.z = bezier (origen.z, manejador.z, destino.z, t);
    eventos.add ((MOVDATA) {patas, resta (punto, last), tps, absolute}, comienzo);  
    comienzo += tps; 
  } 
}