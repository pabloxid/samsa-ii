
/*** SPI Interrupt routine, Byte recieved ***/
ISR (SIG_SPI)  {
  
  // Used for temporary storing SPI Data register
  static byte spiTemp;
  
  spiTemp = SPDR;     // read spi data register
  
  /* Write SPI Data register */
  // SPDR = lo que se quiere transmitir
    
  store_char (spiTemp, &rx_buffer);   // lo storea en el buffer de mensajes 
  
}

void leer_serial () {                  // esta es la "máquina de estados" que recibe y decodifica los mensajes Seriales 
  
  // variables locales estáticas
  static byte largo, instruccion, cont;
  static byte data [8];                               // el largo de los mensajes puede ser de hasta 8 bytes               
     
  if (comm.available() > 0) {                      // si hay al menos 1 byte en el Serial...
    
    byte b = comm.read();                             // ...lo lee
    
    if (b >= 128) {                                   // ...si es un HEAD...
      largo = b&3;                                      // el largo de la data puede ser hasta 3 bytes
      instruccion = (b>>2)&31;                          // hay 32 tipos de mensajes (para cada largo)
      cont = 0;                                         // inicializa el índice del buffer                            
    } else {                                          // ...de lo contrario (si es un DATA)...
      if (cont < largo) {data[cont++] = b;}             // va llenando el buffer
    }
    
    if (cont == largo) {                            // cuando termina el cuerpo del mensaje
      execute (instruccion, largo, data);           // lo ejecuta                       
      cont ++;                                      // y deshabilita tanto la ejecución como el llenado de buffer (no tiene 'running status')
    }
  } 

}

void execute (byte instruccion, byte largo, byte* data) {
  
  struct COORD c1, c2;
  if (largo > 0) {c1 = coord(data[0]);}
  if (largo > 1) {c2 = coord(data[1]);}
    
  switch (instruccion) {
    
    case 0:
      cls ();
      break;
    
    case 1: 
      borraZona (c1.x, c1.y, c2.x, c2.y);
      break;
      
    case 2:
      borraCuadrante (data[0]);
      break;
      
    case 3:
      setColor (data[0], data[1], data[2]);
      break;
      
    case 4:
      rePaintScreen ();
      break;
      
    case 5:
      invertScreen ();
      break;
      
    case 6:  
      writePixel (data[0], data[1]);
      break;
      
    /*case 7:
      readPixel (data[0]);
      break;*/
      
    case 8: 
      setPixel (c1.x, c1.y);
      break;
    
    case 9: 
      resetPixel (c1.x, c1.y);
      break;
    
    case 10: 
      togglePixel (c1.x, c1.y);
      break;
    
    case 11: 
      linea_v (c1.x, c1.y);
      break;
    
    case 12: 
      linea_h (c1.x, c1.y);
      break;
   
    case 13:
      linea (c1.x, c1.y, c2.x, c2.y);
      break;
        
    case 14:   
      cuadrado (c1.x, c1.y, data[1]);
      break;
    
    case 15:    
      circulo (c1.x, c1.y, data[1]);
      break;
    
    case 16: 
      conway ();
      break;
    
    case 17:
      medidor (data[0], data[1]);
      break;
      
    case 18:  
      readScreen ();
      break;
      
    case 19:  
      writeScreen ();
      break;
      
    case 20:  
      merge ();
      break;
    
    case 21:
      loadChr (data[0]);
      break;
      
    case 22:
      scroll (data[0]-10, data[1]-10, data[2]&1, (data[2]>>1)&1);
      break;
      
    case 23:
      rotaciones (data[0]);
      break;
      
    case 24:
      lsm (data[0], data [1]-10);
      break;   
  }    
}

struct COORD coord (byte n) {             // decodifica las coordenadas enviadas como 1 solo byte
  COORD c;
  c.x = (n>>3)&7;
  c.y = n&7;
  return c;
}
