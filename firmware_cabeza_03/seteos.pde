

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


