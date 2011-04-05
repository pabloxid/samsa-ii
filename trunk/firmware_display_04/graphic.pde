////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Rutinas Auxiliares ////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

// Used for make the PWM scale logaritmic
byte lin2log (byte in) {
  switch (in & 0x03) {
    case 0: return 0;
    case 1: return 3;
    case 2: return 9;
    case 3: return 16;
  }
}

// ...a la inversa...
byte log2lin (byte in) {
  switch (in) {
    case 0: return 0;
    case 3: return 1;
    case 9: return 2;
    case 16: return 3;
  }
}

// calcula el color de cada pixel segun su posición
byte getColor (byte columna, byte fila) {
  switch (colorMode) {
    case LISO: return colorMap [0];
    case DEGRADE_V: return colorMap [columna];
    case DEGRADE_H: return colorMap [fila];   
  }
}

// setea el color de un pixel en pantalla
void writePixel (byte npixel, byte color) {
  if (npixel>63) {return;}
  // Use the PWM variant (Byte format: 00RRGGBB)
  image_red [npixel] = lin2log ((color >> 4));
  image_green [npixel] = lin2log ((color >> 2));
  image_blue [npixel] = lin2log ((color)); 
}

// devuelve el valor de un pixel en pantalla
byte readPixel (byte npixel) {
  return (log2lin (image_red [npixel])<<4) 
    + (log2lin (image_green [npixel])<<2) + log2lin (image_blue [npixel]);  
}

