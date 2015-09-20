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

// mov_bajo_nivel.cpp
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

#include "mov_bajo_nivel.h"
#include "display.h"
#include "ax12.h"
#include "util.h"
#include "hardware.h"
#include "Print.h"
#include "dsp.h"
#include "settings.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                               movimiento bajo nivel                                                             //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const byte ids[6][3] = {{0,1,2}, {4,5,6}, {8,9,10}, {12,13,14}, {16,17,18}, {20,21,22}};      // IDs de los motores
// const char offset[3] = {0, 0, 0};                                                              // offsets angulares de cada anillo (no se usó al final)

AX12 motors [6][3];                                          // array de los 18 motores
AX12 broadcast = AX12();                                      // objeto broadcast, rara vez usado
int ang_des [6][3];                                          // ángulo destino de cada motor (ojo puede necesitar 'volatile')
int dur_des [6][3];                                          // duración de cada pata  (ojo puede necesitar 'volatile')
COORD3D pos_des [6];                                         // posición destino de cada pata  
char load [6][3];                                            // carga de los motores
volatile int sns_fuerza, sns_angulo;                             // sensores analógicos (de cuerdita)

void servo (byte pata, byte anillo, int angulo, int duracion) {                 
  ang_des [pata][anillo] = angulo;    
  dur_des [pata][anillo] = duracion;
}

void pos_ang (byte pata, ANGULOS A, int duracion) {                       // mueve 1 pata
  servo (pata, 0, A.gama, duracion);                           
  servo (pata, 1, A.alfa, duracion);                          
  servo (pata, 2, A.beta, duracion);    
}

ANGULOS alfabetagama (COORD3D P) {     
  // convierte las coordenadas cartesianas en los ángulos correspondientes
  // recibe: xn, y, z; devuelve: gama (ángulo cuerpo|coxa), alfa (ángulo coxa|femur), beta (ángulo femur|tibia)  
  // fundamento: teorema de pitágoras + teorema del coseno (ver figura) 
  
  P.x = abs (P.x);        // pequeño truco para abarajar la simetría del eje x 
  ANGULOS A;
  
  // fase 1, calculamos "x" y el ángulo "gama"
  // COXA es la distancia entre el eje del primer motor y el segundo
  float x = hypot (P.z, P.x) - COXA;   // P.x = xn (esto es confuso)                          
  A.gama = ANG_SCALE*(atan(P.z/P.x))/PI;                   
  
  // fase 2, calculamos alfa y beta
  // ver constantes definidas al principio
  float sqx = sq(x); float sqy = sq(P.y);
  float cosalfa = (sqx + sqy + sqaminussqb) / (doublea*sqrt(sqx + sqy));          // coseno del angulo que forman el "femur" y la hipotenusa
  float cosbeta = (sqaplussqb - sqx - sqy) / doubleab;                         // coseno del angulo que forman "femur" y "tibia"
  A.alfa = ANG_SCALE*(acos(cosalfa) + atan(P.y/x))/PI;
  A.beta = ANG_SCALE/2 - ANG_SCALE*acos(cosbeta)/PI;
  
  return A;  
}

COORD3D xyz (int angulos[]) {                   // "cinemática directa"
  float rad [3]; 
  for (byte i=0; i<3; i++) {
    rad[i] = PI * angulos[i] / ANG_SCALE;        // convierte a radianes
  }
  // rad[0] = gama; rad[1] = alfa; rad[2] = beta 
  float sqc = sqaplussqb - (doubleab * cos(rad[2]-HALF_PI));
  float c = sqrt(sqc);
  float cosalfa = (sqc + sqaminussqb) / (doublea*c);
  float delta = rad[1] - acos(cosalfa);
  float x = c * cos(delta);
  return (COORD3D) {(x+COXA)*cos(rad[0]), c*sin(delta), (x+COXA)*sin(rad[0])};
}

