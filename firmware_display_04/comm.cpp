/*
  Based on HardwareSerial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.
*/


#include "comm.h"

ring_buffer rx_buffer = { { 0 }, 0, 0 };

// Constructors //

Comm::Comm (ring_buffer *rx_buffer) {
  _rx_buffer = rx_buffer;
}

// Public Methods //


int Comm::available(void) {
  return (128 + _rx_buffer->head - _rx_buffer->tail) % 128;     // siempre hardcodeado a 128
}

int Comm::read(void) {
  // if the head isn't ahead of the tail, we don't have any characters
  if (_rx_buffer->head == _rx_buffer->tail) {
    return -1;
  } else {
    unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
    _rx_buffer->tail ++;
    _rx_buffer->tail &= 127;      // acá
    return c;
  }
}

void Comm::write(uint8_t c) { }   // falta implementar


// Preinstantiate Objects //

Comm comm (&rx_buffer);

