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

// vectores.cpp
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

#include "vectores.h"
#include "util.h"
#include "ax12.h"              // sólo para el bin2sign (se podría ubicar en util, pero no quiero modificar la ax12)
#include "mov_bajo_nivel.h"    // para acceder a 'pos_des[]';

float bezier (float p0, float p1, float p2, float t) {
  return sq(1-t)*p0 + 2*t*(1-t)*p1 + sq(t)*p2;
}

COORD3D suma (COORD3D punto1, COORD3D punto2) {
  return (COORD3D) {punto1.x + punto2.x, punto1.y + punto2.y, punto1.z + punto2.z}; 
}

COORD2D suma (COORD2D punto1, COORD2D punto2) {
  return (COORD2D) {punto1.x + punto2.x, punto1.z + punto2.z}; 
}

COORD3D resta (COORD3D punto1, COORD3D punto2) {
  return (COORD3D) {punto1.x - punto2.x, punto1.y - punto2.y, punto1.z - punto2.z}; 
}

COORD2D resta (COORD2D punto1, COORD2D punto2) {
  return (COORD2D) {punto1.x - punto2.x, punto1.z - punto2.z}; 
}

COORD3D producto (COORD3D vector, float numero) {
  return (COORD3D) {numero*vector.x, numero*vector.y, numero*vector.z}; 
}

COORD2D producto (COORD2D vector, float numero) {
  return (COORD2D) {numero*vector.x, numero*vector.z}; 
}

COORD3D xz2xyz (COORD2D vector) {
  return (COORD3D) {vector.x, 0, vector.z}; 
}

COORD2D xyz2xz (COORD3D vector) {
  return (COORD2D) {vector.x, vector.z}; 
}

COORD3D applyMatrix (COORD3D P, COORD3D * matrix) {
  float x = P.x*matrix[0].x + P.y*matrix[1].x + P.z*matrix[2].x;    
  float y = P.x*matrix[0].y + P.y*matrix[1].y + P.z*matrix[2].y;  
  float z = P.x*matrix[0].z + P.y*matrix[1].z + P.z*matrix[2].z;  
  return (COORD3D) {x, y, z};
}

COORD2D applyMatrix (COORD2D P, COORD2D * matrix) {
  float x = P.x*matrix[0].x + P.z*matrix[1].x;    
  float z = P.x*matrix[0].z + P.z*matrix[1].z;  
  return (COORD2D) {x, z};
}

void getRotationMatrix (COORD3D * matrix, float angulox, float anguloy, float anguloz) {
  float sinx = sin (angulox); float cosx = cos (angulox);  
  float siny = sin (anguloy); float cosy = cos (anguloy);  
  float sinz = sin (anguloz); float cosz = cos (anguloz); 
  matrix[0].x = cosy*cosz;  matrix[1].x = cosy*sinz; matrix[2].x = -siny;  
  matrix[0].y = -cosx*sinz+sinx*siny*cosz;  matrix[1].y = cosx*cosz+sinx*siny*sinz; matrix[2].y = sinx*cosy;
  matrix[0].z = sinx*sinz+cosx*siny*cosz;  matrix[1].z = -sinx*cosz+cosx*siny*sinz; matrix[2].z = cosx*cosy;
}

void getRotationMatrix (COORD2D * matrix, float angulo) {
  float seno = sin (angulo);
  float coseno = cos (angulo);
  matrix[0].x = coseno;  matrix[1].x = -seno;
  matrix[0].z = seno;  matrix[1].z = coseno;  
}

COORD2D getOffset (byte pata) {
  return (COORD2D) {ORIGEN*bin2sign(pata>2), (1-(pata%3))*SEPARA};      // nota: las patas de la izquierda son de x negativo
}

void sumasigna (COORD3D *punto, COORD3D vector) {
  (*punto).x += vector.x;
  (*punto).y += vector.y;
  (*punto).z += vector.z; 
}

void sumasigna (COORD2D *punto, COORD2D vector) {
  (*punto).x += vector.x;
  (*punto).z += vector.z; 
}

float distancia (COORD2D a, COORD2D b) {
  return hypot (a.x - b.x, a.z - b.z);
}

byte patas2pata (byte patas) {
  byte pata;
  for (pata=0; pata<6; pata++) {
    if ((patas>>pata)&1) {break;}
  }
  return pata;
}

// genera una posición hexagonal semi-regular
POSICION hexagono (float altura, float ancho, float largo, float ancho_central) {
  POSICION posicion;
  for (byte pata=0; pata<6; pata++) {
    COORD2D P = getOffset (pata);
    posicion.patas[pata] = (COORD3D) {(ancho+(ancho_central-ancho)*(P.z==0))*sign(P.x)/2 - P.x, -altura, largo*sign(P.z)/2 - P.z};
  }  
  return posicion;
}

// rota una posición con un centro y una matriz de rotacion
POSICION rotate (COORD3D *pos_ref, COORD3D centro, COORD3D *matrix) {
  POSICION posicion;
  for (byte pata=0; pata<6; pata++) {
    COORD3D O = xz2xyz (getOffset(pata));
    posicion.patas[pata] = resta (suma (applyMatrix (resta (suma (pos_ref[pata], O), centro), matrix), centro), O);
  }
  return posicion;
} 

// traslada una posición con un vector de traslación
POSICION traslate (COORD3D *pos_ref, COORD3D traslacion) {
  POSICION posicion;
  for (byte pata=0; pata<6; pata++) {
    posicion.patas[pata] = suma (pos_ref[pata], traslacion);
  }
  return posicion;
}