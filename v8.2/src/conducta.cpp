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
#include "movimiento.h"
#include "hardware.h"
#include "display.h"
#include "vectores.h"


ACTION Conducta::estado;

Conducta::Conducta (byte in_prioridad) {
	prioridad = in_prioridad;
}

void Conducta::ejecutar (ACTION accion) {   // ejecuta la accion, y actualiza el estado
	 
	static byte last_type = IDLE;
	static unsigned int counter = 0;
	
	if (accion.tipo == last_type) {
		counter ++;
	} else {
		counter = 0;
		last_type = accion.tipo;
	}
	
	switch (accion.tipo) {
		case IDLE:
			break;
		case STOP:
			break;
		case WALK:
			mov.recta (accion.param2, 10, HALF_PI*accion.param1);    // por ahora caminatitas de 10 cm
			estado = accion;
			break;
		case SLOW:
			// en el futuro esto va a llamar a una función específica en la movimiento
			
			break;
		case AFRAID:
			break;
		case BODY:
			break;
		case LEG:
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
			
}

Empujones::Empujones (byte in_prioridad) : Conducta (in_prioridad) {
	// setea la red neuronal de 1 capa que procesa el load
	process = (Nlayer*) malloc (sizeof(Nlayer));
	*process = Nlayer (3, 18);
	process->node[0].set_weights((float[]){0, -.05, -.025, 0, -.05, -.025, 0, -.05, -.025, 0, -.05, -.025, 0, -.05, -.025, 0, -.05, -.025, 0});   // abajo
	process->node[1].set_weights((float[]){-.025, 0, 0, -.05, 0, 0, -.035, 0, 0, -.025, 0, 0, -.05, 0, 0, -.035, 0, 0, 0});                  // atrás
	process->node[2].set_weights((float[]){.025, 0, .025, 0, 0, .05, -.025, 0, .025, -.025, 0, -.025, 0, 0, -.05, .025, 0, -.025, 0});         // izquierda
}

void Empujones::evaluar () {  
	
	// reducir falsos positivos
	// activar diagonales
	// corregir asimetria
	
	static float last_x, last_z, DCB_x, DCB_z;  // DC blocking    
	
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
			entrada[pata*3+anillo] = .35*load[pata][anillo];    // esto está raro
		}
	}
	process->compute (entrada, salida);  // invoca a la red nuronal
	salida[2] -= .5;
	salida[1] -= .5;
	DCB_x = salida[2]-last_x+.993*DCB_x; last_x = salida[2];       // DC-blocking..
	DCB_z = salida[1]-last_z+.993*DCB_z; last_z = salida[1];       // ..(1-pole IIR HPF)
			
	char x = 215*DCB_x;       //
	char z = 250*DCB_z;       //  
			
	int e_sum, e_dif;
	if (sign(x) == sign(z)) {
		e_sum = abs (x+z); e_dif = abs (x-z);
	} else {
		e_sum = abs (x-z); e_dif = abs (x+z);
	}
	
	// si no hay suficiente empuje...
	if ((e_dif<55 && e_sum<100) || e_dif<35) {                    // corregir: magic numbers
		accion.tipo = IDLE;
		return;
	}
	
	// de lo contrario...
	accion.tipo = WALK;       // param1 = angulo; param2 = velocidad
	if (abs(x) > abs(z)) {
		if (x > 0) {accion.param1 = 0;} else {accion.param1 = 2;}
	} else {
		if (z > 0) {accion.param1 = 3;} else {accion.param1 = 1;}
	}
	accion.param2 = e_dif/2.5;   // proporcional al empuje

}

ReactLoad::ReactLoad (byte in_prioridad) : Conducta (in_prioridad) { }

void ReactLoad::evaluar () {  // también tiene que evaluar si una pata tiene poco load
	
	if (estado.tipo != IDLE) {
		accion.tipo = IDLE;
		return;
	}
  
	int load_pata [6] = {0, 0, 0, 0, 0, 0};
	byte maximo = 0;
	for (byte pata=0; pata<6; pata++) {
		for (byte anillo=0; anillo<3; anillo++) {
			load_pata[pata] += sq(load[pata][anillo]);
		}
 		if (load_pata[pata] > load_pata[maximo]) {maximo = pata;}
	}
		
	if (load_pata[maximo] > 3950) {     // corregir: número mágico 
		accion.tipo = LEG;
		accion.param1 = maximo;
	} else {accion.tipo = IDLE;} 

}


/////////////////////////////////////////////////////////////////////////////////
//                                 "main"                                      //
/////////////////////////////////////////////////////////////////////////////////

// declara y define conductas, con su relativa prioridad
Empujones empujones (80);    
ReactLoad reactLoad (100);  

// un array de punteros a conductas
Conducta *conducta [NUM_CONDUCTAS];  

void conducta_init () {     // esto también

	conducta [0] = &empujones;
	conducta [1] = &reactLoad;
	
}

void main_ () {           // esto hay que lograr meterlo adentro de la clase
	
	if (idle >= IDLE_THRESH) {
		Conducta::estado.tipo = IDLE;    // esto es muy importante porque es un dato que viene del propio robot
	}

	// evalua cada conducta
	for (byte f=0; f<NUM_CONDUCTAS; f++) {
		conducta[f]->evaluar();	
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