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

// remote_control.cpp
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca


// ATENCION: toda esta clase está programada a lo chancho, con el objeto de prototipar rápidamente las ideas

#include "remote_control.h"
#include "util.h"
#include "movimiento.h"
#include "Print.h"
#include "Display.h"
#include "string.h"

RemoteControl rc;                     // ojebto

RemoteControl::RemoteControl () {
	velocidad = 10.0;
	marcha = 1;
	angulo_offset = 0;
	largo_pasos = 0;
	desplazamiento = -1;
	centro = (COORD2D) {0, 0};    
	inc = .5;                   // incremento para todas las variables
	color1 = RGB(2, 3, 2);
	modo = REMOTE_OFF;            // máquina de estados
	isMoving = false;
	retardo = false;
}	

void RemoteControl::procesar_comando (byte comando) {
	
	// acá se procesan las teclas generales, o que tienen una función única
	// las otras son procesadas por la rutina específica del modo
	
	texto1 = "def";
	
	switch (comando) {
		case RC_POWER: 
			if (isMoving || pantalla.isBusy()) {break;}
			if (modo == REMOTE_OFF) {
				color1 = RGB(0, 2, 3);
				texto1 = "REMOTE ON";
				modo = CAMINATAS1;
			} else {
				color1 = RGB(2, 3, 2);
				texto1 = "REMOTE OFF";
				modo = REMOTE_OFF;
			}
			break;
		case RC_SUSPEND:                                // una de estas tiene que resetear incluso la velocidad, paso, etc. (para facilitar la edicion)
			if (isMoving || pantalla.isBusy()) {break;}
			angulo_offset = 0;
			centro = (COORD2D) {0, 0};
			texto1 = "suspend";
			mov.goto_pos_ref (DEFAULT_POSITION);
			break;
		case RC_SLEEP: 
			break;
		case RC_HOME: 
			if (isMoving || pantalla.isBusy() || modo==REMOTE_OFF) {break;}
			angulo_offset = 0;
			centro = (COORD2D) {0, 0};
			texto1 = "home";
			mov.goto_pos_ref ();
			break;
		case RC_DVD1: 
			break;
		case RC_FM: 
			break;
		case RC_TV: 
			if (isMoving || pantalla.isBusy() || modo==REMOTE_OFF) {break;}
			color1 = RGB(0, 2, 3);
			texto1 = "caminatas 1";
			modo = CAMINATAS1;
			break;
		case RC_FMRADIO: 
			if (isMoving || pantalla.isBusy() || modo==REMOTE_OFF) {break;}
			color1 = RGB(1, 3, 1);
			texto1 = "caminatas 2";
			modo = CAMINATAS2;
			break;
		case RC_MUSIC: 
			if (isMoving || pantalla.isBusy() || modo==REMOTE_OFF) {break;}
			color1 = RGB(0, 3, 1);
			texto1 = "traslaciones";
			modo = TRASLACIONES;
			break;
		case RC_PICTURES: 
			if (isMoving || pantalla.isBusy() || modo==REMOTE_OFF) {break;}
			color1 = RGB(2, 2, 0);
			texto1 = "rotaciones";
			modo = ROTACIONES;
			break;
		case RC_VIDEOCLIP: 
			if (isMoving || pantalla.isBusy() || modo==REMOTE_OFF) {break;}
			color1 = RGB(2, 1, 3);
			texto1 = "editar centro";
			break;
		case RC_DVD2: 
			if (isMoving || pantalla.isBusy() || modo==REMOTE_OFF) {break;}
			color1 = RGB(1, 3, 3);
			texto1 = "modo 6";
			break;
		case RC_GAMEZONE: 
			break;
		case RC_APPLICATION: 
			break;
		// teclas numéricas
		case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: break;
		case RC_CH_RTN: break;
		case RC_ENTER2: break;
		case RC_MUTE: break;
		case RC_REW: break;
		case RC_PLAYPAUSE: break;
		case RC_FFWD: break;
		case RC_REC: break;
		case RC_PREV: break;
		case RC_STOP: break;
		case RC_NEXT: break;
		default:
			switch (modo) {
				case CAMINATAS1: case CAMINATAS2: caminatas (comando); break;
				case TRASLACIONES: traslaciones (comando); break;
				case ROTACIONES: rotaciones (comando); break;
			}
	}
	
	displayText ();
	
}

