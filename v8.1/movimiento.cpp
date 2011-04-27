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

#include "movimiento.h"
#include "ax12.h"                // por la bin2sign()
#include "events.h"
#include <string.h>
#include "wiring.h"
#include "mov_bajo_nivel.h"      // para acceder a 'pos_des[]'

Movimiento mov;              // preinstanciado

Movimiento::Movimiento () {                 // constructor
	set_pos_ref (DEFAULT_POSITION);                            // posición de referencia (por las dudas que se olviden de setear)
	for (byte i=0; i<9; i++) {
	  oscilator[i] = (OSCILATOR) {0, 0, 0, false};                  // inicializa los osciladores
	  param_tronco[i] = 0;
	}
	// inicializa los monitores a NULL
	mon_angulo = NULL;
	mon_desplazamiento = NULL;
	// inicia el estado de movimiento
	enable = false;
}

///////////////////////////////////////// CAMINATAS /////////////////////////////////////////

// recta común
void Movimiento::recta (float velocidad, float desplazamiento, float angulo, byte marcha, float largo_pasos) {
  caminata (velocidad, desplazamiento, false, (COORD2D){0,0}, angulo, marcha, largo_pasos);
}
		
// curva común
void Movimiento::curva (float velocidad, float desplazamiento, COORD2D centro, bool sentido, byte marcha, float largo_pasos) {
  caminata (velocidad, desplazamiento, true, centro, bin2sign(sentido), marcha, largo_pasos);
}

