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

// cabeza.cpp
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

#include "wiring.h"
#include "cabeza.h"
#include "remote_control.h"

Cabeza kbza;                                           // objeto Cabeza
byte Cabeza::cm_dist;
char Cabeza::vel, Cabeza::lat, Cabeza::evnt_vel, Cabeza::evnt_lat;
int Cabeza::mm_dist;
char* Cabeza::values[4] = {&vel, &lat, &evnt_vel, &evnt_lat};   // array de punteros a las variables, para indexarlas

Cabeza::Cabeza () {
	comm = Serialcomm (&Serial2, 115200, 3, 2, &callback);    // objeto Serialcomm
}

void Cabeza::posicion (byte servo, int angulo) {
  if (servo==0) {servo = S_1_PIN;} else {servo = S_2_PIN;}
  analogWrite (servo, S_CENTER + angulo);
}

int Cabeza::twobyte2int (byte* data) {          // transforma 2 bytes de 7 bits en un int
  return 128*data[0] + data[1];
} 

// usar esta rutina cuando está habilitado el polling (por defecto)
void Cabeza::request (byte parametro) {
	comm.send_msg (parametro, 0, NULL);      // los requests son todos de 0 byte
}

/////////// ATENCIÓN: no usar esta rutina si está habilitado el polling \\\\\\\\\\\\\\

int Cabeza::read_value (byte parametro) {       // obtiene un valor individual, de los 6 valores definidos
  // se prepara para recibir la data
  byte instruccion, largo, data[2];
  if (parametro < MM_DIST) {             // los receive, son todos de 1 byte excepto...
    instruccion = parametro;
    largo = 1;
  } else if (parametro == MM_DIST) {     // ...la distancia en mm, que es de 2 bytes
    instruccion = 0;
    largo = 2;
  }
  // manda el request
  request (parametro);                          
  // recibe la data, comprueba error y devuelve valor
  if (comm.leer_serial (instruccion, largo, data)) {
    if (parametro == CM_DIST) {return data[0];}                        // distancia en cm (1 byte)
    else if (parametro == MM_DIST) {return twobyte2int (data);}           // distancia en mm (2 bytes)
    else {return data[0] - 100;}                                    // vel, lat, evnt_vel y evnt_lat (1 byte con signo)
  } else {
    return ERROR;
  }
} 

void Cabeza::set_threshold (byte parametro, unsigned int valor) {
  if (parametro < MM_DIST) {
    comm.send_msg (parametro, 1, (byte*) &valor);
  } else if (parametro == MM_DIST) {
    comm.send_2byte_msg (0, valor);
  }     
} 

void Cabeza::set_time (byte ms) {
  comm.send_msg (5, 1, &ms);
}

void Cabeza::enable_send_all () {comm.send_msg (6, 0, NULL);}

void Cabeza::disable_send_all () {comm.send_msg (7, 0, NULL);}

void Cabeza::callback (byte instruccion, byte largo, byte* data) {
	
	// combina instrucción y largo, dando una combinación única
	instruccion += 8*largo;                        
	
	switch (instruccion) {
    case 8:                                          // distancia cm (1 byte)
      cm_dist = data[0];
      break;
    case 9: case 10: case 11: case 12:                    // vel, lat, evnt_vel y evnt_lat (1 byte con signo)
      *values[instruccion-9] = data[0] - 100;
      break;
    case 15:                                          // comando IR
			 rc.procesar_comando (data[0]);
			 break;
		 case 16:                                          // distancia en mm (2 bytes)
      mm_dist = twobyte2int (data);
      break; 
    case 31:                                         // send all (distancia cm + vel + lat)
			 cm_dist = data[0];
			 vel = data[1] - 100;
			 lat = data[2] - 100;
      break;
  } 

}

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