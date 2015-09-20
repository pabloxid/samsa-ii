
#include <MsTimer2.h>
#include "ring_buffer.h"

int buffer_dist [32], buffer_izq [32], buffer_der [32];
int buffer_flt_dist [26], buffer_flt_izq [1], buffer_flt_der [1];
int buffer_velocidad [30], buffer_lateralidad [50];

RingBuffer dist (buffer_dist, 32), izq (buffer_izq, 32), der (buffer_der, 32);
RingBuffer flt_dist (buffer_flt_dist, 26), flt_izq (buffer_flt_izq, 1), flt_der (buffer_flt_der, 1);
RingBuffer velocidad (buffer_velocidad, 30), lateralidad (buffer_lateralidad, 50); 
int event_vel, event_lat;

int umbrales [] = {0, 127, 127, 127, 127, 0};
byte values [5];
unsigned int mm;
byte pausa = 10;
boolean send_all = false;

unsigned long time;

byte infrared;

void setup() {
  
  Serial.begin (115200);
  pinMode (10, OUTPUT);
  
  pinMode (2, INPUT);
  // digitalWrite (2, HIGH);                 // activa el pull-up interno
  attachInterrupt(0, detect, LOW);       // activa la interrupción externa para el IR-remote
  
  ADCconfig ();
  
  MsTimer2::set(2, sample);                                                  // muestreo del ojo electrónico cada 5ms
  MsTimer2::start();
  
  digitalWrite (10, 1);

}

/* 
   flt_dist.read(0)/21 = distancia en mm
   velocidad.read(0) = velocidad en mm/s (a una distancia de 10cm) y  en cm/s (a una distancia de 1m)  
   lateralidad.read(0) = indica si el obstáculo está más a la derecha o a la izquierda  
   event_vel = detección del gesto de pasar la mano en cualquier sentido
   event_lat = detección del gesto de pasar la mano horizontalmente 
*/

void loop() {
  
  /* esto va a funcionar así:
     1) valores a demanda: al recibir un request, devuelve el valor solicitado
      - la distancia puede ser en cm o mm
      - los valores con signo serán de 1 byte + 100
     
     2) comandos para setear umbrales: cada variable tiene un "umbral", a partir del cual envía valores automáticamente, con un período de 5-10ms
      - la distancia en este caso será en cm 
      - para los valores con signo, el umbral es en valor absoluto
      - todos los umbrales están por defecto al máximo (o sea desactivados) 
      - los umbrales de distancia son negativos, y su valor inicial es 0 
      
     3) va a ser necesario implementar un lenguajecito, como el del display (evolución del clásico midi con elementos del butiá)
  */
      
  values [0] = flt_dist.read(0) / 210;                                  // distancia en cm
  values [1] = 100 + constrain (velocidad.read(0), -100, 100);          // velocidad 
  values [2] = 100 + constrain (lateralidad.read(0), -100, 100);        // si el obstáculo está más a la derecha o a la izquierda  
  values [3] = 100 + constrain (event_vel, -100, 100);                  // gesto de pasar la mano en cualquier sentido
  values [4] = 100 + constrain (event_lat, -100, 100);                  // gesto de pasar la mano horizontalmente  
  mm = flt_dist.read(0) / 21;                                           // distancia en mm          
     
  // auto sends
  if (send_all) {send_msg (7, 3, values);}                              // send_all manda los 3 primeros valores (distancia en cm, velocidad, lateralidad)
                                                                        // es el mensaje 7, para que sea compatible con el osciloscopio de processing (8*3+7+224=255)
  if (values[0] < umbrales[0]) {send_msg (0, 1, values);}
  for (byte f=1; f<5; f++) {
    if (abs(values[f]-100) > umbrales[f]) {send_msg (f, 1, &values[f]);}
  }  
  if (mm < umbrales[5]) {send_2byte_msg (0, mm);}
    
  time = millis ();
  
  while (millis()-time < pausa) {
    leer_serial ();                         // la respuesta es rápida, pero la data que devuelve puede estar atrasada
  }

} 
