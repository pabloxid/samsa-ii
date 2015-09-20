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

// display.h
// version: 0.8
// date: 7/2/2011
// authors: Pablo Gindel, Jorge Visca

/* TO DO DISPLAY:
extender la display con más cosas dinámicas
  - triángulos
  - animación con líneas encadenadas (nibbles, de longitud variable y multicolores)
  - color scroll, plasma, línea que se mueve, objeto que se mueve, caracter que se mueve, metaballs
  - scroll, rotaciones, etc. en versión animada
  - cambio de modo de color y de color mismo, automáticos
  - la medidor, en versión animada (usar punteros a las variables de la caminata, o viceversa)
  - la display tendría una función "monitorear x" y se le pasa un puntero a algo
  - por ejemplo, un "piano roll" de patas apoyadas, el "gait monitor", el load monitor
  - mostrar textos en más de 1 color
*/


#ifndef DISPLAY_H
#define DISPLAY_H

typedef unsigned char byte;

// clase para manejar remotamente (via SPI) el display LED 8x8 RGB, con el firmware "inteligente"  

class Display {
  
  public:

    Display ();
    
    // screen manipulation
    void cls ();
    void borraZona (byte x_from, byte x_to, byte y_from, byte y_to);
    void borraCuadrante (byte cuadrante);
    void setColor (byte colorMode_, byte colorMin_, byte colorMax_);
    void rePaintScreen ();
    void invertScreen ();
    
    // pixel manipulation
    void writePixel (byte x, byte y, byte color);
    byte readPixel (byte x, byte y);
    void setPixel (byte x, byte y);
    void resetPixel (byte x, byte y);
    void togglePixel (byte x, byte y);
    
    // graphic primitives (shapes & animations)
    void linea_v (byte x, bool set);
    void linea_h (byte y, bool set);
    void linea (byte x0, byte y0, byte x1, byte y1);
    void cuadrado (byte x, byte y, byte lado);
    void circulo (byte x, byte y, byte diametro);
    void conway ();
    void conway (int pausa_, int ciclos);
    void medidor (byte shape, byte valor);  // sahpe 0-7, valor 0-8
    
    // buffer primitives (cut-copy-paste)
    void readScreen ();
    void writeScreen ();
    void merge ();
    
    // buffer advanced (fx, text, etc) 
    void loadChr (byte chr);
    void scroll (char desplazamientoH, char desplazamientoV, bool circular, bool isRW);
    void rotaciones (byte modo);
      
    // rutinas de alto nivel (locales)
    void scrollText (const char *text_, int pausa_=60, int retardo_=0, char sentido_=1, int index_=-1);
    void lsm (char chr, char sc);  
    
    // rutina universal para que camine la cosa
    void update (unsigned long milis);
		
    // rutinas de control
		 bool isBusy ();
		
  private:
    // rutinas auxiliares
    void lsm_int (char index_, char sc_);
    byte ancho (char chr);
    byte offset (char chr);
    byte SPIsendMsg (byte instruccion, byte largo, byte* data);
    byte xy2byte (byte x, byte y);
		
    // parámetros
		 char *text;
		 int pausa;
		 char sentido;
		 int index;
		 
		 // variables de control
    bool init;
    byte mode;
    int retardo;
    
};

extern Display pantalla;

#define RGB(r,g,b) (r<<4)+(g<<2)+b     // macro para el color en formato RGB

// modos de color
#define LISO         0                
#define DEGRADE_V    1
#define DEGRADE_H    2

// macros de color

// modos de la 'medidor' (y de la biblioteca misma)
enum {BARRA_H, BARRA_V, OSC_H, OSC_V, OSC_RADIAL, ESCALERA, ALEATORIO, TEXTO, SCROLL_TEXT, CONWAY, DISPLAY_OFF};	

// definir modos de rotación


#endif