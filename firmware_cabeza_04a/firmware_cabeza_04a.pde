#define INIT_PAUSA  10
#define IR_LED_PIN  12

// servos de la cabeza
#define S_1_PIN      9
#define S_2_PIN      10
#define S_CENTER     3300
#define PAN          0
#define TILT         1
#define S_MAX        700      
#define S_MIN        -700

#include <MsTimer2.h>
#include "ring_buffer.h"

int buffer_dist [32], buffer_izq [32], buffer_der [32];
int buffer_flt_dist [26], buffer_flt_izq [1], buffer_flt_der [1];
int buffer_velocidad [30], buffer_lateralidad [50];

RingBuffer dist (buffer_dist, 32), izq (buffer_izq, 32), der (buffer_der, 32);
RingBuffer flt_dist (buffer_flt_dist, 26), flt_izq (buffer_flt_izq, 1), flt_der (buffer_flt_der, 1);
RingBuffer velocidad (buffer_velocidad, 30), lateralidad (buffer_lateralidad, 50); 
int event_vel, event_lat;

byte (*compute_values[5]) () = {cm, vel, lat, e_vel, e_lat};    // puntero a funciones, sólo para indexar

int umbrales [] = {0, 127, 127, 127, 127, 0};
byte pausa = INIT_PAUSA;
boolean send_all = false;

unsigned long time;

byte infrared;

void set_pos (byte servo, int angulo, bool absolute = true);

void setup() {
  
  Serial.begin (115200);
  pinMode (IR_LED_PIN, OUTPUT);
  
  pinMode (2, INPUT);                     // pin del sensor infrarrojo
  // digitalWrite (2, HIGH);              // activa el pull-up interno
  attachInterrupt(0, detect, LOW);       // activa la interrupción externa para el IR-remote
  
  ADCconfig ();
  
  MsTimer2::set (2, sample);                                                  // muestreo del ojo electrónico cada 2ms
  MsTimer2::start ();
  
  digitalWrite (IR_LED_PIN, 1);
  
  set_timer1 ();                     // timer1 para manejar directamente 2 servos
  set_pos (PAN, 0); 
  set_pos (TILT, 300);               // posiciona la cabeza

}

/* 
   flt_dist.read(0)/21 = distancia en mm
   velocidad.read(0) = velocidad en mm/s (a una distancia de 10cm) y  en cm/s (a una distancia de 1m)  
   lateralidad.read(0) = indica si el obstáculo está más a la derecha o a la izquierda  
   event_vel = detección del gesto de pasar la mano en cualquier sentido
   event_lat = detección del gesto de pasar la mano horizontalmente 
*/


void loop() {
  
  alto_nivel ();
  
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
  
  // calcula todos los valores y los guarda en las variables correspondientes
  static byte values [5];
  for (byte i=0; i<5; i++) {
    values [i] = (*compute_values[i]) ();
  }
  unsigned int mm = compute_mm();           
     
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
    leer_serial ();                         // la respuesta es rápida, pero la data que devuelve puede estar atrasada [ya no, gracias a las funciones de acá abajo]
  }

} 

// estas funciones calculan en el momento los valores a retornar, para evitar el delay
byte cm () {return flt_dist.read(0) / 210;}                                   // distancia en cm
byte vel () {return 100 + constrain (velocidad.read(0), -100, 100);}          // velocidad 
byte lat () {return 100 + constrain (lateralidad.read(0), -100, 100);}        // si el obstáculo está más a la derecha o a la izquierda  
byte e_vel () {return 100 + constrain (event_vel, -100, 100);}                // gesto de pasar la mano en cualquier sentido
byte e_lat () {return 100 + constrain (event_lat, -100, 100);}                // gesto de pasar la mano horizontalmente  
unsigned int  compute_mm () {return flt_dist.read(0) / 21;}                   // distancia en mm   





