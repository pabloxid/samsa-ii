// movimiento2.cpp created for project v8.2 on 08/30/2011 07:22:11

/*        ******************************
					* mov. alto nivel - caminata *
					******************************          */
					
// revisar toda la parte de inits, el resto está bastante bien	
// la fórmula del pause se repite muchas veces
// todos los vectores tienen un componente 'y' calculado a partir de la ecuación del plano


#include "movimiento.h"
#include "ax12.h"                // por la bin2sign()
#include <string.h>
#include "mov_bajo_nivel.h"      // para acceder a 'pos_des[]'
#include "events.h"
					
///////////////////////////////////////// CAMINATAS /////////////////////////////////////////

// recta común
void Movimiento::recta (float velocidad, float desplazamiento, float angulo, byte marcha, float largo_pasos) {
  caminata (velocidad, desplazamiento, false, (COORD2D){0,0}, angulo, marcha, largo_pasos);
}
		
// curva común
void Movimiento::curva (float velocidad, float desplazamiento, COORD2D centro, bool sentido, byte marcha, float largo_pasos) {
  caminata (velocidad, desplazamiento, true, centro, bin2sign(sentido), marcha, largo_pasos);
}

// caminata con parámetros "inteligentes"
void Movimiento::caminata (float velocidad, float desplazamiento, bool curva, COORD2D centro, float angulo, byte marcha, float largo_pasos) {
	set_values (velocidad, desplazamiento, curva, centro, angulo, marcha, largo_pasos);
	caminata (true);                                                           // revisar esta lógica
}

///////////////////////////////////////// MISC. & PRIVATE /////////////////////////////////////////

// convierte parámetros "inteligentes" en parámetros "RAW" 
void Movimiento::set_values (float velocidad, float desplazamiento, bool curva, COORD2D centro, float angulo, byte marcha, float largo_pasos) {
  // si curva == false -> caminata recta con dirección [angulo]
  // si curva == true  -> caminata curva con centro [centro] y sentido = signo de [angulo]
  // todos los parámetros son en centímetros y en segundos.
  // marcha admite los valores 1, 2 y 3 (es el numero de patas simultáneas)  
  // desplazamiento < 0 -> movimiento infinito, detenerlo con stop();
  
  if (largo_pasos == 0) {
		// esta fórmula para calcular automáticamente el largo de los pasos, contempla la distancia entre las patas
		// pero igual da cualquier fruta, hay que corregirla
		largo_pasos = sigmoide (velocidad/7) * .335 * distancia (rel2abs (xyz2xz(sh.mv.pos_ref[0]), 0), rel2abs (xyz2xz(sh.mv.pos_ref[2]), 2)); 
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
  
  // 5) vamos a calcular: modulo_vector, ticks, altura_pasito, nsegmentos
  float modulo_vector = velocidad * TICK * escala;
  unsigned int ticks;
  if (desplazamiento >= 0) {ticks = desplazamiento / modulo_vector + duracion_pasos;} else {ticks = -1;}  
  /* a esta altura podemos hacer la siguiente operación comprobatoria: 
	    periodo_sub_ciclo = (largo_pasos/modulo_vector + duracion_pasos)*agrupamiento/fases */
  byte nsegmentos =  nseg (duracion_pasos, escala, largo_pasos);
  // fórmula para la altura del punto manejador de la curva bezier
  // contempla el largo y la duración de los pasos, pero igual da fruta
  float altura_pasito = 7.7 + 25*(8.4 + 2.6*sqrt(largo_pasos))/(duracion_pasos*escala);      // ajustar esto es casi imposible
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
      float d = distancia (centro, rel2abs (xyz2xz(sh.mv.pos_ref[pata]), pata));
      if (dist < d) {dist = d;}
    } 
    rotacion = sign (angulo) * modulo_vector / dist;       // aproximación basada en el arco; también podría usarse la cuerda
  }

  /* parámetros "RAW"
   secuencia                      // orden de patas, por ejemplo {1, 2, 4, 8, 16, 32}, etc.  
   fases                          // número de elementos de la secuencia
   agrupamiento                   // cantidad de fases tomadas como una subunidad (puede ser 1, 2 o 3; para desactivar: agrupamiento=1, periodo_sub_ciclo=periodo_pasos)
   escala                         // cantidad de TICKs (ticks del subsistema de movimiento) por tick (tick de la caminata, micropaso). Es inversamente proporcional a la velocidad.
   periodo_sub_ciclo              // cantidad de ticks de una fase o un agrupamiento de fases 
   periodo_pasos                  // inverso de la frecuencia de los pasos  
   duracion_pasos                 // si es == periodo_pasos, significa no aire entre las fases... si es > periodo_pasos hay "solapamiento" 
   altura_pasito                  // altura de las curvas bezier (pasitos) 
   nsegmentos                     // resolución de las curvas bezier (idealmente: largo_pasos*escala/nsegmentos != escala, para distribuir mejor los eventos AX12)  
	 compensate                     // boolean que indica si se debe corregir progresivamente la posición de referencia    */
	 
		wk.mv.vector = vector;         // en la recta esto es el vector, en la curva el centro (el vector es el desplazamiento por micropaso)
		wk.mv.rotacion = rotacion;      // si vale 0 es recta, de lo contrario es el arco de la curva
		sh.mv.ticks = ticks;
		memcpy (wk.mv.secuencia, secuencia, fases);
		wk.mv.fases = fases;
		wk.mv.agrupamiento = agrupamiento;
		sh.mv.escala = escala;
		wk.mv.periodo_sub_ciclo = periodo_sub_ciclo;
		wk.mv.periodo_pasos = periodo_pasos;
		wk.mv.duracion_pasos = duracion_pasos;
		wk.mv.altura_pasito = altura_pasito;
		sh.mv.nsegmentos = nsegmentos;
		
}

