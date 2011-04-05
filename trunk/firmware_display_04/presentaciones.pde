
void presentacion2 () {
  
   // Red Line
   scroll_line (2433609);   

   // Green Line
   scroll_line (9487524); 

   // Blue Line
   scroll_line (4856082); 
   
}

void scroll_line (uint32_t line) {
   
   for (byte row = 0; row < 8; row++) {
     shiftLine (line, row);
     delay (50);
   } 

}			