// devuelve si el pixel está encendido o apagado
boolean isSet (byte columna, byte fila) {
  return (readPixel (columna, fila) != 0);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Graphic Library Propiamente Dicha //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// screen manipulation

void cls () {                                                                  // borra la pantalla
  for (byte i = 0; i < 64; i++) { 
    image_red[i] = 0; 
    image_green[i] = 0;
    image_blue[i] =  0;
  } 
}

void borraZona (byte col_from, byte col_to, byte fil_from, byte fil_to) {          // borra un sector de la pantalla
  for (byte columna=col_from; columna<=col_to; columna++) {
    for (byte fila=fil_from; fila<=fil_to; fila++) {
      resetPixel (columna, fila);
    }
  }
} 
  
void borraCuadrante (byte cuadrante) {                                     // borra uno de los 4 cuadrantes en que se divide la pantalla
  switch (cuadrante) {
    case 1: borraZona (4, 7, 0, 3); break;
    case 0: borraZona (4, 7, 4, 7); break;
    case 3: borraZona (0, 3, 4, 7); break;
    case 2: borraZona (0, 3, 0, 3); break;
  }
}

void setColor (byte colorMode_, byte colorMin_, byte colorMax_) {              // setea el modo y el color
                                                                              // colorMode: 0=liso; 1=degradé vertical; 2=degradé horizontal 
   colorMode = colorMode_;
   byte colorRMin = R(colorMin_);
   byte colorGMin = G(colorMin_);
   byte colorBMin = B(colorMin_);
   float colorRcoef = colorMixer (colorRMin, R(colorMax_));
   float colorGcoef = colorMixer (colorGMin, G(colorMax_));
   float colorBcoef = colorMixer (colorBMin, B(colorMax_));
   for (byte var=0; var<8; var++) {
     byte r = colorRMin + .5 + var*colorRcoef;
     byte g = colorGMin + .5 + var*colorGcoef;
     byte b = colorBMin + .5 + var*colorBcoef;
     colorMap [var] = RGB(r, g, b);
   }
}

void rePaintScreen () {                                                    // pinta todos los puntos activos
  for (byte columna=0; columna<8; columna++) { 
    for (byte fila=0; fila<8; fila++) { 
      if (readPixel (columna, fila)) {setPixel (columna, fila);}
    }
  }
}

void invertScreen () {                                                  // togglea todos los puntos
  for (byte columna=0; columna<8; columna++) { 
    for (byte fila=0; fila<8; fila++) { 
      togglePixel (columna, fila);
    }
  }
}

// pixel manipulation

void writePixel (byte columna, byte fila, byte color) {                      // setea el color de un pixel en pantalla                       
  writePixel (fila + 8*columna, color);
}

byte readPixel (byte columna, byte fila) {                                    // devuelve el valor de un pixel en pantalla                                  
  return readPixel (fila + 8*columna);
}

void setPixel (byte columna, byte fila) {                                             // prende un pixel en pantalla
  writePixel (fila + 8*columna, getColor (columna, fila));
}

void resetPixel (byte columna, byte fila) {                                           // apaga un pixel en pantalla
  writePixel (fila + 8*columna, 0);
}

void togglePixel (byte columna, byte fila) {                                         // cambia el estado de un pixel en pantalla
  if (!readPixel (columna, fila)) {setPixel (columna, fila);}  
}

// graphic primitives (shapes & animations)

void linea_v (byte columna, boolean set) {                                          // dibuja una línea vertical, sin afectar el resto de la pantalla
  for (byte fila=0; fila<8; fila++) {
    if (set) {setPixel (columna, fila);} else {resetPixel (columna, fila);} 
  }
}
  
void linea_h (byte fila, boolean set) {                                             // dibuja una línea horizontal, sin afectar el resto de la pantalla
  for (byte columna=0; columna<8; columna++) {
    if (set) {setPixel (columna, fila);} else {resetPixel (columna, fila);} 
  }
}
  
void linea (byte x0, byte y0, byte x1, byte y1) {                                   // dibuja una línea con el algoritmo de Bresenham
  char dx, dy, p, incE, incNE, stepx, stepy;
  byte x, y;
  dx = (x1 - x0); dy = (y1 - y0);
  
  // determinar qué punto usar para empezar, cuál para terminar
  if (dy < 0) {dy = -dy; stepy = -1;} else {stepy = 1;}
  if (dx < 0) {dx = -dx; stepx = -1;} else {stepx = 1;}
 
  x = x0; y = y0; setPixel (x, y);
  
  // se cicla hasta llegar al extremo de la línea
  if (dx > dy) {
    incE = 2*dy; incNE = 2*(dy-dx); p = incE - dx;
    while (x != x1) {
      x += stepx;
      if (p < 0) {p += incE;} else {y += stepy; p += incNE;}
      setPixel (x, y);
    }
  } else {
    incE = 2*dx; incNE = 2*(dx-dy); p = incE - dy; 
    while (y != y1) {
      y += stepy;
      if (p < 0) {p += incE;} else {x += stepx; p += incNE;}
      setPixel (x, y);
    }
  }
}

void cuadrado (byte x, byte y, byte lado) {                              // dibuja un cuadrado relleno
  byte from = lado/2; byte to = lado - from;
  for (int columna=x-from; columna<=x+to; columna++) {
    for (int fila=y-from; fila<=y+to; fila++) {
      if (columna>=0 && fila>=0) {setPixel (columna, fila);}
    }
  } 
}

void circulo (byte x, byte y, byte diametro) {                                      // dibuja un circulo sin relleno
  for (byte fase=0; fase<16; fase++) {
    setPixel (x+diametro*seno[fase], y+diametro*seno[(fase+4)%16]);
  } 
}

void conway () {                                                                  // procesa la pantalla con e1 juego de la vida (1 ciclo)
  for (byte columna=0; columna<8; columna++) { 
    for (byte fila=0; fila<8; fila++) { 
      int vecinos = 0 - isSet (columna, fila);
      for (int c=columna-1; c<=columna+1; c++) {
        for (int f=fila-1; f<=fila+1; f++) { 
          vecinos += isSet ((c+8)%8, (f+8)%8);
        }
      }
      if (vecinos == 3) {
        buffer[columna][fila] = getColor (columna, fila);
      } else if (vecinos != 2) {
        buffer[columna][fila] = 0;
      }
    } 
  }
  writeScreen (); 
}

void medidor (byte shape, byte valor) {     
  // shape: 0-1 barras, 2-3 osciloscopio lineal, 4 osciloscopio circular, 5 escalerita, 6 aleatorio-probabilístico, 7 texto
  // otras ideas para shape: cuadrados, osciloscopios "rellenos", algo con las simetrías y los desplazamientos
  // rango de valores 0-8
  
  static byte fase = 0;
  
  switch (shape) {
    case 0: case 1:                                                                             // barra horizontal y vertical
      if (valor == 0) {cls();} else {
        byte base = (8-valor)/2;
        for (byte index=0; index<8; index++) { 
          boolean condition = index>=base && index<base+valor;
          if (shape) {linea_v (index, condition);} else {linea_h (index, condition);}
        }
      }
      break;
    case 2:                                                                                     // osciloscopio horizontal
      linea_v (fase, 0);
      if (valor != 0) {setPixel (fase, valor-1);} 
      fase = (fase+1) % 8;
      break;
    case 3:                                                                                     // osciloscopio vertical
      linea_h (fase, 0);
      if (valor != 0) {setPixel (valor-1, fase);} 
      fase = (fase+1) % 8;
      break; 
    case 4:                                                                                     // osciloscopio radial
      if (fase%4 == 0) {borraCuadrante (fase/4);} 
      if (valor != 0) {setPixel (4+valor*seno[fase], 4+valor*seno[(fase+4)%16]);}
      fase = (fase+1) % 16;
      break; 
    case 5:                                                                                     // escalerita
      if (valor == 0) {cls();} else {
        for (byte columna=0; columna<8; columna++) { 
          if (columna<valor) {
            for (byte fila=0; fila<=columna; fila++) {
              setPixel (columna, fila);
            }
          } else {linea_v (columna, 0);}
        }
      }  
      break;
    case 6:                                                                                     // aleatorio-probabilístico 
      if (valor == 0) {cls();} else {
        if (random(8)<valor) {togglePixel (random(8), random(8));}  
      }
      break; 
   case 7:                                                                                      // texto
     loadChr ('0'+valor);
     writeScreen();    
  } 
}

// buffer primitives (cut-copy-paste)

void readScreen () {                                                                // pantalla --> buffer
  for (byte columna=0; columna<8; columna++) {
    for (byte fila=0; fila<8; fila++) {
      buffer [columna][fila] = readPixel (columna, fila);
    }
  }
}

void writeScreen () {                                                               // buffer --> pantalla
  for (byte columna=0; columna<8; columna++) {
    for (byte fila=0; fila<8; fila++) {
      writePixel (columna, fila, buffer [columna][fila]);
    }
  }
}

void merge () {                                                                     // buffer + pantalla --> pantalla
  for (byte columna=0; columna<8; columna++) {
    for (byte fila=0; fila<8; fila++) {
      if (buffer [columna][fila]) {writePixel (columna, fila, buffer [columna][fila]);}
    }
  }
}

// buffer advanced (fx, text, etc) 

void loadChr (byte chr) {                                                // carga un carácter en el buffer (texto)
  chr -= 32;                                                                            
  for (byte index=0; index<8; index++) {
    byte columna = pgm_read_byte (&caracter[chr][index]);                        // (los caracteres están en la flash)
    for (byte fila=0; fila<8; fila++) {
      buffer [index][fila] = getColor (index, fila)*bitRead (columna, fila);
    }
  }
}

void scroll (char desplazamientoH, char desplazamientoV, boolean circular, boolean isRW) {         // desplaza buffer
  if (isRW) {readScreen ();}
  byte buffer2 [8][8];
  byte columna, fila;
  for (columna=0; columna<8; columna++) {
    for (fila=0; fila<8; fila++) {
      char sourceC = columna+desplazamientoH;
      char sourceF = fila+desplazamientoV;
      if (sourceC>=0 && sourceC<=7 && sourceF>=0 && sourceF<=7) {   
        buffer2 [columna][fila] = buffer [sourceC][sourceF];
      } else {
        if (circular) {
          buffer2 [columna][fila] = buffer [(sourceC+8)%8][(sourceF+8)%8];
        } else {buffer2 [columna][fila] = 0;}
      }
    }
  }
  memcpy  (buffer, buffer2, 64);
  if (isRW) {writeScreen ();}
}

void rotaciones (byte modo) {                                           // transformación en el buffer
                                                                        // modo = 00000RVH; R=rotate, V=flip vertical; H=flip horizontal
  boolean flipH = modo&1;
  boolean flipV = (modo>>1)&1;
  boolean rotate = (modo>>2)&1;
  byte buffer2 [8][8];
  byte columna, fila;
  byte sourceC, sourceF;
  for (columna=0; columna<8; columna++) {
    for (fila=0; fila<8; fila++) {
      if (flipH) {sourceF = 7-fila;} else {sourceF = fila;}
      if (flipV) {sourceC = 7-columna;} else {sourceC = columna;}
      if (rotate) {byte temp = sourceF; sourceF = sourceC; sourceC = temp;}
      buffer2 [columna][fila] = buffer [sourceC][sourceF];
    } 
  }
  memcpy  (buffer, buffer2, 64);
} 

void lsm (char chr, char sc) {                // "loadChr - scroll - merge"
  loadChr (chr);                            
  scroll (sc, 0, 0, false);                        
  merge (); 
}

float colorMixer (int Min, int Max) {
  return (Max-Min)/7.0; 
}

