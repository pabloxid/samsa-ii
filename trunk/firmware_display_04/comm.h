
#ifndef COMM_H
#define COMM_H

#include <inttypes.h>

// Define constants and variables for buffering incoming serial data.  We're
// using a ring buffer (I think), in which rx_buffer_head is the index of the
// location to which to write the next incoming character and rx_buffer_tail
// is the index of the location from which to read.

struct ring_buffer {
  volatile uint8_t buffer [128];      // hardcodeado a 128 para que sea más rápido
  uint8_t head;
  uint8_t tail;
};

extern ring_buffer rx_buffer;

inline void store_char (unsigned char c, ring_buffer *rx_buffer)
{
    rx_buffer->buffer[rx_buffer->head] = c;
    rx_buffer->head ++;
    rx_buffer->head &= 127;            // acá
}

class Comm
{
  private:
    ring_buffer *_rx_buffer;
    
  public:
    Comm (ring_buffer *rx_buffer);
    int available ();
    int read ();
    void write (uint8_t c);
};

extern Comm comm;

#endif
