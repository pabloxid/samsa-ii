// Conducta.h created for project v8.2 on 08/16/2011 04:23:33

/* implementación de las conductas, según el esquema propuesto por Jorge.
   Comenzaremos por codificar únicamente lo que tiene que ver con el desplazamiento,
	 y las conductas asociadas con él, como responder a empujones o a la cuerdita         */


#ifndef CONDUCTA_H
#define CONDUCTA_H

#include "ann.h"

#define NUM_CONDUCTAS  4        // hay que ver si metemos todos los defines juntos

// estas son las posibles acciones
// hay que ver si con esto alcanza para codificar la parte de movimientos
// faltan los movs. del tronco
// cada acción debería comprender también display y mov. de cabeza

enum {IDLE, STOP, WALK, SLOW, AFRAID, BODY, REST, LEG, ATTACK, FOLLOW};

typedef struct {
	byte tipo;
	char param1;       // este parámetro para cosas enteras
	float param2;      // y este otro parámetro para cosas flotantes
} ACTION;

typedef unsigned char byte;

void conducta_init ();
void conducta_main ();

class Conducta {   // clase abstracta: cada conducta implementa una función "evaluar" distinta
	
	public:
		Conducta (byte in_prioridad);
		Conducta ();
		virtual void evaluar () = 0;   // virtual pura
		byte prioridad; 
		bool enabled;
		ACTION accion;
		virtual void enable ();  
		virtual void disable ();
		
	//protected:
		static ACTION estado;     // es estática porque representa la situación actual del robot, es la misma para todas las conductas
													     // podría ser una clase aparte, pero probemos así 
		
		static void ejecutar (ACTION accion);  // ejecuta la acción seleccionada, también estática
		
};


class Empujones : public Conducta {
	public:
		Empujones (byte in_prioridad);	
		void evaluar ();
	private:
		Nlayer *process;
};

class ReactLoad : public Conducta {
	public:
		ReactLoad (byte in_prioridad);	
		void evaluar ();
};

class Correa : public Conducta {
	public:
		Correa (byte in_prioridad);	
		void evaluar ();
		void enable ();     // sobrecargada
		void disable ();    // sobrecargada
};
 
class ReactDistance : public Conducta {
	public:
		ReactDistance (byte in_prioridad);	
		void evaluar ();
		void enable ();     // sobrecargada
		void disable ();    // sobrecargada
		void threshMode ();
		void autoMode ();
	private:
		void umbrales ();
		void noumbrales ();
};

extern Conducta *conducta [NUM_CONDUCTAS];

#endif