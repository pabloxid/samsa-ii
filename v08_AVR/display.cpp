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

// display.cpp
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

#include "wiring.h"
#include "display.h"
#include "pins_arduino.h"
#include "hardware.h"
#include <string.h>

Display pantalla;                  // preinstanciado

Display::Display () {               // constructor
  text = NULL;
  mode = DISPLAY_OFF;
}

// screen manipulation
void Display::cls () {
  SPIsendMsg (0, 0, NULL);
}

void Display::borraZona (byte col_from, byte col_to, byte fil_from, byte fil_to) {
  SPIsendMsg (1, 2, (byte[]) {xy2byte(col_from, col_to), xy2byte(fil_from, fil_to)});
}

void Display::borraCuadrante (byte cuadrante) {
  SPIsendMsg (2, 1, (byte[]) {cuadrante});
}

void Display::setColor (byte colorMode, byte colorMin, byte colorMax) {
  SPIsendMsg (3, 3, (byte[]) {colorMode, colorMin, colorMax});
}

void Display::rePaintScreen () {
  SPIsendMsg (4, 0, NULL);
}

void Display::invertScreen () {
  SPIsendMsg (5, 0, NULL);
}    
    
// pixel manipulation
void Display::writePixel (byte columna, byte fila, byte color) {                        
  SPIsendMsg (6, 2, (byte[]) {xy2byte(columna, fila), color});    
}

byte Display::readPixel (byte columna, byte fila) {
  return SPIsendMsg (7, 1, (byte[]) {xy2byte(columna, fila)});       // esto no está implementado en el firmware del display
}

void Display::setPixel (byte columna, byte fila) {
  SPIsendMsg (8, 1, (byte[]) {xy2byte(columna, fila)});
}

void Display::resetPixel (byte columna, byte fila) {
  SPIsendMsg (9, 1, (byte[]) {xy2byte(columna, fila)});
}

void Display::togglePixel (byte columna, byte fila) {
  SPIsendMsg (10, 1, (byte[]) {xy2byte(columna, fila)});
}    

// graphic primitives (shapes & animations)
void Display::linea_v (byte columna, bool set) {
  SPIsendMsg (11, 1, (byte[]) {xy2byte(columna, set)});
}

void Display::linea_h (byte fila, bool set) {
  SPIsendMsg (12, 1, (byte[]) {xy2byte(fila, set)});
}

void Display::linea (byte x0, byte y0, byte x1, byte y1) {
  SPIsendMsg (13, 2, (byte[]) {xy2byte(x0, y0), xy2byte(x1, y1)});
}

void Display::cuadrado (byte x, byte y, byte lado) {
  SPIsendMsg (14, 2, (byte[]) {xy2byte(x, y), lado});    
}

void Display::circulo (byte x, byte y, byte diametro) {
  SPIsendMsg (15, 2, (byte[]) {xy2byte(x, y), diametro});
}

void Display::conway () {
  SPIsendMsg (16, 0, NULL);
}

void Display::conway (int pausa_, int ciclos) {
  pausa = pausa_;
  index = ciclos;
  retardo = 0;
  mode = CONWAY;
}

void Display::medidor (byte shape, byte valor) {
  SPIsendMsg (17, 2, (byte[]) {shape, valor});
}    

// buffer primitives (cut-copy-paste)
void Display::readScreen () {
  SPIsendMsg (18, 0, NULL);
}

void Display::writeScreen () {                                 
  SPIsendMsg (19, 0, NULL);
}

void Display::merge () {
  SPIsendMsg (20, 0, NULL);
}

// buffer advanced (fx, text, etc) 
void Display::loadChr (byte chr) {                                                          
  SPIsendMsg (21, 1, (byte[]) {chr});
}

void Display::scroll (char desplazamientoH, char desplazamientoV, bool circular, bool isRW) {
  SPIsendMsg (22, 3, (byte[]) {10+desplazamientoH, 10+desplazamientoV, circular + 2*isRW});
}

void Display::rotaciones (byte modo) {
  SPIsendMsg (23, 1, (byte[]) {modo});
}

void Display::lsm (char chr, char sc) {                                       // "load - scroll - merge"
  SPIsendMsg (24, 2, (byte[]) {chr, sc+10});
}

// rutinas de alto nivel (locales)
void Display::scrollText (const char *text_, int pausa_, int retardo_, char sentido_, int index_) {  // presenta un texto en movimiento (al estilo cutcsa) 
																																													            // sentido: 0 manual; 1 izquierda; -1 derecha
                                                                                    // index es la letra actual. Al comienzo debe ser -1. 
																																					                         // pausa en milisegundos | reatardo en 'pausas'
  if (text != NULL) {
    free (text);
  }
  text = (char*) malloc (strlen(text_)+1);
  strcpy (text, text_);
  pausa = pausa_;
  retardo = retardo_;
  sentido = sentido_;
  index = index_;
  mode = SCROLL_TEXT;
  init = true;
}

