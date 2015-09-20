/*#include <PID_v1.h>

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
PID myPID (&Input, &Output, &Setpoint, 5, 10, 0, DIRECT);

void search () {

  Setpoint = 0;

  myPID.SetOutputLimits(-700, 700);
  //turn the PID on
  myPID.SetMode(AUTOMATIC);

  myPID.SetSampleTime(50);

  while (1) {
    
    Input = lateralidad.read(0);
    
    myPID.Compute();
    
    set_pos (PAN, Output, true);
        
  }
   
}
        float angulo_actual [2];
	float centro [2] = {0, 0};
	float amp = 15;
	float damper = .00001;
	float delta [2];

void search () {
		
	while (1) {
		angulo_actual[0] = get_pos(PAN);
		angulo_actual[1] = get_pos(TILT);
  
                 delta [0] = amp*frandom(-1,1) + damper*(centro[0] - angulo_actual [0]);
		 delta [1] = 1.7*amp*frandom(-1.0,1.0) + damper*(centro[1] - angulo_actual [1]);
		set_pos (PAN, delta[0], false);
		set_pos (TILT, delta[1], false);
		delay (2);
		
        }
}
 
#include <PID_v1.h>

//Define Variables we'll be connecting to
double Setpoint1, Input1, Output1;
double Setpoint2, Input2, Output2;

//Specify the links and initial tuning parameters
PID anglePID (&Input1, &Output1, &Setpoint1, 1, 0, 0, DIRECT);
PID distancePID (&Input2, &Output2, &Setpoint2, 10, 0, 0, DIRECT);


float radio = 220;
int angulo = random (360);

void search () {
   
  Setpoint1 = 50;
  Setpoint2 = 0;

  anglePID.SetOutputLimits(0, 185);
  distancePID.SetOutputLimits(35, 350);
  
  //turn the PID on
  anglePID.SetMode(AUTOMATIC);
  distancePID.SetMode(AUTOMATIC);

  anglePID.SetSampleTime(60);
  distancePID.SetSampleTime(60);

  while (1) {
     
  float x, y;
  polar2cartesian (&x, &y, radio, angulo);
  set_pos (PAN, x, false);
  set_pos (TILT, 1.71*y, false);
  delay (60);
  
  Input1 = velocidad.read(0);
  Input2 = abs (velocidad.read(0));
  anglePID.Compute();
  distancePID.Compute();
  
  angulo = (angulo + (int)Output1) % 360;
  
  radio = Output2;
  
  Serial.print (radio, DEC);
  //delay (500);
  
  }
}  
  
  
  
/* resumen del algoritmo:
- hay 2 variables: radio y ángulo
- comienza con un movimiento al azar
- si el resultado es positivo, radio tiende a achicarse, y ángulo tiende a permanecer igual
- si el resultado es neutro, ángulo varía levemente
- si el resultado es negativo, ángulo se invierte, y radio tiende a aumentar
*/

#define DIST2MILLIS .3
  
enum {INACTIVO, BUSCAR1, BUSCAR2, SEGUIR, OBJETIVO} estado = INACTIVO;  
  
/*void alto_nivel () {
  // contador de tiempo en el mismo estado
  
  static int radio;
  static int angulo;
    
  switch (estado) {
    case BUSCAR1: 
      radio = 200;
      angulo = random (360);
      mover_cabeza (radio, angulo);
     
      
      
      
      
      
      
      
      
      break;
    case 13:  
      /// aca se pican circulos
      /// y si se mantiene un rato en lo mismo pasa a target
      break;
    case 15:
      break;
  }
}*/  

void alto_nivel() {
/* 
byte cm () {return flt_dist.read(0) / 210;}                                   // distancia en cm
byte vel () {return 100 + constrain (velocidad.read(0), -100, 100);}          // velocidad 
byte lat () {return 100 + constrain (lateralidad.read(0), -100, 100);}        // si el obstáculo está más a la derecha o a la izquierda  
byte e_vel () {return 100 + constrain (event_vel, -100, 100);}                // gesto de pasar la mano en cualquier sentido
byte e_lat () {return 100 + constrain (event_lat, -100, 100);}                // gesto de pasar la mano horizontalmente  
unsigned int  compute_mm () {return flt_dist.read(0) / 21;}                   // distancia en mm   
*/
  
  
  set_pos (PAN, lateralidad.read(0)*5, false);
  
  int g = cm();
  if (g > 10 && velocidad.read(0) > -50) {
    set_pos (TILT, velocidad.read(0)*g/15.0, true);
    set_pos (PAN, random (-1,2)*velocidad.read(0)/2.0, false);
  } 
    
  set_pos (PAN, .99*get_pos(PAN));
  set_pos (TILT, .99*get_pos(TILT));
}
 

float frandom (float a, float b) {
  return random (1000*a, 1000*b) / 1000.0;
}

void mover_cabeza (int radio, int angulo) {
  float rad_ang = radians(angulo);
  int x = radio*cos(rad_ang);
  int y = radio*sin(rad_ang); 
  set_pos (PAN, x, false);
  set_pos (TILT, y, false);
  //delay (DIST2MILLIS*max(abs(x), abs(y)));
} 
 
  
  
 
