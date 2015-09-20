#ifndef RING_H
#define RING_H

typedef unsigned char byte;

class RingBuffer {
  
  public:
    RingBuffer (int* buffer_, byte size_);
    int read (byte index);
    void store (float new_value);
    
  private:
    byte get_index (byte index);
    int* buffer;
    byte p;
    byte size;
    
};


#endif
