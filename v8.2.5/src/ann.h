// ann.h created for project v8.2 on 08/20/2011 07:40:29

/**********************************************************
*  artificial neural network - minimalist implementation  *
**********************************************************/

#ifndef ANN_H
#define ANN_H

typedef unsigned char byte;

class Nnode {
	private:
		float *weight;
	public:
		byte n_inputs;
		Nnode (byte n_inputs);
   void set_weights (float *weight_);
		float compute (float* input);
};
  
class Nlayer {
	public:
		Nnode *node;
		byte n_nodes;
		Nlayer (byte n_nodes, byte n_inputs);
		void compute (float *input, float *output);
};
  
class Nnetwork {
	public:
		Nlayer *layer;
		byte n_layers;
		Nnetwork (byte n_layers, byte *n_nodes, byte n_inputs);
		void compute (float *input, float *output);
};

#endif