void set_coord (MOVDATA &m) {     
  // mueve x patas con coordenadas cartesianas
  // el byte "patas" indica en binario qué patas van
  ANGULOS A;
  if (m.absolute) {A = alfabetagama (m.coord);}
  for (byte pata=0; pata<6; pata++) {
    if ((m.patas>>pata)&1) {
      if (m.absolute) {
        pos_des [pata] = m.coord;
      } else {
        sumasigna (&pos_des[pata], m.coord);
        A = alfabetagama (pos_des [pata]);
      }      
      pos_ang (pata, A, m.duracion);           // es siempre absoluta, en este punto
    }                   
  }
}

byte motor_update () {                                       // actualiza los servos
	static int ang_act [6][3];                                  // ángulo actual de los 18 servos
	byte targetlength = 0;
	byte targets [18]; int posvalues[18]; int velvalues[18];
	for (byte pata=0; pata<6; pata++) {
		for (byte anillo=0; anillo<3; anillo++) {
			int diff = ang_act[pata][anillo] - ang_des[pata][anillo];
			if (diff != 0) {
				targets [targetlength] = ids [pata][anillo];
				// posvalues [targetlength] = ANG_ZERO + bin2sign(pata>2) * (ang_des[pata][anillo] + offset[anillo]);   // versión con offset 
				posvalues [targetlength] = ANG_ZERO + bin2sign(pata>2) * ang_des[pata][anillo];                  // acá no habría que poner un constrain??
				velvalues [targetlength] = constrain(vel_scale*abs(diff)/dur_des [pata][anillo], 1, 1023);                
				// podemos mandar los mensajes individuales (para minimizar errores)...
				// motors[pata][anillo].setPosVel (posvalues [targetlength], velvalues [targetlength]);
				// (en ese caso no se justifica que haya una "update"; lo ideal es usar el gran mensajón)
				targetlength ++;
				ang_act[pata][anillo] = ang_des[pata][anillo];
			}
		}
	}
	if (targetlength > 0) {                               // por las dudas
	  // .. o podemos mandar el gran mensajón:
		AX12::setMultiPosVel (targetlength, targets, posvalues, velvalues);     // gran mensajón  
		// .. otra alternativa es separarlo en 2 mensajotes: 
		// AX12::setMultiVel (targetlength, targets, velvalues);
		// AX12::setMultiPos (targetlength, targets, posvalues);
	} 
	return targetlength;
}

