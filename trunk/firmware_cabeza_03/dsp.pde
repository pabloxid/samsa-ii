
void mov_avg_filter (RingBuffer* origen, RingBuffer* destino, byte puntos) {
  int filter = destino->read(0) - origen->read(puntos) + origen->read(0);
  destino->store (filter);
}
  
char peak_detector (RingBuffer* buffer, byte puntos) {
  char direc = 0;
  byte pico1 = max_abs (buffer, 0, puntos/2);                                       // 'pico1' es la posición del máximo absoluto en la primera mitad
  byte pico2 = max_abs (buffer, puntos/2, puntos);                                  // 'pico2' es la posición del máximo absoluto en la segunda mitad
  if (sign (buffer->read(pico1)) == -sign (buffer->read(pico2))) {
    direc = buffer->read(pico2)-buffer->read(pico1);
  }
  return direc;                 
}

byte max_abs (RingBuffer* buffer, byte from, byte to) {                             // devuelve la posición del máximo absoluto en el rango
  byte mabs = from;
  for (byte i = from + 1; i < to; i++){                                                                   
    if (abs (buffer->read(i)) > abs (buffer->read(mabs))) {mabs = i;}
  }
  return mabs;
}

float blender (float val1, float val2, float blend) {
  return val1*blend + val2*(1-blend); 
}

float promedio (RingBuffer* buffer, byte puntos) {
  float prom = 0;
  for (byte f=0; f<puntos; f++) {
    prom += buffer->read(f);
  }
  return prom/puntos; 
}

char sign (int numero) {
  char signo = 0;
  if (numero > 0) {signo = 1;}
  if (numero < 0) {signo = -1;}
  return signo;
}

int gate (int valor, byte umbral) {                                                            // un buen squelch
  if (abs (valor) <= umbral) {valor = 0;}
  return valor;
}
 
void sample () {                                    // adquiere la información 
  
  static byte ciclo = 0;
  static int izq_, der_; 
  
  sei();                    // atención con esto, es para permitir las interrupciones del IR-remote, pero
                            // puede traer resultados inesperados
                            // revisar bien el caso en que la interrupción caiga en medio de un mensaje
                            // y revisar el tema del running status
  
  switch (ciclo) {
    
    case 0: {
      ADCInitConv (0);
      float palmer = 42192.5/(flt_der.read (0) + flt_izq.read (0) + 2036);    // la misma fórmula funciono para el sensor "palmer"
      float sharp = 4877.42/(max(ADCReadConv(),272)-240.0)-1.29;              // fórmula calculada con el "calibrate sharp" de processing (limitado a 150 cm)
      // rutina de transición entre los 2 sensores
      float distancia;
      if (palmer > 11) {distancia = sharp;} 
      else if (palmer > 9 && palmer <= 11) {distancia = blender (palmer, sharp, (11-palmer)/2);} 
      else {distancia = palmer;}     
      ADCInitConv (1);
      dist.store (7*distancia + .5);                                         // estamos multiplicando por 7 
      izq_ = ADCReadConv();
      ADCInitConv (2);
      mov_avg_filter (&dist, &flt_dist, 30);                                 // esto tiene el efecto de multiplicar nuevamente por 30 
      der_ = ADCReadConv();
      digitalWrite (10, LOW);
      break;
    }
      
    case 1:
      ADCInitConv (1);
      float vel = (flt_dist.read (25) - flt_dist.read (0)) / 21;             // esto da la la velocidad en cm/s
      izq.store (ADCReadConv()-izq_);
      ADCInitConv (2);
      // cálculo de la velocidad   
      float dist_media = promedio (&flt_dist, 25) / 210;                     // distancia media en centímetros
      velocidad.store (vel*10/dist_media + .5); 
      der.store (ADCReadConv()-der_);
      digitalWrite (10, HIGH);
      mov_avg_filter (&izq, &flt_izq, 28);                                   // esto tiene el efecto de multiplicar por 28
      mov_avg_filter (&der, &flt_der, 28);                                   // esto tiene el efecto de multiplicar por 28
      // cálculo de la lateralidad
      lateralidad.store(gate((flt_der.read(0)-flt_izq.read(0))/28, 5));
      // peak detection
      event_vel = peak_detector (&velocidad, 30);
      event_lat = peak_detector (&lateralidad, 50);
      break; 

  }

  ciclo ++;
  ciclo &= 1;

}

