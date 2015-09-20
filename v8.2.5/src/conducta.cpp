// Conducta.cpp created for project v8.2 on 08/16/2011 04:23:11

/////////////////////////////////////////////////////////////////////////////////
//                                conductas                                    //
/////////////////////////////////////////////////////////////////////////////////

/* todo esto es una prueba 
   hay que implementar mejor lo de las acciones
	 acciones con parámetros
	 una clase monitor?                                        */ 	 
	 
#include "conducta.h"
#include "movimiento.h"
#include "hardware.h"
#include "display.h"
#include "vectores.h"
#include "events.h"
#include "cabeza.h"


/////////////////////////////////////////////////////////////////////////////////
//                             clase Conducta                                  //
/////////////////////////////////////////////////////////////////////////////////

ACTION Conducta::estado;

Conducta::Conducta (byte in_prioridad) {
	prioridad = in_prioridad;
	enabled = false;                         // todas las conductas arrancan apagadas, y sus respectivos recursos también
	accion.tipo = IDLE;
}

void Conducta::enable () {
	enabled = true;
}

void Conducta::disable () {
	enabled = false;
	accion.tipo = IDLE;    // esto es necesario
}

void Conducta::ejecutar (ACTION accion) {   // ejecuta la accion, y actualiza el estado
	 
	static byte last_type = IDLE;
	static unsigned int counter = 0;
	
	if (accion.tipo == last_type) {
		counter ++;
	} else {
		counter = 0;
	}
	
	// idea: que esto sea sólo un despachador, y las acciones estén en otro lado
	// observación: las acciones pueden ser "estados".. hay que ver si "estado = accion;" no va en todos los casos
	// puede haber una rutina "normalizadora" que se encargue de llevar al bicho a la posición por defecto
	
	switch (accion.tipo) {
		case IDLE:
			break;
		case STOP: // este caso le falta el "estado = accion". investigar por qué
			mov.stop();
			break;
		case WALK:
			// el ángulo es en radianes x 10
			mov.recta (accion.param2, 2*accion.param2, 0.1*accion.param1);    
			estado = accion;
			break;
		case SLOW:       // este es el unico caso en que no pone "estado = accion". investigar por qué.
			if (counter >= 100) {   // harcodeado a lo bestia. Esto hay que pensarlo
				accion.param2 --;
				mov.set_vel (accion.param2);
				counter = 0;
			}
			break;
		case AFRAID: {
			COORD3D vector = suma ((COORD3D){accion.param1/7.0,-1.8,-2.3}, producto ((COORD3D){0,-1.6,-2}, accion.param2));     // la pos_ref tiene que ser una congelación de pos_des
			mov.tronco (mov.get_pos_ref(), vector, (COORD3D){0,0,0}, 0, 0, 0, 20);
			estado = accion;
			break;
		}
		case BODY:
			break;
		case REST: 
			break;
		case LEG: {
			// obtengo la ecuación del plano actual del robot:
			COORD3D normal; 
			float d;
			ec_plano (pos_des, &normal, &d);    
			// obtengo un punto en 2D cercano a pos_ref_ (en realidad es un vector para sumarle a la posición actual de la pata)
			COORD3D P = suma ((COORD3D){random(-2.5,2.5),0,random(-2.6,2.6)}, producto(resta(mov.get_pos_ref()[accion.param1],pos_des[accion.param1]), .38)); // magik numbers 
			// calculo la coordenada 'y' del vector en cuestión, utilizando la ecuación del plano actual
			P.y = -(P.x*normal.x + P.z*normal.z) / normal.y;    // nótese que 'd' no se usa para nada
			// aplico el movimiento
			mov.pasito (1<<accion.param1, P, false, 0, 85, 5, (COORD3D){0,10,0}); 
			estado = accion;
			break;
		}
		case ATTACK:
			// el ángulo es en radianes x 10	
			mov.salto (accion.param2, 0.1*accion.param1);
			estado = accion;
			break;
		case FOLLOW:
			/*reactDistance.autoMode();    // esto no me convence en absoluto
			// mide la distancia del objeto y lo "sigue con la mirada"
			byte contador = 0;
			char static incremento [2] = {2, 2}; 
			char resultado [2] = {0, 0}; 
			while (contador < 2) {                                                             // 2 = número de repeticiones (cuando no hay cambios)
				for (byte i=0; i<2; i++) {                                                       // actúa sobre el servo 0 (altura) y el 2 (giro)
					kbza.set_pos (i, 30*incremento [i], false); delay (40);                 
					kbza.set_pos (i, -60*incremento [i], false); delay (80);             // mueve desde -incremento a +incremento; el delay es para darle tiempo a q se mueva
					resultado [i] = (resultado[i] + kbza.vel)/2;                              // toma nota del acercamiento o alejamiento del objeto
					Serial.println (kbza.vel, DEC);
					//incremento [i] = 1 + 150/(50+kbza.cm_dist) + 3/(1+abs(resultado[i]));             // para compensar el error de medida por la distancia
					kbza.set_pos (i, 30*incremento [i] - resultado[i], false);                       // retorna a la nueva posición central
					delay (40);
				}                                                                                // el incremento es menor cuanto más próximo está el objeto
				if (kbza.cm_dist < 25 || abs(resultado[0]) + abs(resultado[1]) < 13) {
					contador ++;} else {contador = 0;}                                             // otro umbral importante
			}
			estado = accion;
			reactDistance.threshMode();    // esto no me convence en absoluto*/
			break;	
	}
	
	last_type = accion.tipo;
			
}

