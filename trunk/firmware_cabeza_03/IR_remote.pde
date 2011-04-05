
#define TIMEOUT 3000

void detect() {                                                  // interpreta el comando del control remoto
  int i = midtiemp (0);                                          // más de 1200
  int j = midtiemp (1);                                          // más de 600
  if (i < TIMEOUT && j < TIMEOUT && i > 1200 && j > 600) {       // tiempos del protocolo
    byte comando = 0;
    // cabecera
    for (i=1; i<=16; i++){
      midtiemp (0);  
      midtiemp (1);
    }  
    // byte    
    for (i=1; i<=128; i<<=1){
      midtiemp (0);  
      comando += (midtiemp (1) > 160) * i;            // tiempos del protocolo
    }
    // final
    for (i=1; i<=8; i++){
      midtiemp (0);  
      midtiemp (1);
    }
    midtiemp (0);  // normal
    send_msg (7, 1, &comando);   
  }
}

int midtiemp (boolean estado) {                                   // mide la duración del pulso [control remoto] en microsegundos
  digitalWrite(5, estado);
  int durac = 0;
  while (infrared==estado && (durac < TIMEOUT)) {
    durac++; 
    delayMicroseconds (2);
    infrared = digitalRead(2);
  }
  return durac;
}

