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

// serialcomm.h
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

#ifndef SERIALCOMM_H
#define SERIALCOMM_H

/* el propósito de esta clase es implementar el lenguajecito de comunicación que se va a usar en los 3 puertos seriales
  (el serial1 no existe, porque el recurso hardware está usado por la AX12)
  Las instancias de este protocolo tienen todas la misma estructura básica, pero pueden diferir en la cantidad de bits asignados a la instrucción y al largo
  La clase dispone de 2 métodos para leer la data: uno que devuelve el mensaje y otro que genera un callback. 
*/


#include "HardwareSerial.h"

typedef unsigned char byte;


class Serialcomm {
  
  public:
    Serialcomm ();
    Serialcomm (HardwareSerial* port_, long baud, byte bits_instruccion, byte bits_largo, void (*callback_) (byte, byte, byte*));
    void leer_serial ();  
    bool leer_serial (byte instruccion_, byte largo_, byte *data_);
    void send_msg (byte instruccion_, byte largo_, byte *data_);
    void send_2byte_msg (byte instruccion_, unsigned int value);
    
  private:
    HardwareSerial* port;                              // serial port
    void (*callback) (byte, byte, byte*);                 // callback function
    byte largo, instruccion, cont;                       // largo, instruccion, cont
    byte* data;                                       // buffer 
    byte last;                                        // esto es para el running status    
    byte sync, mask, shift;                             // dependen de los bits asignados a instrucción y largo
    void read_byte ();                                  
    
};  


#endif