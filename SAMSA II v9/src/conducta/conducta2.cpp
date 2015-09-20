// conducta2.cpp created for project v8.3 on 09/03/2011 05:05:55

/****************************************************/
/*   implementación de las conductas individuales   */
/****************************************************/

// corregir todos los números mágicos por todos lados

#include "conducta.h"
#include "cabeza.h"
#include "hardware.h"
#include "movimiento.h"
#include "display.h"
#include "events.h"

bool Empujones::evaluar () {  
	
	// reducir falsos positivos
	// corregir asimetría
	// habilitar balance del cuerpo
	
	if (estado != IDLE) {return false;}       // si no está en idle, ya retorna
		
	// comprobar empuje
	float salida [3], entrada [18];
	for (byte pata=0; pata<6; pata++) {
		for (byte anillo=0; anillo<3; anillo++) {
			entrada[pata*3+anillo] = 0.05 * load[pata][anillo];    // esto está raro
		}                                             // el load cambia según el seteo del motor
	}
	process->compute (entrada, salida);  // invoca a la red neuronal
	float x = 200*(salida[2]-.5);
	float z = 200*(salida[1]-.5);
	modulo = hypot (x, z);
	angulo = atan2 (-z, x);
	
	return (modulo > 76);

}

byte Empujones::ejecutar () {
	float velocidad = 1.2*(modulo-71);	
	mov.recta (velocidad, 2*velocidad, angulo);  
	return WALK;
}

bool ReactLoad::evaluar () {  // también tiene que evaluar si una pata tiene poco load
	
	static byte count = 0;
	
	if (estado != IDLE) {return false;}       // primer filtro
  
	int load_pata [6] = {0, 0, 0, 0, 0, 0};
	byte max = 0;
	byte min = 0;
	
	for (byte pata=0; pata<6; pata++) {
		for (byte anillo=0; anillo<3; anillo++) {
			load_pata[pata] += sq(load[pata][anillo]);
		}
 		if (load_pata[pata] > load_pata[max]) {max = pata;}
		if (load_pata[pata] < load_pata[min]) {min = pata;}
	}
		
	bool entrar;
	
	if (load_pata[max] > 3950) {         // corregir: número mágico
		entrar = true;
		if (current_pata == max) {count ++;} else {count = 1;}
		if (count < 3) {current_pata = max;} else {current_pata = min;}
	} else {
		entrar = false;
		current_pata = -1;        // significa: ninguna
	} 
	
	return entrar;
}

byte ReactLoad::ejecutar () {
	// obtengo la ecuación del plano actual del robot:
	COORD3D normal; 
	float d;
	ec_plano (pos_des, &normal, &d);    
	// obtengo un punto en 2D cercano a pos_ref_ (en realidad es un vector para sumarle a la posición actual de la pata)
	COORD3D P = suma ((COORD3D){random(-2.5,2.5),0,random(-2.6,2.6)}, producto(resta(mov.get_pos_ref()[current_pata],pos_des[current_pata]), .38)); // magik numbers 
	// calculo la coordenada 'y' del vector en cuestión, utilizando la ecuación del plano actual
	P.y = -(P.x*normal.x + P.z*normal.z) / normal.y;    // nótese que 'd' no se usa para nada
	// aplico el movimiento
	mov.pasito (1<<current_pata, P, false, 0, 85, 5, (COORD3D){0,10,0}); 
	return LEG;
}

// idea: cada conducta administra los recursos que debe usar, ej. apaga los pollings cuando no se usan --> hecho

void Correa::enable () {
	enabled = true;
	attachInterrupt (1, sensor_cuerdita, RISING);      // define la interrupción del sensor de cuerdita (ATtiny85)
	bitSet (timer0_int_flag, 3);                   // prende el polling analógico (sensor de cuerdita). Otra manera de hacerlo: timer0_int_flag |= _BV(3); (el "3" tendria que tener un #define
}

void Correa::disable () {
	enabled = false;
	detachInterrupt (1);                   // borra la interrupción del sensor de cuerdita (ATtiny85)
	bitClear (timer0_int_flag, 3);           // apaga el polling analógico (sensor de cuerdita). Otra manera de hacerlo: timer0_int_flag &= ~(_BV(3));
}

// acá hay mucho que arreglar:
// habilitar movimiento continuo
// habilitar curvas
// probablemente hay que sofisticar el sistema de acciones en lo que tiene que ver con caminata

