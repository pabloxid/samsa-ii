// firmware para el ATtiny85 en SAMSA II
// versión 0.1 4/6/2011

#define AN_FUERZA   2
#define AN_ANGULO   3
#define DIG_INT     0
#define DIG_CUAL    1
#define DIG_SIGNO   2


int fuerza, angulo, fuerza1, angulo1; 
int delta_fuerza, delta_angulo;
byte absdf, absda;

void setup() {

  pinMode (DIG_INT, OUTPUT);
  pinMode (DIG_CUAL, OUTPUT);
  pinMode (DIG_SIGNO, OUTPUT);
  
  digitalWrite (DIG_INT, LOW);
  
}

void loop() {
  
  fuerza1 = fuerza;
  angulo1 = angulo;
  delay (5);                          // pausa de muestreo
  fuerza = analogRead(AN_FUERZA);
  angulo = analogRead(AN_ANGULO);
  
  delta_fuerza = (fuerza1-fuerza);
  delta_angulo = (angulo1-angulo);
  absdf = abs(delta_fuerza);
  absda = abs (delta_angulo);
  
  if (absdf + absda > 9) {                 // umbral
    if (absdf > absda) {
      interruptus (true, delta_fuerza>0);
    } else {
      interruptus (false, delta_angulo>0);
    }
  }

}


void interruptus (boolean cual, boolean signo) {
 
  // setea qué parámetro cambió y con qué signo cambió
  digitalWrite (DIG_CUAL, cual);
  digitalWrite (DIG_SIGNO, signo);
  
  digitalWrite (DIG_INT, HIGH);      // activa la interrupción remota
  
  delay (120);             // tiempo de recuperación (efecto antirrebote)  
  
  digitalWrite (DIG_INT, LOW);     // vuelve INT a 0 
  
  // evita que se re-triggee
  fuerza = analogRead(AN_FUERZA);
  angulo = analogRead(AN_ANGULO);
  
}


