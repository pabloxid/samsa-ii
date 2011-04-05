
#include "ring_buffer.h"
#include <stdlib.h>


RingBuffer::RingBuffer (int* buffer_, byte size_) {
  buffer = buffer_;
  size = size_;
}
  
byte RingBuffer::get_index (byte index) {
  return (p+index) % size;
} 

int RingBuffer::read (byte index) {                  // 0 es la posicion actual, 1 la última, 2 la penúltima, etc.
  return buffer [get_index(index)];
}  

void RingBuffer::store (float new_value) {
  p = (p + size - 1) % size;
  buffer [p] = new_value;
}  