bool Correa::evaluar () {
	// el propósito de cuerdita_flag es evitar tener que comprobar cada vez si el estado del robot
	// coincide con el de la cuerdita: si la cuerdita no se movió, no hacemos nada.
	return (cuerdita_flag >= 128);
}

byte Correa::ejecutar () {
	
	cuerdita_flag  &= 127;
	bool inc_vel = (cuerdita_flag == 2);
	bool dec_vel = (cuerdita_flag == 3);
	
	float velocidad = 6*log(sns_fuerza)-14;           // velocidad
	float angulo = -HALF_PI - sns_angulo*PI/512;       // ángulo 
		
	byte resultado;
	
	switch (estado) {
		case IDLE:
			if (inc_vel && sns_fuerza > 50) {
				mov.recta (velocidad, -1, angulo);
				resultado = WALK;
			}
			break;
		case WALK:
			if (dec_vel && sns_fuerza > 50) {
				mov.set_vel (velocidad);
				resultado = WALK;
			}
			break;
		default:
			break;
	}
	
	if (sns_fuerza <= 50) {
		mov.stop();
		resultado = STOP;
	}
	
	cuerdita_flag = 0;       // resetea el flag
	
	return resultado;       // no están cubiertos todos los casos, por lo tanto el resultado puede ser indefinido
}

// problemas de las conductas asociadas al sensor inteligente de distacia:
// 1) latencia
// 2) geometría
// 3) linealidad del sensor

bool Follow::evaluar () {
	//Serial.print ("dist: ");
	//Serial.println (kbza.cm_dist, DEC);
	return (kbza.cm_dist < 60);     
}
	
/*
	if (kbza.flag & 2) {           // si hubo un cambio de distancia (en modo umbrales)   
		switch (estado.tipo) {
			case WALK:
				if (kbza.cm_dist < 20) {accion.tipo = STOP;} 
				else if (kbza.vel < 0) {accion.tipo = SLOW;}   // en caso de acercamiento
				break;
			default:
				if (kbza.flag & 32) {              // esto equivale a kbza.mm_dist < 250
					accion.tipo = AFRAID;
					accion.param2 = 5.6 - log(1+kbza.mm_dist);                               // "cercanía" 
					if (kbza.flag & 4) {accion.param1 = kbza.lat;} else {accion.param1 = 0;}       // lateral
				} else if (kbza.cm_dist < 60) {
					accion.tipo = FOLLOW;
				} else {
					if (estado.tipo == AFRAID) {
						accion.tipo = ATTACK;
						accion.param1 = 10;
						accion.param2 = HALF_PI*10;
					}
				}
				break;
		}
		kbza.flag = 0;       // importante, resetear el flag
	}
	*/


byte Follow::ejecutar () {
		
	enum {FASE1, FASE2, FASE3};
	static byte fase = FASE1;
	
	switch (fase) {
		case FASE1:
			break;
		case FASE2:
			break;
		case FASE3:
			break;
	}
	
	
	/*reactDistance.autoMode();    // esto no me convence en absoluto
			// mide la distancia del objeto y lo "sigue con la mirada" 
			byte contador = 0;
			char static incremento [2] = {2, 2}; 
			char resultado [2] = {0, 0}; 
			while (contador < 2) {                                                             // 2 = número de repeticiones (cuando no hay cambios)
				for (byte i=0; i<2; i++) {                                                       // actúa sobre el servo 0 (altura) y el 2 (giro)
					bitClear (kbza.flag, 1); 
		     kbza.set_pos (i, 30*incremento [i], false); delay (40);                 
					kbza.set_pos (i, -60*incremento [i], false); delay (80);             // mueve desde -incremento a +incremento; el delay es para darle tiempo a q se mueva
					delay (120);
					resultado [i] = (resultado[i] + kbza.vel*(kbza.flag&2))/2;                              // toma nota del acercamiento o alejamiento del objeto
					Serial.println (resultado [i], DEC);
					//incremento [i] = 1 + 150/(50+kbza.cm_dist) + 3/(1+abs(resultado[i]));             // para compensar el error de medida por la distancia
					kbza.set_pos (i, 30*incremento [i] - resultado[i], false);                       // retorna a la nueva posición central
					delay (1000);
				}                                                                                // el incremento es menor cuanto más próximo está el objeto
				if (kbza.cm_dist < 25 || abs(resultado[0]) + abs(resultado[1]) < 13) {
					contador ++;} else {contador = 0;}                                             // otro umbral importante
			}
			//estado = accion;
			//reactDistance.threshMode();    // esto no me convence en absoluto*/
	return FOLLOW;
}