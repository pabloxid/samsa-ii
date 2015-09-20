// conducta2.cpp created for project v8.3 on 09/03/2011 05:05:55

/****************************************************/
/*   implementación de las conductas individuales   */
/****************************************************/

#include "conducta.h"
#include "util.h"
#include "mov_bajo_nivel.h"
#include "cabeza.h"
#include "hardware.h"
#include "display.h"

void Empujones::evaluar () {  
	
	// reducir falsos positivos
	// corregir asimetría
	// habilitar balance del cuerpo
	
	// verifica si se está moviendo; en caso afirmativo, acción "slow" (esto podria ser otra conducta)
	if (estado.tipo == WALK) {
		accion.tipo = SLOW; 
		return;
	}
	
	// en caso contrario, excepto si está en IDLE, no hace nada	
	if (estado.tipo != IDLE) {
		accion.tipo = IDLE;
		return;
	}
		
	// comprobar empuje
	float salida [3], entrada [18];
	for (byte pata=0; pata<6; pata++) {
		for (byte anillo=0; anillo<3; anillo++) {
			entrada[pata*3+anillo] = 0.05 * load[pata][anillo];    // esto está raro
		}
	}
	process->compute (entrada, salida);  // invoca a la red neuronal
	float x = 200*(salida[2]-.5);
	float z = 200*(salida[1]-.5);
	float modulo = hypot (x, z);
	float angulo = atan2 (-z, x);
	
	// acción
	if (modulo > 76) {                    
		accion.tipo = WALK;        
		accion.param1 = angulo * 10;     // param1 = angulo x 10
		accion.param2 = 1.2*(modulo-71);   // param2 = velocidad   
	} else {
		accion.tipo = IDLE;        
	}

}

void ReactLoad::evaluar () {  // también tiene que evaluar si una pata tiene poco load
	
	static byte count = 0;
	static char last_pata = -1;
	
	if (estado.tipo != IDLE) {
		accion.tipo = IDLE;
		return;
	}
  
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
		
	if (load_pata[max] > 3950) {         // corregir: número mágico
		accion.tipo = LEG;
		if (last_pata == max) {count ++;} else {count = 1;}
		if (count < 3) {accion.param1 = max;} else {accion.param1 = min;}
		last_pata = accion.param1;
	} else {
		accion.tipo = IDLE;
		last_pata = -1;        // significa: ninguna
	} 

}

// idea: cada conducta administra los recursos que debe usar, ej. apaga los pollings cuando no se usan --> hecho

void Correa::enable () {
	enabled = true;
	attachInterrupt (1, sensor_cuerdita, RISING);      // define la interrupción del sensor de cuerdita (ATtiny85)
	bitSet (timer0_int_flag, 3);                   // prende el polling analógico (sensor de cuerdita). Otra manera de hacerlo: timer0_int_flag |= _BV(3);
}

void Correa::disable () {
	enabled = false;
	detachInterrupt (1);                   // borra la interrupción del sensor de cuerdita (ATtiny85)
	bitClear (timer0_int_flag, 3);           // apaga el polling analógico (sensor de cuerdita). Otra manera de hacerlo: timer0_int_flag &= ~(_BV(3));
	accion.tipo = IDLE;                   // esto es necesario
}

// acá hay mucho que arreglar:
// habilitar movimiento continuo
// habilitar curvas
// probablemente hay que sofisticar el sistema de acciones en lo que tiene que ver con caminata

void Correa::evaluar () {

	accion.tipo = IDLE;
		
	// el propósito de cuerdita_flag es evitar tener que comprobar cada vez si el estado del robot
	// coincide con el de la cuerdita: si la cuerdita no se movió, no hacemos nada.
	if (cuerdita_flag < 128) {return;}
	
	cuerdita_flag  &= 127;
	bool inc_vel = (cuerdita_flag == 2);
	bool dec_vel = (cuerdita_flag == 3);
	
	accion.param2 = 6*log(sns_fuerza)-14;           // velocidad
	accion.param1 = 10*(-HALF_PI - sns_angulo*PI/512);    // ángulo 
		
	switch (estado.tipo) {
		case IDLE:
			if (inc_vel && sns_fuerza > 50) {
				accion.tipo = WALK;
			}
			break;
		case WALK:
			if (dec_vel) {
				//accion.tipo = SLOW;
			}
			break;
		default:
			break;
	}
	
	if (sns_fuerza > 1010) {accion.tipo = STOP;}
	
	cuerdita_flag = 0;

}

void ReactDistance::enable () {
	enabled = true;
	// bitSet (timer0_int_flag, 2);   
	threshMode (); 
}

void ReactDistance::disable () {
	enabled = false;
	// bitClear (timer0_int_flag, 2);    // Atención: el polling serial de la kbza no se puede apagar porque de él depende el control remoto
	kbza.disable_send_all ();         // deshabilita la lectura automática de sensores
	// deshabilita todos los umbrales
	noumbrales ();       
	accion.tipo = IDLE;             // esto es necesario
}

void ReactDistance::threshMode () {
	kbza.disable_send_all ();      // deshabilita la lectura automática de sensores
	umbrales ();                // habilita los umbrales 
	// kbza.flag = 0;
}
	
void ReactDistance::autoMode () {
	noumbrales ();                 // deshabilita los umbrales
	kbza.enable_send_all ();         // habilita la lectura automática de sensores
}

void ReactDistance::umbrales () {
	kbza.set_threshold (VEL, 2);        // habilita el umbral de velocidad, lo que equivale a notificar cada vez que hay un cambio de distancia
	kbza.set_threshold (MM_DIST, 280);   // umbral de distancias en mm cuando es menor a 28cm
	kbza.set_threshold (EVNT_VEL, 3);    // evento de velocidad
	kbza.set_threshold (EVNT_LAT, 3);    // evento lateral
	kbza.set_threshold (LAT, 1);        // umbral lateral
	// el único umbral que no se define es el de CM_DIST, que se opta por leerlo a manualmente
}

void ReactDistance::noumbrales () {
	kbza.set_threshold (VEL, 127);        
	kbza.set_threshold (MM_DIST, 0);      
	kbza.set_threshold (EVNT_VEL, 127);    
	kbza.set_threshold (EVNT_LAT, 127);    
	kbza.set_threshold (LAT, 127);
}

// problemas:
// 1) latencia
// 2) geometría
// 3) linealidad del sensor

void ReactDistance::evaluar () {
	// ATENCION: si se usa el send all, hay una probabilidad de falla debida al running_status
	// para reducirla, hay que requestar de vez en cuando otra cosa
	// requesta la distancia en cm, todo lo otro se activa por umbrales
	kbza.request (CM_DIST);
	accion.tipo = IDLE;
	
	if (kbza.flag & 2) {              // si hubo un cambio de distancia
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
	
}