void Display::lsm_int (char index_, char sc_) {                  // "load - scroll - merge" (interna)
  if (index_ < strlen(text) && index_ >= 0) {
    lsm (text[index_], sc_);
  } else {lsm (' ', sc_);} 
}

// rutinas auxiliares
byte Display::ancho (char chr) {                                     // tabla de anchos de los caracteres
  byte anch;
  switch (chr) {
    case 'E': case 'F': case 'H': case 'J': case 'K': case 'L': case 'P': case 'Z':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'g': case 'h': case 'k': 
    case 'n': case 'o': case 'p': case 'q': case 's': case 'u': case 'y': case 'z': 
    case '!': case '&': case '-': case '0': case '2': case '3': case '4': case '5': 
    case '6': case '7': case '8': case '9': case '=': case '?': case '{': case '}': anch=5; break;
    case 'f': case 'r': case 't': case '/': anch=4; break;
    case 'I': case 'i': case 'l': case ' ': anch=2; break;
    case 'j': case '(': case ')': case ',': case '.': case '1': case ':': case ';': anch=3; break;
    case '%': anch=7; break;       
    case '@': case '*': case 'W': case '~': case '<': case '>': case '[': case ']': case '^': case '_': case '|': anch=8; break;
    default: anch=6; break;
  }
  return anch;
}

byte Display::offset (char chr) {                                          // tabla de "offsets" de los caracteres
  byte offs;
  switch (chr) {
    case 'E': case 'F': case 'H': case 'J': case 'K': case 'L': case 'P': case 'Z':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'g': case 'h': case 'k': 
    case 'n': case 'o': case 'p': case 'q': case 's': case 'u': case 'y': case 'z': 
    case '!': case '&': case '-': case '0': case '2': case '3': case '4': case '5': 
    case '6': case '7': case '8': case '9': case '=': case '?': case '{': case '}':
    case 'f': case 'r': case 't': case '/': case '"': offs=1; break;
    case 'j': case '(': case ')': case ',': case '.': case '1': case ':': case ';': 
    case 'I': case 'i': case 'l': case ' ': offs=2; break;
    default: offs=0; break;
  }
  return offs;
}

byte Display::SPIsendMsg (byte instruccion, byte largo, byte* data) {        // manda los mensajes via SPI al display
  digitalWrite(SS, LOW);
  SPItransfer (128+(instruccion<<2)+largo);             // byte de cabecera, con 5 bits de instrucción y 2 bits de largo (no tiene "running status")
  for (byte i=0; i<largo; i++) {
    SPItransfer (data[i]);
  }  
  digitalWrite(SS, HIGH);
  return 0;                                       // falta implementar la recepción
}
  
byte Display::xy2byte (byte x, byte y) {        // codifica las coordenadas como 1 solo byte
  return y + 8*x;
}

void Display::update (unsigned long milis) {
	
	// variables de control
	static char sc;                    // sc es la posición actual de la letra
	static unsigned long timer = 0;
	
	if (milis-timer < pausa) {return;}    // verifica si pasó x tiempo desde la última vez, y si no, retorna sin hacer nada
			
	timer = milis;
	
	if (retardo > 0) {retardo--; return;}   // retardo
	
	if (init) {            // init es el flag que indica que se deben actualizar las variables de control 
		sc = 0; 
		init = false;
	}
	
	// empieza el loop
	switch (mode) {
		case DISPLAY_OFF: return;
		case SCROLL_TEXT: {
			sc -= sentido;                                           
			if (sc == ancho(text[index-1])) {index--; sc=0;} 
			else if (sc < 0) {index++; sc=ancho(text[index-1])-1;}              
			scroll (sentido, 0, 0, true);                               
			switch (sentido) {
				case 1:  
					lsm_int (index, offset(text[index])-sc);
					if (sc + ancho(text[index]) < 7) {lsm_int (index+1, offset(text[index+1])-ancho(text[index])-sc);}
					break;      
				case -1:
					lsm_int (index-1, offset(text[index-1])+ancho(text[index-1])-sc);
					break;      
			}
			if (index >= strlen(text) || index < 0) {mode = DISPLAY_OFF;}
			break;
		}
		case CONWAY: {
			conway ();
			index --;
			if (index == 0) {mode = DISPLAY_OFF;}
			break;
		}
	}
}

bool Display::isBusy () {
	return (mode == SCROLL_TEXT && retardo == 0);
}