// mov_alto_nivel.cpp created for project v8.2 on 06/05/2011 03:10:08

/* la intención de esta clase es convertirse en la clase universal de movs de alto nivel que 
   actualmente están en remote_control... el is_moving, los ángulos, etc., va a pasar a estar 
	 todo acá, y va a ser controlable por todos los sensores, no sólo por el control remoto..
	 también, de alguna manera, hacer que se puedan cambiar los parámetros de la caminata realtime  */
	 
/* talvez ahora que surgió lo de las conductas, este archivo desaparezca */


/*****************************************
* 
ESTO SE CONVIRTIO EN EL BASURERO
*
******************************************/

// otras funciones que se pueden llegar a usar

//#define word(...) makeWord(__VA_ARGS__)
//uint16_t makeWord(uint16_t w);
//uint16_t makeWord(byte h, byte l);
//void curved_noise (float *ptr, float center, float amp, float curvature);
//float log_random (float curvature);
//int mcd (int a, int b);
//byte maximo (float* array, byte largo);
//float fourier (float ampl[], byte fase[], int index);
//void decrement_p (volatile byte *p);
//byte ring (byte posicion, byte p);
//byte markov (byte largo, ...);

//===================================================================================================================================
//======================================= otras utils que se pueden llegar a necesitar ==============================================
//===================================================================================================================================

/*

unsigned int makeWord(unsigned int w) { return w; }
unsigned int makeWord(unsigned char h, unsigned char l) { return (h << 8) | l; }

void curved_noise (float *ptr, float center, float amp, float curvature) {           // suma un valor aleatorio a una variable #2                                   
  char sign = bin2sign (*ptr > center);                                       // revisar que esté correcto el signo
  *ptr += amp * sign * log_random (curvature);
}

float log_random (float curvature) {              // curvature=1, random normal; curvature>1, valores próximos a 0; curvature<1, valores próximos a 1                      
  return pow(random(1), curvature); 
}

int mcd (int a, int b) {                        // máximo común divisor, algoritmo recursivo clásico
  if (a % b == 0) {return b;}
  else {return mcd (b, a%b);}
}

byte maximo (float* array, byte largo) {            // rutina clásica para sacar el máximo (devuelve la posición del máximo en un array)
  byte maxx = 0;
  for (byte i=1; i<largo; i++) {                                                                   
    if (array [i] > array [maxx]) {maxx = i;}
  }
  return maxx;
}

float fourier (float ampl[], byte fase[], int index) {          // da el valor instantáneo de la suma de sinusoides con amplitudes ampl[] y fases fase[]
  float y = 0;
  for (byte freq=0; freq<9; freq++) {
    y += ampl[freq] * seno [(index*freq + fase[freq])%16];
  }
  return y;
}

void decrement_p (volatile byte *p) {                                // decrementa la posicion en buffers circulares de tamaño=32
  *p = (*p - 1) & 31;  
}

byte ring (byte posicion, byte p) {                                   // devuelve la posición relativa en buffers circulares de tamaño=32
  return (p + posicion) & 31;
}

byte markov (byte largo, ...) {             // devuelve un valor aleatorio según las probabilidades de transferencia
                                            // ejemplo: markov (7,1,3,1,0,2,0,5); --> devuelve valores de entre 0 y 6; 0 con 1/12, 1 con 3/12, 2 con 1/12, etc. de probabilidad
  int sum = 0;
  byte prob [largo];
  
	va_list valores;
  va_start(valores,largo);
  for (byte f=0; f<largo; f++) {
    prob[f] = va_arg(valores,int);
    sum += prob[f];
  }
  va_end(valores);
	
	float num = random (sum);
  byte i;
  for (i=largo-1; i>=0; i--) {                   
    sum -= prob[i];
    if (num >= sum) {break;}
  }
  return i;  
} 

*/

//COORD3D proyeccion (COORD3D P, COORD3D A, COORD3D B);

/* devuelve la diferencia entre el punto P y su proyección sobre el plano mediador del segmento AB
COORD3D proyeccion (COORD3D P, COORD3D A, COORD3D B) {
	// ecuación del vector normal
	COORD3D normal = producto (resta(B, A), 2);
	// término independiente de la ecuación del plano
	float d = sumaproducto (A, A) - sumaproducto (B, B);
	// precalcula a^2 + b^2 + c^2
	float sqabc = sumaproducto (normal, normal);
	// calcula la proyección
	float t = (sumaproducto (P, normal) + d) / sqabc;
	return producto (normal, -t);
} */

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* zona de reciclaje

// exploración, es la rutina que sigue los objetos

// otras rutinas interesantes eran los movimientos cíclicos y brownianos de la cabeza

void exploracion () {                                                                // mide la distancia del objeto y lo "sigue con la mirada"
  byte contador = 0;
  char static incremento [2] = {1, 1}; 
  char resultado [2] = {0, 0}; 
  while (contador < 2) {                                                             // 2 = número de repeticiones (cuando no hay cambios)
    for (byte i=0; i<2; i++) {                                                       // actúa sobre el servo 0 (altura) y el 2 (giro)
      servo (i*2, incremento [i]*(i+1), false); delay (60);                 
      servo (i*2, -2*incremento [i]*(i+1), false); delay (80);                       // mueve desde -incremento a +incremento; el delay es para darle tiempo a q se mueva
      resultado [i] = (resultado [i] + velocidad2())/2;                              // toma nota del acercamiento o alejamiento del objeto
      incremento [i] = 1 + 150/(50+global[0]) + 3/(1+abs(resultado[i]));             // para compensar el error de medida por la distancia
      servo (i*2, incremento [i]*(i+1) - resultado[i], false);                       // retorna a la nueva posición central
    }                                                                                // el incremento es menor cuanto más próximo está el objeto
    if (global[0] < 10 || abs(resultado[0]) + abs(resultado[1]) < 13) {
      contador ++;} else {contador = 0;}                                             // otro umbral importante
  }
}

*/

//========================================================================================================================================================
//*************************************************************  COMENTARIOS  ****************************************************************************
//========================================================================================================================================================


/* ejemplo de oscilador (creo que ya se puede descartar)

    mov.set_oscilador (ROT_Z, .6, .05, 0, true); 
		mov.set_oscilador (ROT_Y, .6, .05, 0, true); 
		mov.oscilador (2, 1000);
		delay (5000);
		
*/


// reciclar esto (monitor de 18 cosas)
		/*
		static byte motor_index = 0;                        // motor al que se le mide el load
	  byte pata = motor_index / 3;
	  byte anillo = motor_index % 3;
		if (motor_index==0) {pantalla.cls(); delay (5);}
		pantalla.setColor (LISO, 32+load[pata][anillo],0);
		delay (1);
		pantalla.setPixel(anillo+3,6-pata);    // esto podría ser un tipo de visualizador, atenti
		delay (10);
		motor_index ++;
	 motor_index %= 18;
	 */

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


/*
			byte inferior, superior;
			switch (accion.param1) {
				case 0: superior=2; inferior=3; break;
				case 1: superior=2; inferior=0; break;
				case 2: superior=5; inferior=0; break;
				case 3: superior=0; inferior=5; break;
				case 4: superior=3; inferior=5; break;
				case 5: superior=3; inferior=2; break;
			}
			COORD3D P = proyeccion (rel2abs(pos_des[accion.param1],accion.param1), rel2abs(pos_des[inferior],inferior), rel2abs(pos_des[superior],superior));
*/