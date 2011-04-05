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

// events.cpp
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

#include "events.h"
#include "hardware.h"
#include "wiring.h"
#include "mov_bajo_nivel.h"

Events eventos (BUFFER);          // preinstanciado

Events::Events (byte _max_size) {
  max_size = _max_size;
  buffer = (EVENT*) malloc (max_size * sizeof(EVENT));
  head = 0;
  size = 0;
  timer = 0;
}

byte Events::process () {                                          // esta rutina es llamada desde la ISR
  byte count = 0;
  while (get_time() <= timer) {
    MOVDATA m = get_data ();                                       // cuando hace el get_data(), automáticamente se borra el evento
    set_coord (m.patas, m.coord, m.duracion, m.absolute);          
    count ++;
  }
  if (count > 0) {count = motor_update();}
  timer ++;  
  return count;                        // retorna la cantidad de motores updateados
}

bool Events::add (MOVDATA movdata, int rel_time) {     // add entry & sort deque
  if (size < max_size) {
    disable_ovf2();
    byte index = get_index (size);
    buffer [index].timestamp = timer + rel_time;
    buffer [index].movdata = movdata;
    // sort
    if (size > 0) {
      for (byte i=size; i>0; i--) {
        byte index1 = get_index (i-1);
        if (buffer[index].timestamp < buffer[index1].timestamp) {
          swap (index, index1);
          index = index1;
        } else {break;}    
      } 
    }   
    size ++;
    enable_ovf2();
    return true;  
  } else {
    return false;
  }
}  

unsigned long Events::get_time () {                                // get current timestamp 
  if (size > 0) {
    return buffer [head].timestamp;
  } else {
    return -1;                                                     // este -1 en realidad lo que retorna es el valor más alto posible, impidiendo que la isr llame a la get_data()
  }
}

MOVDATA Events::get_data () {                                      // get data & remove entry (atención: es responsabilidad de la isr no llamar esto cuando size==0)
  byte index = head;
  head = get_index (1);
  size --; 
  return buffer [index].movdata;
}

COORD3D Events::search (byte pata, int rel_time) {
  disable_ovf2();
  COORD3D result = {0, 0, 0};
  unsigned long timestamp = timer + rel_time; 
  int match;
  do {
    match = search_index (timestamp, pata);
    if (match != -1) {
      sumasigna (&result, buffer[match].movdata.coord);
      timestamp = buffer[match].timestamp-1;
    } else {break;}
  } while (!buffer[match].movdata.absolute);
  if (match == -1) {
    sumasigna (&result, pos_des[pata]);
  }
  enable_ovf2();
  return result; 
}


void Events::swap (byte index1, byte index2) {
  EVENT temp =  buffer [index2];
  buffer [index2] = buffer [index1];
  buffer [index1] = temp;
}

byte Events::get_index (byte position) {
  return (head + position) % max_size;
}
 
int Events::search_index (unsigned long timestamp, byte pata) {  
  int match = -1; 
  for (byte i=0; i<size; i++) {
    byte index = get_index (i);
    if (buffer[index].timestamp > timestamp) {break;}
    if ((buffer[index].movdata.patas>>pata)&1) {match = index;}
  }
  return match;
}