// ejecución de la caminata
void Movimiento::caminata (bool compensate) {  
	
	// variables de control
	wk.mv.compensate = compensate;                     // revisar esta lógica (compensate deberia ser una variable de control, por ejemplo)
	if (!sh.cn.enable || sh.cn.mode!=CAMINATA) { 
		sh.cn.mode = CAMINATA;
		start ();
	} else {
		caminata_init2 ();
	}
	
}

// inicialización del runtime de la caminata
void Movimiento::caminata_init () {           
	 
	// variables geométricas internas de la caminata (no son parámetros)              
	wk.rt.ciclo = wk.mv.periodo_sub_ciclo*wk.mv.fases/wk.mv.agrupamiento;                 // duración total del ciclo (en ticks) 
	wk.rt.coeficiente = (wk.rt.ciclo-wk.mv.duracion_pasos) / 2;                     // duracion de los "pasitos" (en ticks)
	
	if (wk.mv.rotacion) {
		getRotationMatrix (wk.rt.rotor1, -wk.mv.rotacion);                    // matriz de rotación del cuerpo
		getRotationMatrix (wk.rt.rotor2, wk.mv.rotacion*wk.rt.coeficiente);     // matriz de rotación de los "pasitos". 
																																					   // Para hacer una verdadera "zancada progresiva" habría que recalcular esta matriz todo el tiempo
	}
 
	// variables de control de la caminata
	for (byte f=0; f<wk.mv.fases; f++) {wk.rt.counter [f] = wk.mv.duracion_pasos;}    // empieza valiendo duracion_pasos
																								  											            // esto hace que durante el primer pasito no haya movimiento del tronco (al no haber patas "apoyadas")
	wk.rt.index %= wk.mv.fases;                                            // index intenta mantenerse igual
	
	// compensación de la posición de referencia, attenti:
	if (wk.mv.compensate) {
		memcpy (wk.rt.pos_ref, pos_des, 6*sizeof(COORD3D));                   // pos_ref = posición actual
		proyeccion (wk.rt.pos_ref, sh.mv.pos_ref);                         // proyecta la posición de referencia sobre el plano actual
		for (byte pata=0; pata<6; pata++) {
			wk.rt.comp [pata] = producto(resta(sh.mv.pos_ref[pata], wk.rt.pos_ref[pata]), .5/wk.rt.ciclo);  // calcula el factor de corrección
		} 
	} else {
		memcpy (wk.rt.pos_ref, sh.mv.pos_ref, 6*sizeof(COORD3D));
	}
	
	sh.cn.tick = wk.mv.periodo_pasos*(wk.rt.index%wk.mv.agrupamiento);   // esto equivale a inicializarlo en 0, tiene el efecto de que arranque dando un pasito
	
}

