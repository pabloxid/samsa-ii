
/*  RESUMEN DEL LENGUAJE

  data > 224 --> cabecera
  
  data - 224 --> instrucción
  
  instruccion 0-7   --> sin parámetros
  instruccion 8-15  --> 1 parámetro
  instruccion 16-23 --> 2 parámetros
  instruccion 24-31 --> 3 parámetros
  
  * RECEIVE:
  
      inst 0 --> request cm dist
      inst 1 --> request vel
      inst 2 --> request lat
      inst 3 --> request event_vel
      inst 4 --> request event_lat
      inst 5 --> request mm dist
      
      inst 6 --> enable send all
      inst 7 --> disable send all
      
      inst-1 0 --> set thresh cm dist
      inst-1 1 --> set thresh vel
      inst-1 2 --> set thresh lat
      inst-1 3 --> set thresh event_vel
      inst-1 4 --> set thresh event_lat
      
      inst-1 5 --> set time (pause)
      
      inst-2 0 --> set thresh mm dist
 
  * SEND:
    
      inst-1 0 --> cm dist value
      inst-1 1 --> vel value
      inst-1 2 --> lat value
      inst-1 3 --> event_vel value
      inst-1 4 --> event_lat value
      
      inst-1 7 --> IR command
      
      inst-2 0 --> mm dist value
      
      inst-3 7 --> send_all packet
       
 */
  
 //=========================================================================================================================
 //=============================================== cachos de código sin usar ===============================================
 //=========================================================================================================================
 

/*                                                       
  int temp = HP[0];
  HP[0] = ((global[0]+global[2])*385 - global[1]*770 - HP[0]*325 - HP[1]*215)/1000;       // pasa-alto 'chebyshev' recursivo de 2 polos (con 5% ripple)
  HP[1] = temp;
  if (HP[0]==0 && HP[1]==0) {stop_detect = 0;} else {stop_detect++;}                      // detección del gesto "stop" (alta frecuencia)
  if (stop_detect > 33) {rst = true;}                                                     // en caso afirmativo, activa el flag de reset (hay que subir este umbral)
*/

/*
byte meter () {                                                                          // devuelve el pico máximo de global[] en un lapso de 500ms
                                                                                         // para usar en estado=0, detectar los "bursts" de LMDLD
  byte a, b = 0;
  for (byte i=0; i<5; i++) {
    delay (100);
    a = global [maximo (global, 20)];
    if (a > b) {b = a;}
  }
  return b;
} 

char bin2sign (boolean var) {                       // var = 0 --> sign = 1; var = 1 --> sign = -1
  char signo = 1 - 2*var;
  return signo;
}  

byte maximo (volatile int variable [], byte historia) {                                        // devuelve la posición del máximo en el bloque
  byte i;
  byte maxx = 0;
  for (i = 1; i < historia; i++){                                                                   
    if (variable [i] > variable [maxx]) {maxx = i;}
  }
  return maxx;
}

byte minimo (volatile int variable [], byte historia) {                                       // devuelve la posición del mínimo en el bloque
  byte i;
  byte minn = 0;
  for (i = 1; i < historia; i++){                                                                   
    if (variable [i] < variable [minn]) {minn = i;}
  }
  return minn;
}

*/