void RemoteControl::caminatas (byte comando) {
	
	// todo esto está programado como el orto, hay que reorganizarlo
	// por ejemplo este "anguloso"... talvez las otras variables también deban estar acá
	static float anguloso = 0;
	
	if (anguloso != 0) {
		COORD2D matrix [2];
		getRotationMatrix (matrix, anguloso);
		centro = applyMatrix (centro, matrix);
		anguloso = 0;
	}
	
	// switch de seteo
	switch (comando) {
		case RC_UP:
			texto1 = "UP";
			angulo = angulo_offset + PI/2;     
			break;
		
		case RC_DOWN:
			texto1 = "DN";
			angulo = angulo_offset - PI/2;
			break;
		
		case RC_LEFT:
			texto1 = "LEFT";
			if (modo == CAMINATAS1) {
				angulo = angulo_offset + PI;
			} else {
				anguloso = 0;
				mov.mon_angulo = &anguloso;       // aca el ángulo offset se usaría para rotar el centro      
				mov.curva (velocidad, desplazamiento, (COORD2D) {0, 0} , CCW, marcha, largo_pasos);
			}
			break;
	
		case RC_RIGHT:
			texto1 = "RIGHT";
			if (modo == CAMINATAS1) {
				angulo = angulo_offset;
			} else {
				anguloso = 0;
				mov.mon_angulo = &anguloso;       // aca el ángulo offset se usaría para rotar el centro 
				mov.curva (velocidad, desplazamiento, (COORD2D) {0, 0} , CW, marcha, largo_pasos);
			}
			break;

		case RC_MENU:
			texto1 = "MENU";
			mov.mon_angulo = NULL;
			mov.curva (velocidad, desplazamiento, centro, CCW, marcha, largo_pasos);
			break;
			
		case RC_EXIT:
			texto1 = "EXIT";
			mov.mon_angulo = NULL;
			mov.curva (velocidad, desplazamiento, centro, CW, marcha, largo_pasos);
			break;
			
		case RC_MTS:
			texto1 = "MTS";
			mov.mon_angulo = &angulo_offset;
			mov.curva (velocidad, desplazamiento, centro, CCW, marcha, largo_pasos);
			break;
			
		case RC_CCTTX:
			mov.mon_angulo = &angulo_offset;
			mov.curva (velocidad, desplazamiento, centro, CW, marcha, largo_pasos);
			texto1 = "CC_TTX";
			break;
			
		case RC_ENTER1:
			texto1 = "STOP";
			mov.stop();
			isMoving = false;
			break;
			
		case RC_VOL_UP:
			if (pantalla.isBusy()) {break;}
			if (!isMoving) {
				velocidad = constrain (velocidad+inc, 1, 40);
				texto1 = "Vel " + float2string (velocidad);
			} else {
				texto1 = "Escala " + String (mov.dec_escala(), DEC);
			}
			retardo = true;
			break;
			
		case RC_VOL_DN:
			if (pantalla.isBusy()) {break;}
			if (!isMoving) {
				velocidad = constrain (velocidad-inc, 1, 40);
				texto1 = "Vel " + float2string (velocidad);
			} else {
				texto1 = "Escala " + String (mov.inc_escala(), DEC);
			}
			retardo = true;
			break;
			
		case RC_CH_UP:
			if (pantalla.isBusy()) {break;}
			if (!isMoving) {largo_pasos = constrain (largo_pasos+inc, 0, 20);}
			texto1 = "Paso "; 
			if (largo_pasos == 0) {texto1 += "AUTO";} else {texto1 += float2string (largo_pasos);}
			retardo = true;
			break;
			
		case RC_CH_DN:
			if (pantalla.isBusy()) {break;}
			if (!isMoving) {largo_pasos = constrain (largo_pasos-inc, 0, 20);}
			texto1 = "Paso "; 
			if (largo_pasos == 0) {texto1 += "AUTO";} else {texto1 += float2string (largo_pasos);}
			retardo = true;
			break;
	}
	
	// switch de ejecución (y puede haber más; talvez la variable swicheada en segunda instancia no sea "comando")
	switch (comando) {
		case RC_UP: case RC_DOWN: case RC_LEFT: case RC_RIGHT:
			if (modo == CAMINATAS1) {mov.mon_desplazamiento = NULL;}
			else if (modo == CAMINATAS2) {
				if (comando == RC_LEFT || comando == RC_RIGHT) {break;}    // la lógica hay que reformularla toda 
				mov.mon_desplazamiento = &centro;
			}
		  mov.recta (velocidad, desplazamiento, angulo, marcha, largo_pasos);
			isMoving = true;
			break;
	}

}

