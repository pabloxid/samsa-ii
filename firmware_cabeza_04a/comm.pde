
void leer_serial () {                  // esta es la "máquina de estados" que recibe y decodifica los mensajes Seriales 
  
  // variables locales estáticas
  static byte largo, instruccion, cont;
  static byte data [3];                               // el largo de los mensajes puede ser de hasta 3 bytes               
     
  if (Serial.available() > 0) {                      // si hay al menos 1 byte en el Serial...
    
    byte b = Serial.read();                             // ...lo lee
    
    if (b >= 224) {                                   // ...si es un HEAD...
      instruccion = (b-224)&7;                          // hay 8 tipos de mensajes (para cada largo) 
      largo = (b-224)>>3;                               // el largo de la data puede ser hasta 3 bytes
      cont = 0;                                         // inicializa el índice del buffer                            
    } else {                                          // ...de lo contrario (si es un DATA)...
      if (cont < largo) {data[cont++] = b;}             // va llenando el buffer
    }
    
    if (cont == largo) {                            // cuando termina el cuerpo del mensaje
      execute (instruccion, largo, data);           // lo ejecuta                       
      cont = 0;                                     // y resetea cont, lo que produce un "running status" (el propio running status)
    }
  } 
}

void execute (byte instruccion, byte largo, byte* data) {
   
   // 0-7 requests
   // 8-15 setea umbrales (incluido el send_all on/off)  
   // 16-31 reservadas para futuras cosas

  switch (instruccion + 8*largo) {
    case 0: case 1: case 2: case 3: case 4:                                // 1-byte requests
      {byte value = (*compute_values[instruccion]) ();                     // calcula un valor fresco
      send_msg (instruccion, 1, &value);                                  
      break;}
    
    case 5: {                                                              // 2-byte request (distancia en mm)
      send_2byte_msg (0, compute_mm());                                    // calcula un valor fresco 
      break; 
    }
    
    case 6: {                                     // "fast" distance
      byte value = dist.read(0) / 7;
      send_msg (instruccion-1, 1, &value);
      break;
    }
    
    case 7: {                                     // "scan"
      byte value = minimo (&dist, 0, 31);
      send_msg (instruccion-1, 1, &value);
      break;
    }
      
    case 8: case 9: case 10: case 11: case 12:                  // 1-byte threshold setters 
      umbrales [instruccion] = data [0];
      break;
      
    case 13:                                                    // pause setter
      pausa = data [0];
      break;      
      
    case 16:                                                    // 2-byte threshold setter 
      umbrales[5] = twobyte2int (data);
      break;
      
    case 15:                                                    // enable/disable send all 
      send_all = data [0];
      break;  
  } 
    
}

void send_msg (byte instruccion, byte largo, byte* data) {
  volatile static byte last = 0;
  cli();
  byte head = 224 + instruccion + 8*largo;                       // esto es para el running status
  if (head != last || largo == 0) { 
    Serial.write (head);                                         // byte de cabecera (la instrucción no incluye el largo)
  }
  Serial.write (data, largo);
  last = head;
  sei();
}

int twobyte2int (byte* data) {          // transforma 2 bytes de 7 bits en un int
  return 128*data[0] + data[1];
} 

void send_2byte_msg (byte instruccion, unsigned int value) {
  send_msg (instruccion, 2, (byte[]) {value>>7, value&127});
}