// caminata con parámetros "inteligentes". 
void Movimiento::caminata (float velocidad, float desplazamiento, bool curva, COORD2D centro, float angulo, byte marcha, float largo_pasos) {
  // si curva == false -> caminata recta con dirección [angulo]
  // si curva == true  -> caminata curva con centro [centro] y sentido = signo de [angulo]
  // todos los parámetros son en centímetros y en segundos.
  // marcha admite los valores 1, 2 y 3 (es el numero de patas simultáneas)  
  // desplazamiento < 0 -> movimiento infinito, detenerlo con stop();
  
  if (largo_pasos == 0) {
		// esta fórmula para calcular automáticamente el largo de los pasos, contempla la distancia entre las patas
		// pero igual da cualquier fruta, hay que corregirla
		largo_pasos = sigmoide (velocidad/7) * .335 * distancia (suma (xyz2xz(pos_ref_[0]), getOffset (0)), suma (xyz2xz(pos_ref_[2]), getOffset (2))); 
  }
	
  // estos seteos hay que ajustarlos lo mejor posible
  #define PERIODO_MIN_TICKS  131   // aproximadamente 1/2 segundo para todo el ciclo
  #define Kmin               0.33  // aprox. 40 ticks por paso (podría estar en relación a largo_pasos, que a su vez podría estar en relación a velocidad)
  #define Kinc               0.1	  // define el punto de transición de un estilo a otro (mínimo de periodo_pasos)
       
	/* 1) En este mega-loop, vamos a calcular: 
	           K (relación entre duracion_paso y periodo_sub_ciclo), agrupamiento, periodo_sub_ciclo, marcha y fases */
  byte agrupamiento = 0, Amax = 0, fases;
  int periodo_sub_ciclo;
  float K = 0, Kmax = 0;
  marcha --;                     // marcha se va a incrementar por lo menos 1 vez
  do {
    if (K < Kmax-2*Kinc*(agrupamiento-1)) { // esto es otra manera de prevenir el overflow (antes era K < Kmax) 
      // K = min (K + .05, Kmax);            // esto previene el overflow, que después provoca errores en el cálculo de 'periodo_pasos'
      K += Kinc;
    } else {
      if (agrupamiento < Amax) {
        agrupamiento ++;
        Kmax = 3.0/(marcha*agrupamiento*1.2);    // revisar esto
        K = Kmin;
      } else {
        if (marcha < 3) {
          marcha ++;
          Amax = 3 / marcha;    // división entera: va a dar 1 si marcha>1
          fases = 6 / marcha;
          agrupamiento = 0;
        } else {break;}
      }
    }
    periodo_sub_ciclo = -largo_pasos*agrupamiento/(velocidad*TICK*(K*agrupamiento-fases));  // [velocidad*TICK] es [velocidad*TICK*escala], pero escala=1 en este momento.
  } while (periodo_sub_ciclo < PERIODO_MIN_TICKS);
  
	// 2) vamos a elegir la secuencia
  byte secuencia [fases];
  if (marcha == 1) {memcpy (secuencia, (byte[]){1, 16, 4, 8, 2, 32}, fases);}
  else if (marcha == 2) {memcpy (secuencia, (byte[]){33, 12, 18}, fases);}
  else {memcpy (secuencia, (byte[]){42, 21}, fases);}
  /* otras secuencias que se podrían haber utilizado son {1, 2, 4, 8, 16, 32} y {1, 32, 4, 8, 2, 16}; */
  
  // 3) vamos a calcular: periodo_pasos, duracion_pasos
  int periodo_pasos;
  int duracion_pasos = K * periodo_sub_ciclo;
  if (agrupamiento == 1) {
    periodo_pasos = periodo_sub_ciclo;
  } else {
    periodo_pasos = (Kmax - K) * periodo_sub_ciclo / agrupamiento + 2;    // revisar esto también   
  }
   
  // 4) vamos a calcular: escala (y escalar los 3 parámetros anteriores)
  byte escala = min (min(duracion_pasos, periodo_pasos), periodo_sub_ciclo) / 4;  // esto hace que luego del escalamiento, el parámetro más chico valga 4
  if (escala > 1) {
		periodo_sub_ciclo = 1.0*periodo_sub_ciclo/escala + 0.5;
		duracion_pasos = 1.0*duracion_pasos/escala + 0.5;
		periodo_pasos = 1.0*periodo_pasos/escala + 0.5;
	} else {escala = 1;}
  
  // 5) vamos a calcular: modulo_vector, ticks, altura_pasito, nsegmentos, compensate
  float modulo_vector = velocidad * TICK * escala;
  unsigned int ticks;
  if (desplazamiento >= 0) {ticks = desplazamiento / modulo_vector + duracion_pasos;} else {ticks = -1;}  
  /* a esta altura podemos hacer la siguiente operación comprobatoria: 
	    periodo_sub_ciclo = (largo_pasos/modulo_vector + duracion_pasos)*agrupamiento/fases */
  byte nsegmentos =  nseg (duracion_pasos, escala, largo_pasos);
  // fórmula para la altura del punto manejador de la curva bezier
  // contempla el largo y la duración de los pasos, pero igual da fruta
  float altura_pasito = 6.7 + 25*(8.4 + 2.6*sqrt(largo_pasos))/(duracion_pasos*escala);      // ajustar esto es casi imposible
  bool compensate = true;
	// hasta acá lo que es común a traslación y rotación

  // 6) vamos a calcular: los vectores de movimiento (el único paso que varía según sea recta o curva)
  COORD2D vector;
  float rotacion;
  if (!curva) {
    vector = (COORD2D) {-modulo_vector*cos(angulo), -modulo_vector*sin(angulo)};  // lo invierte para acelerar los cálculos
    rotacion = 0;
  } else {
    vector = centro;
    // medimos la pata más distante del centro
    float dist = 0;
    for (byte pata=0; pata<6; pata++) {
      float d = distancia (centro, suma (xyz2xz(pos_ref_[pata]), getOffset (pata)));
      if (dist < d) {dist = d;}
    } 
    rotacion = sign (angulo) * modulo_vector / dist;       // aproximación basada en el arco; también podría usarse la cuerda
  }
  
  caminata (vector, rotacion, ticks, secuencia, fases, agrupamiento, escala, periodo_sub_ciclo, periodo_pasos, duracion_pasos, altura_pasito, nsegmentos, compensate);
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


///////////////////////////////////////// OSCILADORES /////////////////////////////////////////

void Movimiento::set_oscilador (byte parametro, float amplitud, float frecuencia, float fase, bool brown) {
	oscilator [parametro] = (OSCILATOR) {amplitud, frecuencia, fase, brown};
}

OSCILATOR Movimiento::get_oscilador (byte parametro) {
	return oscilator [parametro];
}

void Movimiento::set_amp (byte parametro, float value) {
	oscilator [parametro].amp = value;
}

void Movimiento::set_freq (byte parametro, float value) {
	oscilator [parametro].freq = value;
}

void Movimiento::set_phase (byte parametro, float value) {
	oscilator [parametro].phase = value;
}

void Movimiento::set_brown (byte parametro, bool value) {
	oscilator [parametro].brown = value;
}

void Movimiento::oscilador (float frecuencia_fund, int duracion) {
  // duración en TICKS (4ms), igual que en las rutinas del tronco, en la 'event', en la 'pasito'
	
	escala_ = 1.0/(CIRCLE_RES*TICK*frecuencia_fund);                   // período en TICKS para una subdivisión del círculo
	nsegmentos_ = nseg (escala_);                     
	ticks_ = duracion/escala_;
	mode = TRONCO;
	start ();
	
}


///////////////////////////////////////// MISC. & PRIVATE /////////////////////////////////////////

void Movimiento::set_pos_ref (COORD3D *pos_ref) {
  memcpy (pos_ref_, pos_ref, 6*sizeof(COORD3D));
}

void Movimiento::actual_pos_ref () { 
  memcpy (pos_ref_, pos_des, 6*sizeof(COORD3D));
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
  caminata ((COORD2D) {0, 0}, 0, 90, (byte[]) {8, 2, 32, 1, 16, 4}, 6, 1, 1, 15, 15, 60, 10, 7, false);
}

COORD3D* Movimiento::get_pos_ref () {
	return pos_ref_;
}

// curvas bezier (usada para dar "pasitos" en la caminata, pero se puede usar para muchas más cosas)
void Movimiento::pasito (byte patas, COORD3D destino, bool absolute, float comienzo, int duracion, byte nsegmentos, COORD3D manejador) { 
  
  float t = 0;
  float t_inc = 1.0 / nsegmentos;
  float tps = duracion * t_inc;         // ticks-per-segment
  COORD3D punto, last, origen;
  if (absolute) {
    origen = eventos.search (patas2pata(patas), comienzo);
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

// caminata con parámetros "RAW"
void Movimiento::caminata (COORD2D vector, float rotacion, unsigned int ticks, byte *secuencia, byte fases, byte agrupamiento, byte escala, int periodo_sub_ciclo, int periodo_pasos, int duracion_pasos, float altura_pasito, byte nsegmentos, bool compensate) 
{  
   /* parámetros
   secuencia                      // orden de patas, por ejemplo {1, 2, 4, 8, 16, 32}, etc.  
   fases                          // número de elementos de la secuencia
   agrupamiento                   // cantidad de fases tomadas como una subunidad (puede ser 1, 2 o 3; para desactivar: agrupamiento=1, periodo_sub_ciclo=periodo_pasos)
   escala                         // cantidad de TICKs (ticks del subsistema de movimiento) por tick (tick de la caminata, micropaso). Es inversamente proporcional a la velocidad.
   periodo_sub_ciclo              // cantidad de ticks de una fase o un agrupamiento de fases 
   periodo_pasos                  // inverso de la frecuencia de los pasos  
   duracion_pasos                 // si es == periodo_pasos, significa no aire entre las fases... si es > periodo_pasos hay "solapamiento" 
   altura_pasito                  // altura de las curvas bezier (pasitos) 
   nsegmentos                     // resolución de las curvas bezier (idealmente: largo_pasos*escala/nsegmentos != escala, para distribuir mejor los eventos AX12)  
	 compensate                     // boolean que indica si se debe corregir progresivamente la posición de referencia  */
	 
		vector_ = vector;
		rotacion_ = rotacion;
		ticks_ = ticks;
		memcpy (secuencia_, secuencia, fases);
		fases_ = fases;
		agrupamiento_ = agrupamiento;
		escala_ = escala;
		periodo_sub_ciclo_ = periodo_sub_ciclo;
		periodo_pasos_ = periodo_pasos;
		duracion_pasos_ = duracion_pasos;
		altura_pasito_ = altura_pasito;
		nsegmentos_ = nsegmentos;
		compensate_ = compensate;
	 	 
	 // variables de control
	if (!enable || mode!=CAMINATA) { 
		mode = CAMINATA;
		start ();
	} else {
		caminata_init2 ();
	}
	
}


////////////////////////////////////////// RUNTIME ///////////////////////////////////////////

// idea: tanto para la "modulación" como para arrancar caminatas desde cualquier 
// posición y que lentamente se vaya acomodando, la clave es: transformar pos_ref_

void Movimiento::update (unsigned long milis) {        // esto es un kilombo. Solución: la "update" pasaría a ser una clase nueva
  
  // única variable de control exclusiva de la "update"
  static unsigned long timer = 0;                         // timer
  
  // verifica si pasó x tiempo desde la última vez que se ejecutó el micropaso, y si no, retorna sin hacer nada 
  if (milis-timer < pausa) {return;} 
  timer = milis;
  
	// empieza el loop (bloque activo)
	if (enable) {
		if (tick < ticks_) {
			switch (mode) {
				case CAMINATA: {
			
					// compensación de posición de referencia
					if (compensate_ && tick < 2*ciclo) {
						for (byte pata=0; pata<6; pata++) {
							sumasigna (&pos_ref[pata], comp[pata]);
						}
					}
					
					// tratamiento de los "pasitos"
					if ((tick%periodo_sub_ciclo_)-periodo_pasos_*(index%agrupamiento_) == 0) {
									
						for (byte pata=0; pata<6; pata++) {
							if ((secuencia_[index]>>pata)&1) {
								COORD2D C;
								float atenuador; 
								
								// "zancada progresiva"
								if (tick <= 2*ciclo) {atenuador = 1.6 * sigmoide(.5*tick/ciclo) - .6;} else {atenuador = 1;}
								
								if (rotacion_ == 0) {
									// traslación
									C = suma(xyz2xz (pos_ref[pata]), producto(vector_, -coeficiente*atenuador)); 
									if (tick > 2*ciclo) {
										C = resta (C, xyz2xz(pos_des[pata]));
										// si las condiciones están dadas, agenda un solo pasito relativo para todas las patas y sale
										pasito (secuencia_[index], xz2xyz(C), false, 0, escala_*duracion_pasos_, nsegmentos_, suma(xz2xyz(producto(C, .5)), (COORD3D){0,altura_pasito_,0}));
										break;
									}
								} else {
									// rotación
									COORD2D O = getOffset (pata);
									C = resta (suma (applyMatrix (resta (suma (xyz2xz(pos_ref[pata]), O), vector_), rotor2), vector_), O);   
									// en la rotación, hacer la "zancada progresiva" es un poco más complicado..
									// como no podemos recalcular la matriz de rotación, hacemos interpolación lineal
									if (tick <= 2*ciclo) {
										C = suma (producto (resta (C, xyz2xz(pos_ref[pata])), atenuador), xyz2xz(pos_ref[pata]));
									}
								} 
								
								pasito (1<<pata, (COORD3D) {C.x, pos_ref[pata].y, C.z}, true, 0, escala_*duracion_pasos_, nsegmentos_, suma(pos_ref[pata], (COORD3D){0,altura_pasito_,0}));
							}
						}
				
						counter[index] = duracion_pasos_;
						index = (index+1)%fases_;
					}
					
					// determinación de las patas apoyadas
					apoyadas = 0;
					for (byte i=0; i<fases_; i++) {
						if (counter[i] > 0) {counter[i]--;} else {apoyadas += secuencia_[i];} // cuando counter[i]=0, el conjunto de patas correspondiente está apoyado
					}
					
					// aplicación del movimiento continuo del cuerpo
					if (rotacion_ == 0) {
						// traslación
						eventos.add ((MOVDATA) {apoyadas, xz2xyz (vector_), escala_, false}, 0);
						if (apoyadas) {
							if (mon_desplazamiento != NULL) {sumasigna (mon_desplazamiento, vector_);}   // monitor de desplazamiento
						}
					} else {
						// rotación
						for (byte pata=0; pata<6; pata++) {
							if ((apoyadas>>pata)&1) {
								COORD2D O = getOffset (pata);
								COORD2D C = resta (suma (applyMatrix (resta (suma (xyz2xz(pos_des[pata]), O), vector_), rotor1), vector_), O);
								eventos.add ((MOVDATA) {1<<pata, (COORD3D) {C.x, pos_des[pata].y, C.z}, escala_, true}, 0);  
							}
						}
						if (mon_angulo != NULL && apoyadas) {*mon_angulo -= rotacion_;}  // monitor del ángulo de rotación
					}
				 
					break;
				}
				case TRONCO: {
					for (byte index=0; index<9; index++) {
						if (oscilator[index].amp != 0) {
							if (!oscilator[index].brown) {
								param_tronco[index] = oscilator[index].amp*sin (tick*angle_step*oscilator[index].freq + oscilator[index].phase);
							} else {
								brownian_noise (&param_tronco[index], 0, oscilator[index].amp, oscilator[index].freq);     // revisar esto y la brownian noise
							}
						}
					}
					// el centro de la oscilación es pos_ref_
					// eso hace posible que la amplitud de la oscilación sea la desviación con respecto a pos_ref_
					tronco (pos_ref_, (COORD3D){param_tronco[0],param_tronco[1],param_tronco[2]}, (COORD3D){param_tronco[3],param_tronco[4],param_tronco[5]}, param_tronco[6], param_tronco[7], param_tronco[8], escala_, nsegmentos_);
					break;
				}
			}
			tick ++;
		} else {
			enable = false;
		}
	}
	 
}


/********************************************************************************************
                          MODIFICADORES ON-THE-FLY (revisar esto)
********************************************************************************************/													

void Movimiento::caminata_init () {           // inicialización del runtime de la caminata
	 
	// variables geométricas internas de la caminata (no son parámetros)              
	ciclo = periodo_sub_ciclo_*fases_/agrupamiento_;                 // duración total del ciclo (en ticks) 
	coeficiente = (ciclo-duracion_pasos_) / 2;                     // duracion de los "pasitos" (en ticks)
	if (rotacion_) {
		getRotationMatrix (rotor1, -rotacion_);                // matriz de rotación del cuerpo
		getRotationMatrix (rotor2, rotacion_*coeficiente);       // matriz de rotación de los "pasitos". 
																														   // Para hacer una verdadera "zancada progresiva" habría que recalcular esta matriz todo el tiempo
	}
 
	// variables de control de la caminata
	for (byte f=0; f<fases_; f++) {counter [f] = duracion_pasos_;}    // empieza valiendo duracion_pasos
																																			 // esto hace que durante el primer pasito no haya movimiento del tronco (al no haber patas "apoyadas")
	index %= fases_;                                          // index intenta mantenerse igual
	
	// compensación de la posición de referencia, attenti:
	if (compensate_) {
		memcpy (pos_ref, pos_des, 6*sizeof(COORD3D));             // pos_ref = posición actual
		proyeccion (pos_ref, pos_ref_);                        // proyecta la posición de referencia sobre el plano actual
		for (byte pata=0; pata<6; pata++) {
			comp [pata] = producto(resta(pos_ref_[pata], pos_ref[pata]), .5/ciclo);  // calcula el factor de corrección
		} 
	} else {
		memcpy (pos_ref, pos_ref_, 6*sizeof(COORD3D));
	}
	
	tick = periodo_pasos_*(index%agrupamiento_);  // esto equivale a inicializarlo en 0, tiene el efecto de que arranque dando un pasito
	
}

void Movimiento::caminata_init2 () {           // inicialización del runtime de la caminata "on-the-fly"
	 
	// variables geométricas internas de la caminata (no son parámetros)              
	if (rotacion_) {
		getRotationMatrix (rotor1, -rotacion_);                // matriz de rotación del cuerpo
		getRotationMatrix (rotor2, rotacion_*coeficiente);       // matriz de rotación de los "pasitos". 
	}
 
	// variables de control de la caminata
	for (byte f=0; f<fases_; f++) {counter [f] += duracion_pasos_;}    // empieza valiendo duracion_pasos
																																			 // esto hace que durante el primer pasito no haya movimiento del tronco (al no haber patas "apoyadas")
	compensate_ = false;
	
	tick %= periodo_sub_ciclo_;
	
}

// reinicia el movimiento
void Movimiento::start () {
	switch (mode) {
		case CAMINATA:
			caminata_init ();
			break;
		case TRONCO:
			tick = 0;         // la tronco_init () se reduce a esto
			break;
	}
	
	// common
	pausa = 1000*TICK*escala_;
	enable = true;
}

// detiene cualquier movimiento
void Movimiento::stop () {
	enable = false;
}

// prosigue el movimiento
void Movimiento::cont () {
	enable = true;
}

// aumenta la velocidad en movimiento
byte Movimiento::dec_escala () {
	if (escala_ > 1) {
		escala_ --;
		pausa = 1000*TICK*escala_;
		nsegmentos_ = nseg (duracion_pasos_, escala_, 10);  // largo_pasos no se conoce, así que se estima en 10
	}
	return escala_;
}

// reduce la velocidad en movimiento
byte Movimiento::inc_escala () {
	if (escala_ < 255) {
		escala_ ++;
		pausa = 1000*TICK*escala_;
		nsegmentos_ = nseg (duracion_pasos_, escala_, 10);  // largo_pasos no se conoce, así que se estima en 10
	}
	return escala_;
}

byte Movimiento::nseg (int duracion) {
	// duracion en TICKS
	// esta fórmula no tiene en cuenta la distancia
	return constrain (duracion/TPS, 2, 5);           // limitado a 5 porque son movimientos cortos
}

byte Movimiento::nseg (int duracion_pasos, byte escala, float largo_pasos) {
	return constrain (duracion_pasos*escala*(largo_pasos/10)/TPS, 2, 10);
}

/*======================================================================================================
                             ALGUNAS RUTINAS QUE ME GUSTARÍA REFLOTAR
======================================================================================================*/ 

/*
byte mov_paseo (byte modo, int centro, int amplitud) {                       // genera una "caminata" al azar; devuelve evento
                                                                             // centro y amplitud controlan la evolucion aleatoria de la velocidad en el modo 0
  
  byte static direccion=2, pasos;                
  float static velocidad = 220; 
  int static pausa=0;
  
  /* aca vendrían una serie de reglas que determinen
     el comportamiento, es decir, qué caminata y a qué velocidad
     puede suceder a otra, etc., etc. * /
  
  byte dir_ant = direccion;
  
  switch (direccion) {
    case 0:
      direccion = markov (4,5,1,3,3);
      break;
    case 1:
      direccion = markov (4,1,0,3,3);
      break;
    case 2:
      direccion = markov (4,6,2,3,1);
      break;
    case 3:
      direccion = markov (4,6,2,1,3);
      break;
  }   
  
  bool cambio = (direccion != dir_ant);
  
  /* nota: velocidad alta + pausa grande = movimiento "militar" 
           velocidad baja + pausa chica = movimiento "resbaloso"
           restantes combinaciones = movimiento normal           * /
  
  byte evento;
  switch (modo) {               // optimizar este switch, hay mucho código repetido
    case 0:                                                                                                            // curved noise: este es el + usado
      curved_noise (&velocidad, centro, amplitud, 3.1-velocidad/280-cambio/1.6);                                       // centro standard=370, amplitud standard=280 
      pasos = (random (4,6) + map(velocidad, 90, 650, 0, random (2,3)) + cambio*random(2,3)) / ((direccion!=0)+1); 
      if (cambio || pasos>5) {pausa = 0;} else {pausa = random (20, 40);}                                              // (revisar esto)
  //    if (umbral_snd==0) {umbral_snd = map (velocidad, 0, 650, 15, 33);}                                               // umbral sonido automático
      evento = caminata (direccion, velocidad, pausa, pasos);  
      break;
    case 1:                                                                                                            // brownian noise
      brownian_noise (&velocidad, 370, 230); 
      pasos = (random (4,6) + map(velocidad, 90, 650, 0, random (2,3)) + cambio*random(2,3)) / ((direccion!=0)+1); 
      if (cambio || pasos>5) {pausa = 0;} else {pausa = random (20, 40);}                                              // (revisar esto)
  //    if (umbral_snd==0) {umbral_snd = map (velocidad, 0, 650, 15, 33);}
      evento = caminata (direccion, velocidad, pausa, pasos);  
      break;
    case 2:                                                                                                            // todo random (revisar todo)
      direccion = random (4);
      pasos = random (2, 7); 
      velocidad = random (90, 650);
      if (random(1)<.8) {pausa = 0;} else {pausa = random (10, 50);}
      if (random(1)>.87) {evento = caminata3 (direccion&1, pasos);} else {evento = caminata (direccion, velocidad, pausa, pasos);}
      break;
  }
      
  return evento;
    
}

  /* 
  // ejemplo:
  shape = random (8);
  source = random (9, 14);
  execute (0,0,0,400);
  */

	/*
void execute (byte tiempo, byte tiempo_sup, byte nivel, byte pausa) {
  
  float static altura=-5.2, cercania, deltaAID, deltaHID, deltaCID, deltaHAA;
  
  byte prob [4][6] = {{1, 2, 0, 3, 4, 5}, {3, 0, 1, 5, 2, 4}, {2, 1, 3, 0, 4, 5}, {3, 5, 4, 2, 1, 0}};
  
  byte nota = prob [2*tiempo_sup + tiempo] [byte (6*log_random (5))];   // curva = estabilidad
  
  if (nivel < 2) {nota=5;}   // límite inferior de recursión
  if (nivel > 4) {nota=0;}   // límite superior de recursión
  
  if (nota<5) {
    active_delay (pausa/pow(2, nivel));
    switch (nota) {
      case 0: break;  // "silencio"
      case 1: curved_noise (&cercania, 0, 1.4, .1); break;
      case 2: curved_noise (&altura, -5.2, 1.5, .3); break;
      case 3: curved_noise (&deltaAID, 0, 1.2, 2); break;
      case 4: 
        curved_noise (&deltaHID, 0, .9, 1.5);
        curved_noise (&deltaCID, 0, .9, 1.5);
        curved_noise (&deltaHAA, 0, .9, 1.5);
        break;
    }
    postura (2.8, 0, altura, cercania, deltaAID, deltaHID, deltaCID, deltaHAA, 530);
  } else {execute (0, tiempo, nivel+1, pausa); execute (1, tiempo, nivel+1, pausa);}        

}  

*/