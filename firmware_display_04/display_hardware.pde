
/** Initialize IO Pins **/
void ioinit (void) {
  // MISO
  DDRB |= (1 << 4);
	
  // Row Selection
  DDRD = 0xFF;  
  PORTD = 0x01;              
	
  // CLK, CLR, DATA, LATCH, EN
  DDRC = 0x1F;
	
  // PORTC |= 0x1F;
  sbi(PORTC, CLK);
  sbi(PORTC, CLR);
  sbi(PORTC, DATA);
  sbi(PORTC, LATCH);
  sbi(PORTC, EN);
	
  // Initialize SPI
  SPCR = (1 << SPE) | (1 << SPIE);
  sei();
}

// this is the function that shifts 16 bits out to the 74hc595 shift registers
// it is inlined to speed things up
void inline shiftLine (uint32_t line, volatile uint8_t rowNum) {
  
  // This will hold the bytes that are shifted out on each row
  static byte lineByte [3]; 
  
  lineByte[0] = line;
  lineByte[1] = line >> 8;
  lineByte[2] = line >> 16;
    
  byte i, j;

  // Clear Latch
  cbi (PORTC, LATCH);
  
  for (j = 0; j < 3; j++){
    for(i = 0; i < 8; i++){
      
      // Lower Clock
      cbi (PORTC, CLK);
      
      if (lineByte[j] & (1 << i)) {     
        sbi (PORTC, DATA);
      } else {
        cbi (PORTC, DATA);
      }

      // Raise Clock
      sbi (PORTC, CLK);

    }
  }
  
  sbi(PORTC, EN);             // Tristate Shift Output
  sbi(PORTC, LATCH);          // Latch Data
  PORTD = (1 << rowNum);      // Set row
  cbi(PORTC, EN);             // Enable Shift Output
}


void refresh_screen () {
 
  uint32_t line; 
  static byte row = 0;
  static byte pwm = 0;
    
  //Process each row 16 times, for PWM functionality
  // Start with no leds
  line = 0;
	  
  // Process each column
  for (byte pixel = 0; pixel < 8; pixel++) {
    byte point = pixel + (8 * row);
    // Check if red led should burn
    if (image_red [point] > pwm) {line |= red [pixel];}
    // Check if green led should burn
    if (image_green [point] > pwm) {line |= green [pixel];}
    // Check if blue led should burn
    if (image_blue [point] > pwm) {line |= blue [pixel];}
  } 
          
  // Shift line out to Shift Registers
  shiftLine (line, row);
                         
  pwm = (pwm + 1) % 16;
  if (pwm == 0) {row = (row + 1) % 8;}

}


