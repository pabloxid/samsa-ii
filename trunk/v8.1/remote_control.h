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

// remote_control.h
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

#ifndef REMOTE_CONTROL_H
#define REMOTE_CONTROL_H

#include "vectores.h"
#include "WString.h"

// definiciones de las teclas del control remoto IR MSI 
#define RC_POWER          18
#define RC_SUSPEND        80
#define RC_SLEEP          81
#define RC_HOME           82
#define RC_DVD1           84
#define RC_FM             86
#define RC_UP             92
#define RC_DOWN           95
#define RC_LEFT           93
#define RC_RIGHT          94
#define RC_ENTER1         83
#define RC_MENU           16
#define RC_EXIT           78
#define RC_MTS            19
#define RC_CCTTX          79
#define RC_TV             64
#define RC_FMRADIO        65
#define RC_MUSIC          66
#define RC_PICTURES       67
#define RC_VIDEOCLIP      68
#define RC_DVD2           69
#define RC_GAMEZONE       70
#define RC_APPLICATION    71
#define RC_CH_UP          26
#define RC_CH_DN          30
#define RC_VOL_UP         27
#define RC_VOL_DN         31
#define RC_CH_RTN         23
#define RC_ENTER2         77
#define RC_MUTE           12
#define RC_REW            22
#define RC_PLAYPAUSE      20
#define RC_FFWD           24
#define RC_REC            72
#define RC_PREV           73
#define RC_STOP           89
#define RC_NEXT           91

// teclas numéricas = lo que dicen

// modos del control remoto
enum {REMOTE_OFF, CAMINATAS1, CAMINATAS2, TRASLACIONES, ROTACIONES, EDITAR_CENTRO, EDITAR_POS, OSCILADORES1,  OSCILADORES2, OSCILADORES3};


class RemoteControl {

	public:
		RemoteControl ();
		void procesar_comando (byte comando); 
	
	private:
		String float2string (float number);
		void displayText ();
		
		// las variables son cualquiera: hay que rever todo esto
		float velocidad;
		byte marcha;
		float angulo;
		float angulo_offset;
		float largo_pasos;
		float step;           // es la unidad en las traslaciones y las rotaciones
		float desplazamiento;
		COORD2D centro_caminata;               
		COORD3D centro_rotacion;               
		float inc;
		bool isMoving;
		byte modo;
		char pagina;
		String texto1, texto2;
		byte color1, color2;
		bool retardo;
	  
	  // handlers
	  void caminatas (byte comando);
	  void traslaciones (byte comando);
	  void rotaciones (byte comando);
		void editar_centro (byte comando);
		void editar_pos (byte comando);
		void osciladores (byte comando);
		
};

extern RemoteControl rc;

#endif