#ifndef DSP_H
#define DSP_H

// este DSP es exclusivamente para procesar la data del load de los motores

typedef unsigned char byte;

class RingBuffer {
  
	public:
		RingBuffer (); 
		RingBuffer (char* buffer_, byte size_);
		char read (byte index);
		void store (char new_value);
    
	private:
		byte get_index (byte index);
		char* buffer;
		byte p;
		byte size;
    
};

extern RingBuffer filtro[18];

void init_filters ();
void mov_avg_filter (RingBuffer* origen, char* destino, byte puntos);

#endif