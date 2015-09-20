// Conducta.cpp created for project v8.2 on 08/16/2011 04:23:11

/////////////////////////////////////////////////////////////////////////////////
//                                conductas                                    //
/////////////////////////////////////////////////////////////////////////////////

/* todo esto es una prueba 
   hay que implementar mejor lo de las acciones
	 acciones con parámetros
	 una clase monitor?                                        */ 	 
	 
#include "conducta.h"
#include "mov_bajo_nivel.h"
#include "util.h"
#include "cabeza.h"


/////////////////////////////////////////////////////////////////////////////////
//                             clase Conducta                                  //
/////////////////////////////////////////////////////////////////////////////////

byte Conducta::estado;

Conducta::Conducta (byte in_prioridad) {
	prioridad = in_prioridad;
	enabled = false;                      // todas las conductas arrancan apagadas, y sus respectivos recursos también
}

void Conducta::enable () {
	enabled = true;
}

void Conducta::disable () {
	enabled = false;
}

void Conducta::salir () {
	// por defecto no hace nada al salir
}

/*
void Conducta::ejecutar (ACTION accion) {       // ejecuta la acción, y actualiza el estado
	 
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
		
		case ATTACK:
			// el ángulo es en radianes x 10	
			mov.salto (accion.param2, 0.1*accion.param1);
			estado = accion;
			break;
		case FOLLOW:
			
			break;	
	}
	
	last_type = accion.tipo;
			
}*/

//////////////////////////////////////////////////////////////////////////////////////////////////
//          constructores de las clases derivadas de Conducta (o sea de las conductas)          //
//     por alguna misteriosa razón, estos constructores tienen que ir acá y no en otro lado     //
//////////////////////////////////////////////////////////////////////////////////////////////////

Empujones::Empujones (byte in_prioridad) : Conducta (in_prioridad) {
	// setea la red neuronal de 1 capa que procesa el load
	process = (Nlayer*) malloc (sizeof(Nlayer));
	*process = Nlayer (3, 18);
	process->node[0].set_weights((float[]){0, -.5, -.25, 0, -.5, -.25, 0, -.5, -.25, 0, -.5, -.25, 0, -.5, -.25, 0, -.5, -.25, 0});             // abajo
	process->node[1].set_weights((float[]){-.25, .005, .01, -.5, 0, 0, -.35, -.005, -.01, -.25, .005, .01, -.5, 0, 0, -.35, -.005, -.01, 0});     // atrás
	process->node[2].set_weights((float[]){.25, .005, .25, 0, .005, .5, -.25, .005, .25, -.25, -.005, -.25, 0, -.005, -.5, .25, -.005, -.25, 0});  // izquierda
} 

ReactLoad::ReactLoad (byte in_prioridad) : Conducta (in_prioridad) {
	current_pata = -1;
}

Correa::Correa (byte in_prioridad) : Conducta (in_prioridad) { 
  // comunicación con el ATtiny85
	pinMode (3, INPUT);          // request int1
	pinMode (4, INPUT);          // sensor_id
	pinMode (5, INPUT);          // sign_id	
}

Follow::Follow (byte in_prioridad) : Conducta (in_prioridad) { } 


/////////////////////////////////////////////////////////////////////////////////////////////
//                                       "main"                                            //
/////////////////////////////////////////////////////////////////////////////////////////////

// declara y define conductas, con sus respectivas prioridades
Empujones empujones (50);    
ReactLoad reactLoad (60);
//Correa correa (70);
//Follow follow (80);

// un array de punteros a conductas
Conducta *conducta [NUM_CONDUCTAS] = {&empujones, &reactLoad, /*&correa, &follow*/};  

void conducta_init () {     
	
	// todos los enables y disables iniciales van acá
	conducta [0]->enable ();
	conducta [1]->enable ();
	//conducta [2]->disable ();
	//conducta [3]->disable ();
	
	// inicialiceta del sensor inteligente de distancia, es común a varias conductas
	kbza.threshMode ();        // activa los umbrales de auto-envío
	
	/* esto iria en caso de desactivar todas las conductas asociadas al sensor inteligente de distancia
	   hay que ver la forma de integrarlo en la propia clase de las conductas
	// bitClear (timer0_int_flag, 2);    // Atención: el polling serial de la kbza no se puede apagar porque de él depende el control remoto
	                                     // de todas maneras esto equivaldría a desregistrarlo del timer0, y ya no tendría efecto porque 
																			 // ahora está harcodeado directo a la interrupción serial
	kbza.disable_send_all ();            // deshabilita la lectura automática de sensores
	kbza.noumbrales ();                  // resetea los umbrales                                     */
}

// actual = "modo" o "conducta a la que se le está dando bola"
// este modo determinaría lo que se muestra en el display, por ejemplo

void conducta_main () {           
	
	static byte actual = 0;         // conducta actual; la primera vez no coincide
	
	// cosas generales
	
	// requesta la distancia en cm, todo lo otro se activa por umbrales (en principio)
	kbza.request (CM_DIST);       // esto es para las conductas que usan el sensor inteligente de distacia
	
	if (idle >= IDLE_THRESH) {
		Conducta::estado = IDLE;    // esto es muy importante porque es un dato que viene del propio robot
	}
	
	// evalua cada conducta y a continuación, analiza qué hacer
	// algoritmo básico = de todas las conductas que retornan true, le hace caso a la de mayor prioridad	
	/* posibles mejoras:
	   - tareas simultáneas? porque ocupan distintos "recursos"?
	   - prioridades relativas a cada conducta                                                               */
	
	char selected = -1, max_prioridad = 0;
	for (byte f=0; f<NUM_CONDUCTAS; f++) {
		if (conducta[f]->enabled) {												                         // principio de habilitación de conductas
			if (conducta[f]->evaluar() && (conducta[f]->prioridad > max_prioridad)) {    // evalua y compara prioridad
				selected = f;
				max_prioridad = conducta[f]->prioridad;
			}
		}
	}
	
	if (selected == -1) {return;}          // evita que se ejecute nada si todas las conductas retornaron false
																		       // también se puede definir una conducta "Idle" con prioridad 0 que retorne siempre true
		
	if (selected != actual) {conducta[actual]->salir();}        // si cambia la conducta, ejecuta salir() en la conducta saliente
	actual = selected;                                   // actualiza
	Conducta::estado = conducta[selected]->ejecutar();          // ejecuta la conducta nueva y actualiza el estado del robot
		
}