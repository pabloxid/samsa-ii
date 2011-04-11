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

// serialcomm.cpp
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

#include "serialcomm.h"
#include "wiring.h"
#include <string.h>

#define TIMEOUT 3000

Serialcomm::Serialcomm () { }   // construtor por defeto

Serialcomm::Serialcomm (HardwareSerial* port_, long baud, byte bits_instruccion, byte bits_largo, void (*callback_) (byte, byte, byte*)) {
  port = port_;
  port->begin (baud);
  sync = 256 - 1<<(bits_instruccion+bits_largo);
  mask = (1<<bits_instruccion) - 1;
  shift = bits_instruccion;
  data = (byte*) malloc (1<<bits_largo); 
  last = 0;
  callback = callback_;
}

void Serialcomm::leer_serial () {                   // esta es la "máquina de estados" que recibe y decodifica los mensajes Seriales 
  /* al cambiar el "if" por "while" tiene el efecto de
	vaciar el buffer cada vez que se invoca a esta rutina */ 
  while (port->available() > 0) {                   // si hay al menos 1 byte en el Serial... 
    read_byte ();                                // ..lo lee, lo interpreta, etc.
    if (cont == largo) {                          // cuando termina el cuerpo del mensaje..
      (*callback) (instruccion, largo, data);         // ..lo ejecuta                       
      cont = 0;                                 // y resetea cont, lo que produce un "running status" 
    }
  } 
}

bool Serialcomm::leer_serial (byte instruccion_, byte largo_, byte *data_) {         // esta otra versión de leer_serial, lee todo un mensaje y retorna la data     
																																										       // además verifica que el mensaje recibido sea realmente el esperado
  unsigned int timeout; 
  do {
    timeout = TIMEOUT;                             // timeout
    while (port->available() == 0) {
      timeout --;
      if (timeout == 0) {return false;}
    }
    read_byte ();
  } while (cont < largo_);
  memcpy (data_, data, largo_);
  cont = 0;
  return (instruccion == instruccion_ && largo == largo_);
} 

void Serialcomm::send_msg (byte instruccion_, byte largo_, byte* data_) {
  byte head = sync + instruccion_ + (mask+1)*largo_;
  if (head != last || largo_ == 0) {                   // el running status no es válido para mensajes de que son sólo head (largo=0)
    port->write (head);                                // byte de cabecera (la instrucción no incluye el largo)
  }
  port->write (data_, largo_);
  last = head;
}

void Serialcomm::read_byte () {
  byte b = port->read();                             // ...lo lee
  if (b >= sync) {                                   // ...si es un HEAD...
    instruccion = (b-sync)&mask;                      // hay [mask+1] tipos de mensajes (para cada largo) 
    largo = (b-sync)>>shift;                          // el largo de la data puede ser hasta [shift] bytes
    cont = 0;                                         // inicializa el índice del buffer                            
  } else {                                          // ...de lo contrario (si es un DATA)...
    if (cont < largo) {data[cont++] = b;}             // va llenando el buffer
  }
}

void Serialcomm::send_2byte_msg (byte instruccion_, unsigned int value) {
  send_msg (instruccion_, 2, (byte[]) {value>>7, value&127});                    // los valores de 2 bytes son arbitrariamente limitados a 14 bits
}