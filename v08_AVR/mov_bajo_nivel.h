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

// mov_bajo_nivel.h
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

#ifndef BAJO_H
#define BAJO_H

#include "vectores.h"

typedef struct {int gama; int alfa; int beta;} ANGULOS;

// funciones
void set_coord (byte patas, COORD3D p, int duracion, bool absolute);
byte motor_update ();
void motor_setup ();
COORD3D get_coord (byte pata); 

// posición actual del robot (en realidad es la posición destino, desde el punto de vista del bajo nivel)
extern COORD3D pos_des[6];

#endif