// inicialización del runtime de la caminata "on-the-fly"
void Movimiento::caminata_init2 () { 
	 
	// variables geométricas internas de la caminata (no son parámetros)              
	if (wk.mv.rotacion) {
		getRotationMatrix (wk.rt.rotor1, -wk.mv.rotacion);                     // matriz de rotación del cuerpo
		getRotationMatrix (wk.rt.rotor2, wk.mv.rotacion*wk.rt.coeficiente);       // matriz de rotación de los "pasitos". 
	}
 
	// variables de control de la caminata
	for (byte f=0; f<wk.mv.fases; f++) {wk.rt.counter [f] += wk.mv.duracion_pasos;}     // empieza valiendo duracion_pasos
																																			               // esto hace que durante el primer pasito no haya movimiento del tronco (al no haber patas "apoyadas")
	wk.mv.compensate = false;
	sh.cn.tick %= wk.mv.periodo_sub_ciclo;
	
	// esto es de la init normal
	wk.rt.ciclo = wk.mv.periodo_sub_ciclo*wk.mv.fases/wk.mv.agrupamiento;              // duración total del ciclo (en ticks) 
	wk.rt.coeficiente = (wk.rt.ciclo-wk.mv.duracion_pasos) / 2;                     // duracion de los "pasitos" (en ticks)
	wk.rt.index %= wk.mv.fases;
	sh.cn.pausa = 1000*TICK*sh.mv.escala;
	
}

/********************************************************************************************
                          MODIFICADORES ON-THE-FLY (revisar esto)
********************************************************************************************/													

void Movimiento::set_vel (float velocidad) {
		
	// re-calcular los parámetros
	if (wk.mv.rotacion != 0) {return;}                                              // por ahora sólo para caminatas rectas
	float modulo_vector = hypot (wk.mv.vector.x, wk.mv.vector.z);
	float desplazamiento = (sh.mv.ticks - sh.cn.tick - wk.mv.duracion_pasos) * modulo_vector; 
	float angulo = atan2 (-wk.mv.vector.z, -wk.mv.vector.x);
	float largo_pasos = modulo_vector * 2 * wk.rt.coeficiente;
	
	set_values (velocidad, desplazamiento, false, (COORD2D){0,0}, angulo, 1, largo_pasos); 
	caminata_init2 ();
		
}

// reinicia el movimiento
void Movimiento::start () {
	switch (sh.cn.mode) {
		case CAMINATA:
			caminata_init ();
			break;
		case TRONCO:
			sh.cn.tick = 0;         // la tronco_init () se reduce a esto
			break;
	}
	
	// common
	sh.cn.pausa = 1000*TICK*sh.mv.escala;
	sh.cn.enable = true;
}

// detiene cualquier movimiento
void Movimiento::stop () {
	sh.cn.enable = false;
}

// prosigue el movimiento
void Movimiento::cont () {
	sh.cn.enable = true;
}

// aumenta la velocidad en movimiento
byte Movimiento::dec_escala () {
	if (sh.mv.escala > 1) {
		sh.mv.escala --;
		sh.cn.pausa = 1000*TICK*sh.mv.escala;
		sh.mv.nsegmentos = nseg (wk.mv.duracion_pasos, sh.mv.escala, 10);  // largo_pasos no se conoce, así que se estima en 10
	}
	return sh.mv.escala;
}

