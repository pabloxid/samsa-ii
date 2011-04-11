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

// events.h
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

#ifndef EVENTS_H
#define EVENTS_H

#define BUFFER  128                    // cantidad de eventos que se pueden agendar

#include "vectores.h"

typedef struct {byte patas; COORD3D coord; int duracion; bool absolute;} MOVDATA;
typedef struct {unsigned long timestamp; MOVDATA movdata;} EVENT;

class Events
{
  public:
    Events (byte _max_size);                                    // declare deque
    byte process ();                                           // execute events
    bool add (MOVDATA movdata, int rel_time);                      // add entry & sort deque
    COORD3D search (byte pata, int rel_time);                      // search into deque for specific timestamp events
   
  private:
    volatile unsigned long timer;
    EVENT *buffer;
    byte head;
    byte size;
    byte max_size;
    unsigned long get_time ();                                     // get current timestamp 
    MOVDATA get_data ();                                          // get current data & remove entry
    void swap (byte index1, byte index2);
    byte get_index (byte position);
    int search_index (unsigned long timestamp, byte pata);
     
};

extern Events eventos;

#endif