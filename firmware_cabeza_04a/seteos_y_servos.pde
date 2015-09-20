

// rutinas del ADC manual

void ADCconfig () {
  /* configureta del analog, esto va a servir */
  ADCSRA = _BV(ADEN)|_BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0);        // prende el ADC, modo manual, no int, no inicia nada, prescaler a 1/128 (125KHz, 104us cada conversión)
  ADMUX = _BV(REFS0);                                         // setea el Vref
}

void ADCInitConv (byte pin) {
  ADMUX = (ADMUX & 0xe0) | (pin & 0x07);         // setea el canal ADC
  delayMicroseconds(2);
  ADCSRA |= _BV(ADSC);                           // inicia una nueva conversión manual
}

int ADCReadConv () {
  while (ADCSRA & _BV(ADSC)) ;                   // espera hasta que se apague ADSC (está pronta la conversión)  
  return ADC;                                    // equivale al analogRead
}

void set_timer1 () {
  
  // timer 1
  // lo que sigue es para setear el timer1 para generar PWM para los 2 servos de la cabeza
  // fast PWM, 16 bits, prescaler CPU/8, TOP = 40000 
  TCCR1A |= _BV(WGM11); TCCR1A &= ~_BV(WGM10);
  TCCR1B |= _BV(WGM12) | _BV(WGM13) | _BV(CS11);
  TCCR1B &= ~(_BV(CS10) | _BV(CS12));	
  ICR1 = 40000;
  
}

////////////////////////////////////////////////////////////////////////////  
////                              servos                               /////
////////////////////////////////////////////////////////////////////////////

int angulo_act [2];      // posición actual de los servos (en unidades servísticas, que son de -750 a 750, más o menos)

void set_pos (byte servo, int angulo, bool absolute) {
  if (!absolute) {angulo += angulo_act [servo];}
  angulo = constrain (angulo, S_MIN, S_MAX);
  angulo_act [servo] = angulo;
  analogWrite ((servo==0 ? S_1_PIN : S_2_PIN), S_CENTER + angulo);
}

int get_pos (byte servo) {
  return angulo_act [servo];
}