// reduce la velocidad en movimiento
byte Movimiento::inc_escala () {
	if (sh.mv.escala < 255) {
		sh.mv.escala ++;
		sh.cn.pausa = 1000*TICK*sh.mv.escala;
		sh.mv.nsegmentos = nseg (wk.mv.duracion_pasos, sh.mv.escala, 10);  // largo_pasos no se conoce, así que se estima en 10
	}
	return sh.mv.escala;
}

// devuelve el número de segmentos a aplicar en la curva bezier 1
byte Movimiento::nseg (int duracion) {
	// duracion en TICKS
	// esta fórmula no tiene en cuenta la distancia
	return constrain (duracion/TPS, 2, 5);           // limitado a 5 porque son movimientos cortos
}

// devuelve el número de segmentos a aplicar en la curva bezier 2
byte Movimiento::nseg (int duracion_pasos, byte escala, float largo_pasos) {
	return constrain (duracion_pasos*escala*(largo_pasos/10)/TPS, 2, 10);       // esta fórmula sí tiene en cuenta la distancia
}

/*********************************************************************************************************
 *       *       *       *       *        *       RUNTIME       *        *       *       *       *       *
 *********************************************************************************************************/

// idea: tanto para la "modulación" como para arrancar caminatas desde cualquier 
// posición y que lentamente se vaya acomodando, la clave es: transformar sh.mv.pos_ref

