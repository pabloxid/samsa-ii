
#include "dsp.h"
#include <stdlib.h>

char buffer [18][5];                              // buffers para el filtrado de la carga de los motores
RingBuffer filtro[18];                             // circularización de dichos buffers 

void init_filters () {                            // inicialización de los buffers circulares
	for (byte f=0; f<18; f++) {
		filtro [f] = RingBuffer (buffer[f], 5);          
	}
}

RingBuffer::RingBuffer () { }

RingBuffer::RingBuffer (char* buffer_, byte size_) {
  buffer = buffer_;
  size = size_;
}
  
byte RingBuffer::get_index (byte index) {
  return (p+index) % size;
} 

char RingBuffer::read (byte index) {                  // 0 es la posicion actual, 1 la última, 2 la penúltima, etc.
  return buffer [get_index(index)];
}  

void RingBuffer::store (char new_value) {
  p = (p + size - 1) % size;
  buffer [p] = new_value;
}

// atencion, los buffers son *char, el moving average filter no puede ser de más de 4 o 5 puntos
void mov_avg_filter (RingBuffer* origen, char* destino, byte puntos) {
  *destino = *destino - origen->read(puntos) + origen->read(0);
}