
int PERIODO_MIN_TICKS = 150;    // aproximadamente 1 segundo para todo el ciclo
float Kmin =  0.3;   // aprox. 50 ticks por paso (podría estar en relación a largo_pasos, que a su vez podria estar en relación a velocidad)
int TPS = 14;     // ticks-per-segment, constante para las curvas bezier


void setup() {
  
  caminata (19.9999995, 1000, 1, 14);
  
}

void draw() {
  
  
  
  
}


void caminata (float velocidad, float desplazamiento, int marcha, float largo_pasos) {
  // si curva == false -> caminata recta con dirección [angulo]
  // si curva == true  -> caminata curva con centro [centro] y sentido = signo de [angulo]
  // todos los parámetros son en centímetros y en segundos.
  // marcha admite los valores 1, 2 y 3 (es el numero de patas simultáneas)  
  
  
     
  // 1) vamos a calcular: fases y elegir la secuencia
  // int fases = 6 / marcha;
    
  // 2) vamos a calcular: K (relación entre duracion_paso y periodo_sub_ciclo), agrupamiento y periodo_sub_ciclo
  int agrupamiento = 0;
  int periodo_sub_ciclo = 0;
  float K = 0;
  float Kmax = 0;
  int Amax = 0;
  int fases = 0;
  marcha --;      // marcha se va a incrementar por lo menos 1 vez
  do {
    if (K < Kmax) {
      K = min (K + .05, Kmax);         // Kinc = .05
    } else {
      if (agrupamiento < Amax) {
        agrupamiento ++;
        Kmax = 3.0/(marcha*agrupamiento*1.2);    // revisar esto
        K = Kmin;
      } else {
        if (marcha < 3) {
          marcha ++;
          Amax = 3 / marcha;    // división entera: va a dar 1 si marcha>1
          fases = 6 / marcha;
          agrupamiento = 0;
        } else {break;}
      }
    }
    periodo_sub_ciclo = (int) (-largo_pasos*agrupamiento/(velocidad*.004*(K*agrupamiento-fases)));  // [velocidad*TICK] es [velocidad*TICK*escala], pero escala=1 en este momento.
  } while (periodo_sub_ciclo < PERIODO_MIN_TICKS);
  
  // 3) vamos a calcular: periodo_pasos, duracion_pasos
  int periodo_pasos;
  int duracion_pasos = (int) (K * periodo_sub_ciclo);
  if (agrupamiento == 1) {
    periodo_pasos = periodo_sub_ciclo;
  } else {
    periodo_pasos = (int) (((Kmax - K) * periodo_sub_ciclo / agrupamiento) + 2);    // revisar esto también     
  }
   
  // 4) vamos a calcular: escala (y escalar los 3 parámetros anteriores)             /// arreglar primero aca
  int escala = min (min(duracion_pasos, periodo_pasos), periodo_sub_ciclo) / 4;
  if (escala > 1) {
    periodo_sub_ciclo = (int) (1.0*periodo_sub_ciclo/escala + 0.5);
    duracion_pasos = (int) (1.0*duracion_pasos/escala + 0.5);
    periodo_pasos = (int) (1.0*periodo_pasos/escala + 0.5);
  } else {escala = 1;}
  
  // 5) vamos a calcular: modulo_vector, ticks, altura_pasito, nsegmentos
  float modulo_vector = velocidad * .004 * escala;
  int ticks = (int) (desplazamiento / modulo_vector + duracion_pasos);  
  // int ticks = (desplazamiento + sqrt (sq(desplazamiento) + 4*modulo_vector*desplazamiento*duracion_pasos)) / (2*modulo_vector);
  /* a esta altura podemos hacer la siguiente operación comprobatoria: 
	    periodo_sub_ciclo = (largo_pasos/modulo_vector + duracion_pasos)*agrupamiento/fases */
  int nsegmentos =  (int) (constrain (duracion_pasos*escala*(largo_pasos/10)/TPS, 2, 10));
  float altura_pasito = 5 + largo_pasos/2;                                                // este parámetro podría dejar de existir 
  // hasta acá lo que es común a traslación y rotación

  
  println ("ticks :" + ticks);
  println ("marcha :" + marcha);
  println ("fases: " + fases);
  println ("agrupamiento: " + agrupamiento);
  println ("escala: " + escala);
  println ("periodo_sub_ciclo: " + periodo_sub_ciclo);
  println ("periodo_pasos: " + periodo_pasos);
  println ("duracion_pasos: " + duracion_pasos);
  println ("altura_pasito: " + altura_pasito);
  println ("nsegmentos: " + nsegmentos);
    
    
}