COORD3D get_coord (byte pata) {    
  int angulos [3];
  all_timers_off ();
  for (byte i=0; i<3; i++) {
    angulos[i] = (motors[pata][i].getPos() - ANG_ZERO) * bin2sign(pata>2);       // los motores no tienen la propiedad "inverse"
  } 
  all_timers_off (RESTORE);
  COORD3D P = xyz (angulos);
  return (COORD3D) {P.x*bin2sign(pata>2), P.y, P.z};   
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                               inicialización motores                                                            //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int motor_init (byte pata, byte anillo) {                    
  
	motors[pata][anillo].writeInfo (LED, 1);               // prende led
	
	// if (pata>2) {motors [pata][anillo].inverse = true;}       // propiedad "inverse" (no la usamos por el momento)
	// motors[pata][anillo].setEndlessTurnMode (false);          // lo pone en modo servo (no es necesario)
  
	if (MOTORS_EEPROM_CFG) {
		
		motors[pata][anillo].setSRL (RETURN_READ);                   // sólo responde a los mensajes de lectura
		motors[pata][anillo].writeInfo (MAX_TORQUE, 1023);
		motors[pata][anillo].writeInfo (LIMIT_TEMPERATURE, 75);     
		motors[pata][anillo].writeInfo (DOWN_LIMIT_VOLTAGE, 89);
		motors[pata][anillo].writeInfo (UP_LIMIT_VOLTAGE, 145);
		motors[pata][anillo].writeInfo (RETURN_DELAY_TIME, 50);
		
		// falta setear la alarma del led y el shutdown alarm
				
		/* límites angulares (una mejora para la biblioteca AX12: que sean propiedades del objeto)
		patas 0-2: anillo 1 - mínimo 199; anillo 2 - mínimo 323
		patas 3-5: anillo 1 - máximo 827; anillo 2 - máximo 698  */
		if (pata<=2) {
			if (anillo==1) {motors[pata][anillo].writeInfo (CW_ANGLE_LIMIT, 199);} 
			else if (anillo==2) {motors[pata][anillo].writeInfo (CW_ANGLE_LIMIT, 323);}
		} else {
			if (anillo==1) {motors[pata][anillo].writeInfo (CCW_ANGLE_LIMIT, 827);} 
			else if (anillo==2) {motors[pata][anillo].writeInfo (CCW_ANGLE_LIMIT, 698);}
		}
	
	}	
	
	/* CW_COMPLIANCE_MARGIN y  CCW_COMPLIANCE_MARGIN
  Expresan el error mínimo de posición a partir del cual el motor empieza a hacer fuerza 
  (CW y CCW es en cada una de las direcciones) */
  motors[pata][anillo].writeInfo (CW_COMPLIANCE_MARGIN, 1);
  motors[pata][anillo].writeInfo (CCW_COMPLIANCE_MARGIN, 1);
  
  /* CW_COMPLIANCE_SLOPE y  CCW_COMPLIANCE_SLOPE
  Determinan cómo aumenta el torque a medida que aumenta el error de posición. 
  Valores pequeños indican más aumento a menor error */
  if (anillo==0) {
    motors[pata][anillo].writeInfo (CW_COMPLIANCE_SLOPE, 64);
    motors[pata][anillo].writeInfo (CCW_COMPLIANCE_SLOPE, 64);
  } else if (anillo==1) {
    if (pata<=2) {
      motors[pata][anillo].writeInfo (CW_COMPLIANCE_SLOPE, 64);
      motors[pata][anillo].writeInfo (CCW_COMPLIANCE_SLOPE, 128);
    } else {
      motors[pata][anillo].writeInfo (CW_COMPLIANCE_SLOPE, 128);
      motors[pata][anillo].writeInfo (CCW_COMPLIANCE_SLOPE, 64);
    }
  } else {
    motors[pata][anillo].writeInfo (CW_COMPLIANCE_SLOPE, 64);
    motors[pata][anillo].writeInfo (CCW_COMPLIANCE_SLOPE, 64);
  }
    
	/* PUNCH Indica la fuerza mínima que el motor hace cuando comienza a actuar, es decir, 
  la fuerza de la que parte la curva, la fuerza que el motor hace ante el error de posición indicado por COMPLIANCE_MARGIN. 
  A partir de ahí, si el error de posición aumenta, la fuerza también aumenta, tan lentamente como se lo indique COMPLIANCE_SLOPE */
	motors[pata][anillo].writeInfo (PUNCH, 32);
    
 	motors[pata][anillo].writeInfo (LED, 0);                // apaga led
		
	enable_ovf0();           // esto permite que el tiempo sea registrado
	delay (41);             // este delay es necesario, no sé por qué 
	disable_ovf0();          // vuelve a apagar la interrupción del timer0
	
	return motors[pata][anillo].ping();      // testea a ver si el motor responde a un ping

}

