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

// vectores.h
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

#ifndef VECTORES_H
#define VECTORES_H

// tipos
typedef struct {float x; float z;} COORD2D;
typedef struct {float x; float y; float z;} COORD3D;
typedef struct {COORD3D patas[6];} POSICION;

typedef unsigned char byte;

float bezier (float p0, float p1, float p2, float t);
COORD3D suma (COORD3D punto1, COORD3D punto2);
COORD2D suma (COORD2D punto1, COORD2D punto2);
COORD3D resta (COORD3D punto1, COORD3D punto2);
COORD2D resta (COORD2D punto1, COORD2D punto2);
COORD3D producto (COORD3D vector, float numero);
COORD2D producto (COORD2D vector, float numero);
COORD3D xz2xyz (COORD2D vector);
COORD2D xyz2xz (COORD3D vector);
COORD3D applyMatrix (COORD3D P, COORD3D * matrix);
COORD2D applyMatrix (COORD2D P, COORD2D * matrix);
void getRotationMatrix (COORD2D * matrix, float angulo);
void getRotationMatrix (COORD3D * matrix, float angulox, float anguloy, float anguloz);
COORD2D getOffset (byte pata);
void sumasigna (COORD3D *punto, COORD3D vector);
void sumasigna (COORD2D *punto, COORD2D vector);
float distancia (COORD2D a, COORD2D b); 

byte patas2pata (byte patas);

POSICION hexagono (float altura, float ancho, float largo, float ancho_central);
POSICION rotate (COORD3D *pos_ref, COORD3D centro, COORD3D *matrix);
POSICION traslate (COORD3D *pos_ref, COORD3D traslacion);


#endif