void RemoteControl::traslaciones (byte comando) {
	
	switch (comando) {
		case RC_UP:
			texto1 = "|";
			mov.translation ((COORD3D) {0, 0, largo_pasos}, largo_pasos/(velocidad*TICK));
			break;
		
		case RC_DOWN:
			texto1 = "^";
			mov.translation ((COORD3D) {0, 0, -largo_pasos}, largo_pasos/(velocidad*TICK));
			break;
		
		case RC_LEFT:
			texto1 = "<";
			mov.translation ((COORD3D) {largo_pasos, 0, 0}, largo_pasos/(velocidad*TICK));
			break;
	
		case RC_RIGHT:
			texto1 = ">";
			mov.translation ((COORD3D) {-largo_pasos, 0, 0}, largo_pasos/(velocidad*TICK));
			break;

		case RC_CCTTX: case RC_MTS:
			texto1 = "h-";
			mov.translation ((COORD3D) {0, -largo_pasos, 0}, largo_pasos/(velocidad*TICK));
			break;
			
		case RC_EXIT: case RC_MENU:
			texto1 = "h+";
			mov.translation ((COORD3D) {0, largo_pasos, 0}, largo_pasos/(velocidad*TICK));
			break;
			
		case RC_ENTER1:
			texto1 = "STORE";
			mov.actual_pos_ref ();
			break;
			
		case RC_VOL_UP:
			if (pantalla.isBusy()) {break;}
			velocidad = constrain (velocidad+inc, 1, 40);
			texto1 = "Vel " + float2string (velocidad);
			retardo = true;
			break;
			
		case RC_VOL_DN:
			if (pantalla.isBusy()) {break;}
			velocidad = constrain (velocidad-inc, 1, 40);
			texto1 = "Vel " + float2string (velocidad);
			retardo = true;
			break;
			
		case RC_CH_UP:
			if (pantalla.isBusy()) {break;}
			if (!isMoving) {largo_pasos = constrain (largo_pasos+inc, 0, 20);}
			texto1 = "Paso "; 
			if (largo_pasos == 0) {texto1 += "AUTO";} else {texto1 += float2string (largo_pasos);}
			retardo = true;
			break;
			
		case RC_CH_DN:
			if (pantalla.isBusy()) {break;}
			if (!isMoving) {largo_pasos = constrain (largo_pasos-inc, 0, 20);}
			texto1 = "Paso "; 
			if (largo_pasos == 0) {texto1 += "AUTO";} else {texto1 += float2string (largo_pasos);}
			retardo = true;
			break;
	}

}

void RemoteControl::rotaciones (byte comando) {
	
	switch (comando) {
		case RC_UP:
			texto1 = "|";
			mov.rotation ((COORD3D) {0,0,0}, -largo_pasos/15, 0, 0, largo_pasos/(velocidad*TICK));  // radio = 15
			break;
		
		case RC_DOWN:
			texto1 = "^";
			mov.rotation ((COORD3D) {0,0,0}, largo_pasos/15, 0, 0, largo_pasos/(velocidad*TICK));  // radio = 15
			break;
		
		case RC_RIGHT:
			texto1 = "<";
			mov.rotation ((COORD3D) {0,0,0}, 0, 0, largo_pasos/15, largo_pasos/(velocidad*TICK));  // radio = 15
			break;
	
		case RC_LEFT:
			texto1 = ">";
			mov.rotation ((COORD3D) {0,0,0}, 0, 0, -largo_pasos/15, largo_pasos/(velocidad*TICK));  // radio = 15
			break;

		case RC_MENU: case RC_MTS:
			texto1 = "h-";
			mov.rotation ((COORD3D) {0,0,0}, 0, largo_pasos/15, 0, largo_pasos/(velocidad*TICK));  // radio = 15
			break;
			
		case RC_EXIT: case RC_CCTTX:
			texto1 = "h+";
			mov.rotation ((COORD3D) {0,0,0}, 0, -largo_pasos/15, 0, largo_pasos/(velocidad*TICK));  // radio = 15
			break;
			
		case RC_ENTER1:
			texto1 = "STORE";
			mov.actual_pos_ref ();
			break;
			
		case RC_VOL_UP:
			if (pantalla.isBusy()) {break;}
			velocidad = constrain (velocidad+inc, 1, 40);
			texto1 = "Vel " + float2string (velocidad);
			retardo = true;
			break;
			
		case RC_VOL_DN:
			if (pantalla.isBusy()) {break;}
			velocidad = constrain (velocidad-inc, 1, 40);
			texto1 = "Vel " + float2string (velocidad);
			retardo = true;
			break;
			
		case RC_CH_UP:
			if (pantalla.isBusy()) {break;}
			if (!isMoving) {largo_pasos = constrain (largo_pasos+inc, 0, 20);}
			texto1 = "Paso "; 
			if (largo_pasos == 0) {texto1 += "AUTO";} else {texto1 += float2string (largo_pasos);}
			retardo = true;
			break;
			
		case RC_CH_DN:
			if (pantalla.isBusy()) {break;}
			if (!isMoving) {largo_pasos = constrain (largo_pasos-inc, 0, 20);}
			texto1 = "Paso "; 
			if (largo_pasos == 0) {texto1 += "AUTO";} else {texto1 += float2string (largo_pasos);}
			retardo = true;
			break;
	}

}

String RemoteControl::float2string (float number) {
	int parte_entera = number;
	int parte_decimal = number*10 - parte_entera*10;
	return String (parte_entera, DEC) + "," + String (parte_decimal, DEC);
}

void RemoteControl::displayText () {
	if (!pantalla.isBusy()) {
		pantalla.setColor (LISO, color1, color1);
		pantalla.scrollText (texto1.toCharArray(), 53, retardo*6);
	}
	retardo = false;
}