void Movimiento::update (unsigned long milis) {          // esto es un kilombo. Solución: la "update" pasaría a ser una clase nueva
  
  // única variable de control exclusiva de la "update"
  static unsigned long timer = 0;                         // timer
  
  // verifica si pasó x tiempo desde la última vez que se ejecutó el micropaso, y si no, retorna sin hacer nada 
  if (milis-timer < sh.cn.pausa) {return;} 
  timer = milis;
  
	// empieza el loop (bloque activo)
	if (sh.cn.enable) {
		if (sh.cn.tick < sh.mv.ticks) {
			switch (sh.cn.mode) {
				case CAMINATA: {
					
					// compensación de posición de referencia
					if (wk.mv.compensate && sh.cn.tick < 2*wk.rt.ciclo) {
						for (byte pata=0; pata<6; pata++) {
							sumasigna (&wk.rt.pos_ref[pata], wk.rt.comp[pata]);
						}
					}
					
					// tratamiento de los "pasitos"
					if ((sh.cn.tick%wk.mv.periodo_sub_ciclo)-wk.mv.periodo_pasos*(wk.rt.index%wk.mv.agrupamiento) == 0) {
									
						for (byte pata=0; pata<6; pata++) {
							if ((wk.mv.secuencia[wk.rt.index]>>pata)&1) {
								COORD2D C;
								float atenuador; 
								
								// "zancada progresiva"
								if (sh.cn.tick <= 2*wk.rt.ciclo) {atenuador = 1.6 * sigmoide(.5*sh.cn.tick/wk.rt.ciclo) - .6;} else {atenuador = 1;}
								
								if (wk.mv.rotacion == 0) {
									// traslación
									C = suma(xyz2xz (wk.rt.pos_ref[pata]), producto(wk.mv.vector, -wk.rt.coeficiente*atenuador)); 
									if (sh.cn.tick > 2*wk.rt.ciclo) {
										C = resta (C, xyz2xz(pos_des[pata]));
										// si las condiciones están dadas, agenda un solo pasito relativo para todas las patas y sale
										pasito (wk.mv.secuencia[wk.rt.index], xz2xyz(C), false, 0, sh.mv.escala*wk.mv.duracion_pasos, sh.mv.nsegmentos, suma(xz2xyz(producto(C, .5)), (COORD3D){0,wk.mv.altura_pasito,0}));
										break;
									}
								} else {
									// rotación
									COORD2D O = getOffset (pata); // no usamos rel2abs/abs2rel por razones de eficiencia
									C = resta (suma (applyMatrix (resta (suma (xyz2xz(wk.rt.pos_ref[pata]), O), wk.mv.vector), wk.rt.rotor2), wk.mv.vector), O);   
									// en la rotación, hacer la "zancada progresiva" es un poco más complicado..
									// como no podemos recalcular la matriz de rotación, hacemos interpolación lineal
									if (sh.cn.tick <= 2*wk.rt.ciclo) {
										C = suma (producto (resta (C, xyz2xz(wk.rt.pos_ref[pata])), atenuador), xyz2xz(wk.rt.pos_ref[pata]));
									}
								} 
								
								// pasito
								pasito (1<<pata, (COORD3D) {C.x, wk.rt.pos_ref[pata].y, C.z}, true, 0, sh.mv.escala*wk.mv.duracion_pasos, sh.mv.nsegmentos, suma(wk.rt.pos_ref[pata], (COORD3D){0,wk.mv.altura_pasito,0}));
							}
						}
				
						wk.rt.counter[wk.rt.index] = wk.mv.duracion_pasos;
						wk.rt.index = (wk.rt.index+1)%wk.mv.fases;
					}
					
					// determinación de las patas apoyadas
					wk.rt.apoyadas = 0;
					for (byte i=0; i<wk.mv.fases; i++) {
						if (wk.rt.counter[i] > 0) {wk.rt.counter[i]--;} else {wk.rt.apoyadas += wk.mv.secuencia[i];} // cuando counter[i]=0, el conjunto de patas correspondiente está apoyado
					}
					
					// aplicación del movimiento continuo del cuerpo (micropasos)
					if (wk.mv.rotacion == 0) {
						// traslación
						eventos.add ((MOVDATA) {wk.rt.apoyadas, xz2xyz (wk.mv.vector), sh.mv.escala, false}, 0);
						if (wk.rt.apoyadas) {
							if (mon_desplazamiento != NULL) {sumasigna (mon_desplazamiento, wk.mv.vector);}   // monitor de desplazamiento
						}
					} else {
						// rotación
						for (byte pata=0; pata<6; pata++) {
							if ((wk.rt.apoyadas>>pata)&1) {
								COORD2D O = getOffset (pata); // no usamos rel2abs/abs2rel por razones de eficiencia
								COORD2D C = resta (suma (applyMatrix (resta (suma (xyz2xz(pos_des[pata]), O), wk.mv.vector), wk.rt.rotor1), wk.mv.vector), O);
								eventos.add ((MOVDATA) {1<<pata, (COORD3D) {C.x, pos_des[pata].y, C.z}, sh.mv.escala, true}, 0);  
							}
						}
						if (mon_angulo != NULL && wk.rt.apoyadas) {*mon_angulo -= wk.mv.rotacion;}  // monitor del ángulo de rotación
					}
				 
					break;
				}
				case CAMINATA2: {
					
					// no me acuerdo qué se suponía que iba a ir acá
					
					break;
				}
				case TRONCO: {
					for (byte index=0; index<9; index++) {
						if (bd.oscilator[index].amp != 0) {
							if (!bd.oscilator[index].brown) {
								bd.param_tronco[index] = bd.oscilator[index].amp*sin (sh.cn.tick*angle_step*bd.oscilator[index].freq + bd.oscilator[index].phase);
							} else {
								brownian_noise (&bd.param_tronco[index], 0, bd.oscilator[index].amp, bd.oscilator[index].freq);     // revisar esto y la brownian noise
							}
						}
					}
					// el centro de la oscilación es sh.mv.pos_ref
					// eso hace posible que la amplitud de la oscilación sea la desviación con respecto a sh.mv.pos_ref
					tronco (sh.mv.pos_ref, (COORD3D){bd.param_tronco[0],bd.param_tronco[1],bd.param_tronco[2]}, (COORD3D){bd.param_tronco[3],bd.param_tronco[4],bd.param_tronco[5]}, bd.param_tronco[6], bd.param_tronco[7], bd.param_tronco[8], sh.mv.escala, sh.mv.nsegmentos);
					break;
				}
			}
			sh.cn.tick ++;
		} else {
			sh.cn.enable = false;
		}
	}
	 
}