void motor_setup ()   {                
	AX12::init (BAUDRATE);                             // inicializa la biblioteca AX12 al baudrate indicado
	for (byte pata=0; pata<6; pata++) {
		all_timers_off ();
		for (byte anillo=0; anillo<3; anillo++) {
			motors [pata][anillo] = AX12(ids[pata][anillo]);   // asigna los IDs
			int error = motor_init (pata, anillo);
			if (MOTORS_ENABLE) {
				if (error) {
				// en caso de error en algun motor
					enable_ovf0();                                              // esto habilita el delay() y el scrollText() 
					pantalla.setColor (DEGRADE_H, RGB(3, 0, 1), RGB(3, 2, 0));        
					String msg = "error pata " + String (pata, DEC) + ", motor " + String (anillo, DEC);
					for (;;) {pantalla.scrollText (msg.toCharArray(), 70); delay (6900);}                // loop indefinido
				} else {
					motors[pata][anillo].torqueOn;                                     // habilita el torque
					pantalla.setPixel(anillo+3,6-pata);                                 // motor testeado y configurado OK
				}
			} else {
				//motors[pata][anillo].torqueOff;                                              // esto no tiene ningún efecto, debido a un bug en la documentación de los AX12
			  motors[pata][anillo].writeInfo(TORQUE_LIMIT, 0);                       // en su lugar usamos esto que tiene el mismo efecto 
			}      
		}
		all_timers_off (RESTORE);
		pos_des [pata] = get_coord (pata);         // setea la "posición actual" a la posición real 
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                      carga (load) de los motores                                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int idle = 0;     // esto es importantísimo para las conductas

char buffer [18][5];                              // buffers para el filtrado de la carga de los motores
RingBuffer <char> filtro[18];                       // circularización de dichos buffers 

void poll_load () {               /* mide la carga en cada motor. Lo hace cuando los motores no están trabajando, y a un 
                                        ritmo de 14 veces por segundo, lo cual es acorde con las características del motor
																				El resultado queda en array 'char load [6][3]'                                     */
	                                                          
	static byte motor_index = 0;                        // motor al que se le mide el load
	byte pata = motor_index / 3;
	byte anillo = motor_index % 3;
	all_timers_off ();
	sei();                   // habilita las "nested interrupts"...
	// ...lo que hace posible la lectura del motor desde dentro de la ISR
	char carga = bin2sign(motor_index>8)*motors[pata][anillo].getLoad()/8;   // el valor es de 7 bits en realidad, por eso / 8    
	filtro[motor_index].store(carga);                                 // lo guarda en el buffer circular  
	mov_avg_filter (&filtro[motor_index], &load[pata][anillo], 4);         // aplica un moving average de 4 puntos
	cli(); 
	all_timers_off (RESTORE);
	motor_index ++;
	motor_index %= 18;
	
	idle ++;
}

void init_filters () {                            // inicialización de los buffers circulares
	for (byte f=0; f<18; f++) {
		filtro [f] = RingBuffer <char> (buffer[f], 5);          
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                 sensores analógicos (cuerdita)                                               //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SNS_SAMPLE   6                	  // período de muestreo sensores analógicos (en ms)
#define NUM_SENSORS  2                  	// cantidad de sensores analógicos
enum {SNS_FUERZA, SNS_ANGULO};        	// nombres de los sensores
byte sensor [NUM_SENSORS] = {6, 7};   	// mapea el índice de sensor al número de port analógico

void poll_analog_sensor (unsigned long milis) {
	
	static unsigned long timer = 0;
	static byte index;                 // alterna entre los sensores
	
	if (milis-timer >= SNS_SAMPLE) {   
		timer = milis;
		int val = ADCReadConv ();
		switch (index) {
			case SNS_FUERZA:
				sns_fuerza = val;                       // el valor se almacena como viene... 
				break;
			case SNS_ANGULO:
				sns_angulo = val;														  // ...para no gastar procesamiento acá
				break;
		}
		index ++;
		index %= NUM_SENSORS;
		ADCInitConv (sensor[index]);
	}
	
}

volatile byte cuerdita_flag = 0;

// atención de interrupción del sensor de cuerdita (provisorio)
// ojaldre con las nested interrupts
void sensor_cuerdita () {
	byte dat = 2*digitalRead (4) + digitalRead(5); 
	// esto es provisorio
	pantalla.scrollText (("C"+String(dat, DEC)).toCharArray(), 70);    //PRUEBA DE CONCEPTO (provisorio)
	// termina provisorio
	cuerdita_flag = 128 + dat;
}