// ann.cpp created for project v8.2 on 08/20/2011 07:40:47

/*****************************************
*       artificial neural network        *
******************************************/

#include "ann.h"
#include "util.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

Nnode::Nnode (byte n_inputs) {
	this->n_inputs = n_inputs;
	weight = (float*) malloc ((n_inputs+1)*sizeof(float));
}

void Nnode::set_weights (float *weight_) {
	memcpy (weight, weight_, (n_inputs+1)*sizeof(float));
}
	
float Nnode::compute (float* input) {
	double acumulator = 0;
	for (byte n=0; n<n_inputs; n++) {
		acumulator += input[n]*weight[n];
	}
	acumulator += weight[n_inputs];                             // procesa el 'bias weight'
	// if (acumulator < -22.52) {acumulator = -22.52;}                   // esto es por algún bug 
	return sigmoide (acumulator);                               // sigmoid
}

Nlayer::Nlayer (byte n_nodes, byte n_inputs) {
	this->n_nodes = n_nodes;
	node = (Nnode*) malloc (n_nodes*sizeof(Nnode));
	for (byte n=0; n<n_nodes; n++) {
		node [n] = Nnode (n_inputs);
	}
}

void Nlayer::compute (float *input, float *output) {
	for (byte n=0; n<n_nodes; n++) {
		output[n] = node[n].compute(input);
	}
}

Nnetwork::Nnetwork (byte n_layers, byte *n_nodes, byte n_inputs) {
	this->n_layers = n_layers;
	layer = (Nlayer*) malloc (n_layers*sizeof(Nlayer));
	for (byte n=0; n<n_layers; n++) {
		layer [n] = Nlayer (n_nodes[n], n_inputs);
		n_inputs = n_nodes[n];
	}
}
  
void Nnetwork::compute (float *input, float *output) {
  
	// esto quedó sin implementar
	// porque es realmente un huevo
	// pero la idea es muy buena, una red con n cantidad de layers
	
}