//////////////////////////////////////////////////////////////////////////////////////////
//      constructores de las clases derivadas de Conducta (o sea de las conductas)      //
// por alguna misteriosa razón, estos constructores tienen que ir acá y no en otro lado //
//////////////////////////////////////////////////////////////////////////////////////////

Empujones::Empujones (byte in_prioridad) : Conducta (in_prioridad) {
	// setea la red neuronal de 1 capa que procesa el load
	process = (Nlayer*) malloc (sizeof(Nlayer));
	*process = Nlayer (3, 18);
	process->node[0].set_weights((float[]){0, -.5, -.25, 0, -.5, -.25, 0, -.5, -.25, 0, -.5, -.25, 0, -.5, -.25, 0, -.5, -.25, 0});             // abajo
	process->node[1].set_weights((float[]){-.25, .005, .01, -.5, 0, 0, -.35, -.005, -.01, -.25, .005, .01, -.5, 0, 0, -.35, -.005, -.01, 0});     // atrás
	process->node[2].set_weights((float[]){.25, .005, .25, 0, .005, .5, -.25, .005, .25, -.25, -.005, -.25, 0, -.005, -.5, .25, -.005, -.25, 0});  // izquierda
} 

ReactLoad::ReactLoad (byte in_prioridad) : Conducta (in_prioridad) { }

Correa::Correa (byte in_prioridad) : Conducta (in_prioridad) { 
  // comunicación con el ATtiny85
	pinMode (3, INPUT);          // request int1
	pinMode (4, INPUT);          // sensor_id
	pinMode (5, INPUT);          // sign_id	
}

ReactDistance::ReactDistance (byte in_prioridad) : Conducta (in_prioridad) { } 


/////////////////////////////////////////////////////////////////////////////////
//                                 "main"                                      //
/////////////////////////////////////////////////////////////////////////////////

// declara y define conductas, con sus respectivas prioridades
Empujones empujones (50);    
ReactLoad reactLoad (60);
Correa correa (70);
ReactDistance reactDistance (80);

// un array de punteros a conductas
Conducta *conducta [NUM_CONDUCTAS] = {&empujones, &reactLoad, &correa, &reactDistance};  

void conducta_init () {     
	
	// todos los enables y disables iniciales van acá
	conducta [0]->disable ();
	conducta [1]->disable ();
	conducta [2]->enable ();
	conducta [3]->enable ();
	
}

// se agrega una variable: "modo" o "conducta a la que se le está dando bola"
// este modo determinaría lo que se muestra en el display, por ejemplo

void conducta_main () {           
	
	if (idle >= IDLE_THRESH) {
		Conducta::estado.tipo = IDLE;    // esto es muy importante porque es un dato que viene del propio robot
	}
	
	// evalua cada conducta
	for (byte f=0; f<NUM_CONDUCTAS; f++) {
		if (conducta[f]->enabled) {conducta[f]->evaluar();}           // principio de habilitación de conductas
																															      // NOTA: si la conducta no está habilitada, su acción debe ser IDLE
	}
	
	// a continuación, analiza qué hacer
	// algoritmo básico = de todas las conductas que retornan algo distinto de no_action, hacerle caso a la de mayor prioridad	
	// tareas simultáneas? porque ocupan distintos "recursos"?
	// cada clase tiene su accion, pero a la unidad ejecutora hay que pasarle una nueva accion
	byte selected = 0;
	for (byte f=1; f<NUM_CONDUCTAS; f++) {
		if (conducta[f]->accion.tipo != IDLE && conducta[f]->prioridad > conducta[selected]->prioridad) {
			selected = f;
		}
	}
	
	// y finalmente llama a la unidad ejecutora
	Conducta::ejecutar (conducta[selected]->accion);           
	
}