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

// acá faltan rutinas para mover los servos y para interpretar el sensor de cuerdita

#include "wiring.h"
#include "cabeza.h"
#include "remote_control.h"
#include "bluetooth.h"
#include "display.h"
#include "settings.h"

Cabeza kbza;                                                      // objeto Cabeza

// variables estáticas
volatile byte Cabeza::cm_dist;
volatile char Cabeza::vel, Cabeza::lat, Cabeza::evnt_vel, Cabeza::evnt_lat;
volatile int Cabeza::mm_dist;
volatile char* Cabeza::values[4] = {&vel, &lat, &evnt_vel, &evnt_lat};    // array de punteros a las variables, para indexarlas
volatile byte Cabeza::flag;
//volatile byte Cabeza::fastDistanceBuffer [100];
//RingBuffer <volatile byte> Cabeza::fastDistance = RingBuffer <volatile byte> (fastDistanceBuffer, 100);      // esto no sé si se va a usar...

Cabeza::Cabeza () {
	comm = Serialcomm (&Serial2, 115200, 3, 2, &callback);    // objeto Serialcomm
	flag = 0;
}

/*
void Cabeza::set_pos (byte servo, int angulo, bool absolute) {
	if (!absolute) {angulo += angulo_act [servo];}
	angulo_act [servo] = angulo;
	if (servo==0) {servo=S_1_PIN;} else {servo=S_2_PIN;}
	analogWrite (servo, S_CENTER + angulo);
}

int Cabeza::get_pos (byte servo) {
	return angulo_act [servo];
}
*/

int Cabeza::twobyte2int (byte* data) {          // transforma 2 bytes de 7 bits en un int
  return 128*data[0] + data[1];
} 

// usar esta rutina cuando está habilitado el polling (por defecto)
void Cabeza::request (byte parametro) {
	comm.send_msg (parametro, 0, NULL);      // los requests son todos de 0 byte
}

////////////// ATENCIÓN: no usar esta rutina si está habilitado el polling \\\\\\\\\\\\\\

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

/* setear los umbrales:
CM_DIST y MM_DIST el umbral por defecto es 0, y se activan cuando la distancia es MENOR que el umbral
El resto de los parámetros, el umbral por defecto es 127, y se activan cuando |valor| > umbral      */

void Cabeza::set_threshold (byte parametro, unsigned int valor) {
	if (parametro < MM_DIST) {
		byte value = valor;
		comm.send_msg (parametro, 1, &value);
	} else if (parametro == MM_DIST) {
		comm.send_2byte_msg (0, valor);
	}     
} 

void Cabeza::set_time (byte ms) {    // período de muestreo. Valor por defecto 10ms
  comm.send_msg (5, 1, &ms);
}

void Cabeza::enable_send_all () {    // nota: esto cambió en la v4 del firmware cabeza
	byte value = true;
	comm.send_msg (7, 1, &value);
}               

void Cabeza::disable_send_all () {    // nota: esto cambió en la v4 del firmware cabeza
	byte value = false;
	comm.send_msg (7, 1, &value);
}               

// los siguientes son presets de umbrales y modos de operación asociados a ellos

void Cabeza::threshMode () {
	disable_send_all ();          // deshabilita la lectura automática de sensores
	umbrales ();                // habilita los umbrales 
	// flag = 0;
}
	
void Cabeza::autoMode () {
	noumbrales ();                 // deshabilita los umbrales
	set_time (10);                 // período de 10 milisegundos para envio automático	
	enable_send_all ();             // habilita la lectura automática de sensores (aka "muestreo")
	// ATENCION: si se usa el send all, hay una probabilidad de falla debida al running_status
	// para reducirla, hay que requestar de vez en cuando otra cosa
}

void Cabeza::umbrales () {
	set_threshold (VEL, 2);        // habilita el umbral de velocidad, lo que equivale a notificar cada vez que hay un cambio de distancia
	set_threshold (MM_DIST, 280);   // umbral de distancias en mm cuando es menor a 28cm
	set_threshold (EVNT_VEL, 3);    // evento de velocidad
	set_threshold (EVNT_LAT, 3);    // evento lateral
	set_threshold (LAT, 1);        // umbral lateral
	// el único umbral que no se define es el de CM_DIST, que se opta por leerlo manualmente
	set_time (1);                // período de 1 milisegundo; máxima respuesta para el modo umbrales	
}

void Cabeza::noumbrales () {     // resetea todos los umbrales (asi vienen cuando se prende el robot)
	set_threshold (VEL, 127);        
	set_threshold (MM_DIST, 0);      
	set_threshold (EVNT_VEL, 127);    
	set_threshold (EVNT_LAT, 127);    
	set_threshold (LAT, 127);
}

/* esta rutina lo único que hace es meter los valores recibidos en sendas variables,
  pero es posible usar otro comportamiento, por ejemplo en el caso de un "evento",
	provocado por un umbral, que directamente dispare una acción, evitando que tenga que haber
	otro nivel de polling sobre las variables para ver si ocurrió algun evento o no. */
	
/* IMPLEMENTAR: que haya "modos" de respuesta al llamado, 
  por ejemplo, mientras el robot está andando, todos los llamados provocarían un stop() */
	
void Cabeza::callback (byte instruccion, byte largo, byte* data) {
	
	// combina instrucción y largo, dando una combinación única
	instruccion += 8*largo;                        
	
	switch (instruccion) {
		case 8:                                          // distancia cm (1 byte)
			cm_dist = data[0];
			flag |= 1;                                     // bit 0 de flag
			break;
		case 9: case 10: case 11: case 12:                    // vel, lat, evnt_vel y evnt_lat (1 byte con signo)
			*values[instruccion-9] = data[0] - 100;
			bitSet (flag, instruccion-8);                      // bits 1, 2, 3 y 4 de flag
			break;
		case 13:                                         // "fast" distance (cm) 
			//fastDistance.store(data[0]);                      // lo guarda en el buffer circular
			break;
		case 14:                                         // "scan"
			
			break;
		case 15:                                          // comando IR
			/* El comando IR llama directamente a la acción. Los otros 
			eventos, en cambio, se limitan a setear variables  */
			rc.procesar_comando (data[0]);
			break;
		case 16:                                          // distancia en mm (2 bytes)
			mm_dist = twobyte2int (data);
			bitSet (flag, 5);                                // bit 5 de flag
			break; 
		case 31:                                         // send all (distancia cm + vel + lat)
			cm_dist = data[0];
			vel = data[1] - 100;                             // nota: el send_all no modifica flag
			lat = data[2] - 100;
			// ABSOLUTAMENTE PROVISORIO Y TRUCHO
			if (TEST_SENSOR) {blue.send_msg (instruccion, largo, data);}       // para testear con el osciloscopio
			// termina provisorio   
			break;
	} 

}

/* interpretación de flag:
	
	bit0 = cm_dist
	bit1 = vel
	bit2 = lat
	bit3 = evnt_vel
	bit4 = evnt_lat
	bit5 = mm_dist

	"send_all" no modifica el flag   */