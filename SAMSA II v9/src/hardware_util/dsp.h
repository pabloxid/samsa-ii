#ifndef DSP_H
#define DSP_H

/* - esto es un RingBuffer que opera sobre un array de cualquier tipo
   - el array se define afuera, y se le pasa un puntero al mismo
   - está definido sólo en el .h
   - junto con él, una implementación del moving_average_filter y la rutina "minimo"
*/
	
typedef unsigned char byte;

template <class T> class RingBuffer {
  
	public:
		RingBuffer () { }
		
		RingBuffer (T* buffer_, byte size_) {
			buffer = buffer_;
			size = size_;
		}
		
		T read (byte index) {              // 0 es la posición actual, 1 la última, 2 la penúltima, etc.
			return buffer [get_index(index)];
		}
		
		void store (T new_value) {
			p = (p + size - 1) % size;
			buffer [p] = new_value;
		}
    
	private:
		
		byte get_index (byte index) {
			return (p+index) % size;
		} 
		
		T* buffer;
		byte p;
		byte size;
    
};

// atencion, si los buffers son *char, el moving average filter no puede ser de más de 4 o 5 puntos

template <class T> void mov_avg_filter (RingBuffer<T>* origen, T* destino, byte puntos) {
  *destino = *destino - origen->read(puntos) + origen->read(0);
}

template <class T> byte minimo (RingBuffer<T>* buffer, byte from, byte to) {               // devuelve la posición del mínimo en el rango
	byte min = from;
	for (byte i = from + 1; i < to; i++){                                                                   
		if (buffer->read(i) < buffer->read(min)) {min = i;}
	}
	return min;
}

#endif