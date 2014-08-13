#ifndef _NEURON_H
#define _NEURON_H

#define MAX_LOOP 5000

typedef struct {
	int numIn;
	int numHid;
	int numOut;
	double bias;
	double** weightIn;
	double** weightHid;
	double** preDeltaIn;
	double** preDeltaHid;
} Neuron;

void initWeight(Neuron* neuron, double bias);
void initNode();
void releaseNode();

void getWeightFromFile(Neuron* neuron, char* fileName);
void weightToFile(Neuron neuron, char* fileName);
double* forward(Neuron neuron, double* inValue);
void backPropagation(Neuron* neuron, double* inValue, double* outValue, double learn_rate);
double meanSquareError(Neuron neuron, double* inValue, double* outValue);
void deleteNeuron(Neuron neuron);

#endif
