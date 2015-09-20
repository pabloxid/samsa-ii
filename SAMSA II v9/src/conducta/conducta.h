// Conducta.h created for project v8.2 on 08/16/2011 04:23:33

/* implementación de las conductas, según el esquema propuesto originalmete por Jorge y reformulado posteriormente por Chachi. 
   El esquema de Chachi se basa en la implementación subsumption de Lego Mindstorms.
   Comenzaremos por codificar únicamente lo que tiene que ver con el desplazamiento,
	 y las conductas asociadas con él, como responder a empujones o a la cuerdita         */


#ifndef CONDUCTA_H
#define CONDUCTA_H

#include "ann.h"

#define NUM_CONDUCTAS  2        // hay que ver si metemos todos los defines juntos

// estos son los posibles "estados" del robot
// el único válido es IDLE, los otros hay que reformularlos

enum {IDLE, STOP, WALK, SLOW, AFRAID, BODY, REST, LEG, ATTACK, FOLLOW};

typedef unsigned char byte;

void conducta_init ();
void conducta_main ();

class Conducta {   // clase abstracta: cada conducta implementa unas funciones "evaluar" y "ejecutar" distintas
	
	public:
		Conducta (byte in_prioridad);
		Conducta ();
		virtual bool evaluar () = 0;   // (virtual pura) Devuelve si la conducta quiere entrar 
		virtual byte ejecutar () = 0;   // (virtual pura) Devuelve el estado en que quedó el robot
		virtual void salir ();
		byte prioridad; 
		bool enabled;
		//ACTION accion;
		virtual void enable ();  
		virtual void disable ();
		
		//protected:
		static byte estado;     // es estática porque representa la situación actual del robot, es la misma para todas las conductas
															// podría ser una clase aparte, pero probemos así 
		
};

class Empujones : public Conducta {
	public:
		Empujones (byte in_prioridad);	
		bool evaluar ();
		byte ejecutar ();
	private:
		Nlayer *process;
		float modulo;
		float angulo;
};

class ReactLoad : public Conducta {
	public:
		ReactLoad (byte in_prioridad);	
		bool evaluar ();
		byte ejecutar ();
	private:
		char current_pata;
};

class Correa : public Conducta {
	public:
		Correa (byte in_prioridad);	
		bool evaluar ();
		byte ejecutar ();
		void enable ();     // sobrecargada
		void disable ();    // sobrecargada
};
 
class Follow : public Conducta {
	public:
		Follow (byte in_prioridad);	
		bool evaluar ();
		byte ejecutar ();
};

class Avoid : public Conducta {
	public:
		Avoid (byte in_prioridad);	
		bool evaluar ();
		byte ejecutar ();
};

class Miedo : public Conducta {
	public:
		Miedo (byte in_prioridad);	
		bool evaluar ();
		byte ejecutar ();
};

class Atacar : public Conducta {
	public:
		Atacar (byte in_prioridad);	
		bool evaluar ();
		byte ejecutar ();
};

class Deambular : public Conducta {
	public:
		Deambular (byte in_prioridad);	
		bool evaluar ();
		byte ejecutar ();
};

class Buscar : public Conducta {
	public:
		Buscar (byte in_prioridad);	
		bool evaluar ();
		byte ejecutar ();
};

extern Conducta *conducta [NUM_CONDUCTAS];

#endif