#include <math.h>
#include <time.h>
#include "neuron.h"
#include "common.h"
#define EPSILON  0.01
#define MOMENTUM 0.2
#define MAX_HID 200
#define MAX_OUT 33

double *hid, *out, *hidDelta, *outDelta;

double transferFunc(double input){
	return 1/(1 + exp(-input));
}

void initNode(){
    /// init hiden and output
    hid = (double*) malloc(MAX_HID * sizeof(double));
    out = (double*) malloc(MAX_OUT * sizeof(double));

    /// hidden, output error
    hidDelta = (double*) malloc(MAX_HID * sizeof(double));
    outDelta = (double*) malloc(MAX_OUT * sizeof(double));
}

void releaseNode(){
    free(hid);
    free(out);
    free(hidDelta);
    free(outDelta);
}

/**
 * Init weight randomly
 */
void initWeight(Neuron *neuron, double bias)
{
	int i, j;
	int numIn = neuron->numIn;
	int numHid = neuron->numHid;
	int numOut = neuron->numOut;

	/** allocate */
	neuron->weightIn = (double**) malloc(numHid * sizeof(double*));
	for(i = 0; i < numHid; i++){
		neuron->weightIn[i] = (double*) malloc(numIn * sizeof(double));
	}

	neuron->weightHid = (double**) malloc(numOut * sizeof(double*));
	for(i = 0; i < numOut; i++){
		neuron->weightHid[i] = (double*) malloc(numHid * sizeof(double));
	}

    neuron->preDeltaIn = (double**) malloc(numHid * sizeof(double*));
	for(i = 0; i < numHid; i++){
		neuron->preDeltaIn[i] = (double*) malloc(numIn * sizeof(double));
	}

	neuron->preDeltaHid = (double**) malloc(numOut * sizeof(double*));
	for(i = 0; i < numOut; i++){
		neuron->preDeltaHid[i] = (double*) malloc(numHid * sizeof(double));
	}

    /** set random value */
	srand(time(NULL));
	for (i = 0; i < numHid; i++) {
		for (j = 0; j < numIn; j++) {
			neuron->weightIn[i][j] = (double)(rand()%100 - 50)/50;
		}
	}

	for (i = 0; i < numOut; i++) {
		for (j = 0; j < numHid; j++) {
			neuron->weightHid[i][j] = (double)(rand()%100 -50 )/200;
		}
	}

	/** set delta value */
	for (i = 0; i < numHid; i++) {
		for (j = 0; j < numIn; j++) {
			neuron->preDeltaIn[i][j] = 0;
		}
	}

    for (i = 0; i < numOut; i++) {
		for (j = 0; j < numHid; j++) {
			neuron->preDeltaHid[i][j] = 0;
		}
	}

	neuron->bias = bias;
}

/**
 * Calc output of network
 */
double* forward(Neuron neuron, double *inValue)
{
	int i, j;
	int numIn = neuron.numIn;
	int numHid = neuron.numHid;
	int numOut = neuron.numOut;

	/// calc hidden value
	for (i = 0; i < numHid; i++) {
		/// sum function
		hid[i] = 0;
		for (j = 0; j < numIn - 1; j++) {
			hid[i] += inValue[j] * neuron.weightIn[i][j];
		}
		hid[i] += neuron.bias * neuron.weightIn[i][j];
		/// transfer function
		hid[i] = transferFunc(hid[i]);
	}

	/// calc output value - no bias
	for (i = 0; i < numOut; i++) {
		/// sum function
		out[i] = 0;
		for (j = 0; j < numHid; j++) {
			out[i] += hid[j] * neuron.weightHid[i][j];
		}
		/// transfer function
		out[i] = transferFunc(out[i]);
	}
	return out;
}

/**
 * Mean square error
 */
double meanSquareError(Neuron neuron, double* inValue, double* outValue){
	int i;
	double mse = 0;

	double* out = forward(neuron, inValue);
	for(i = 0; i < neuron.numOut; i++){
		out[i] -= outValue[i];
		mse += out[i] * out[i] / neuron.numOut;
	}
	return mse;
}

/**
 * Back propagation algorithm
 */
void backPropagation(Neuron *neuron, double* inValue, double* outValue, double learn_rate)
{
	int i, j;
	double delta;
	int numIn = neuron->numIn;
	int numHid = neuron->numHid;
	int numOut = neuron->numOut;

    /// calc hidden value
	for (i = 0; i < numHid; i++) {
		/// sum function
		hid[i] = -neuron->bias;
		for (j = 0; j < numIn; j++) {
			hid[i] += inValue[j] * neuron->weightIn[i][j];
		}
		/// transfer function
		hid[i] = transferFunc(hid[i]);
	}

	/// calc output value
	for (i = 0; i < numOut; i++) {
		/// sum function
		out[i] = -neuron->bias;
		for (j = 0; j < numHid; j++) {
		out[i] += hid[j] * neuron->weightHid[i][j];
		}
		/// transfer function
		out[i] = transferFunc(out[i]);
	}

	/// calc error output
	for (i = 0; i < numOut; i++) {
//		outDelta[i] = outValue[i] - out[i];
//			if(outDelta[i] > EPSILON || outValue[i] < -EPSILON) cont = 1;
		outDelta[i] = (outValue[i] - out[i])*(1 - out[i])*out[i];
//			printf("outDelta = %.2lf",outDelta[i]);
	}

	/// calc error hidden
	for (j = 0; j < numHid; j++) {
		for (i = 0; i < numOut; i++) {
			hidDelta[j] += outDelta[i]* neuron->weightHid[i][j];
		}
		hidDelta[j] = hid[j]*(1-hid[j])*hidDelta[j];
//			printf("hidDelta = %.2lf", hidDelta[i]);
	}

	/// update weight hid
	for (i = 0; i < numOut; i++) {
		for (j = 0; j < numHid; j++) {
		    delta = MOMENTUM * neuron->preDeltaHid[i][j] + learn_rate * hid[j] * outDelta[i];
			neuron->weightHid[i][j] += delta;
            neuron->preDeltaHid[i][j] = delta;
		}
	}

	/// update weight input
	for (i = 0; i < numHid; i++) {
		for (j = 0; j < numIn; j++) {
		    delta = MOMENTUM * neuron->preDeltaIn[i][j] + learn_rate * inValue[j]* hidDelta[i];
			neuron->weightIn[i][j] += delta;
			neuron->preDeltaIn[i][j] = delta;
		}
	}

}

/**
 * Save weight to file
 */
void weightToFile(Neuron neuron, char* fileName){
	int i, j;
	int numIn = neuron.numIn;
	int numHid = neuron.numHid;
	int numOut = neuron.numOut;

	FILE* f= fopen(fileName, "w");

	fprintf(f, "%lf ", neuron.bias);

	for (i = 0; i < numHid; i++) {
		for (j = 0; j < numIn; j++) {
			fprintf(f, "%lf ", neuron.weightIn[i][j]);
		}
	}

	for (i = 0; i < numOut; i++) {
		for (j = 0; j < numHid; j++) {
			fprintf(f, "%lf ", neuron.weightHid[i][j]);
		}
	}

	fclose(f);
}

/**
 * Get weight from file
 */
void getWeightFromFile(Neuron* neuron, char *fileName){
	int i, j;
	int numIn = neuron->numIn;
	int numHid = neuron->numHid;
	int numOut = neuron->numOut;

	// init weight
	neuron->weightIn = (double**) malloc(numHid * sizeof(double*));
	for(i = 0; i < numHid; i++){
		neuron->weightIn[i] = (double*) malloc(numIn * sizeof(double));
	}

	neuron->weightHid = (double**) malloc(numOut * sizeof(double*));
	for(i = 0; i < numOut; i++){
		neuron->weightHid[i] = (double*) malloc(numHid * sizeof(double));
	}
	neuron->preDeltaIn = (double**) malloc(numHid * sizeof(double*));
	for(i = 0; i < numHid; i++){
		neuron->preDeltaIn[i] = (double*) malloc(numIn * sizeof(double));
	}

	neuron->preDeltaHid = (double**) malloc(numOut * sizeof(double*));
	for(i = 0; i < numOut; i++){
		neuron->preDeltaHid[i] = (double*) malloc(numHid * sizeof(double));
	}


	FILE* f= fopen(fileName, "r");

	fscanf(f, "%lf", &neuron->bias);

	for (i = 0; i < numHid; i++) {
		for (j = 0; j < numIn; j++) {
			fscanf(f, "%lf", &neuron->weightIn[i][j]);
		}
	}

	for (i = 0; i < numOut; i++) {
		for (j = 0; j < numHid; j++) {
			fscanf(f, "%lf", &neuron->weightHid[i][j]);
		}
	}

	fclose(f);
}

/**
 * Delete neuron network
 */
void deleteNeuron(Neuron neuron){
    int i;
    int numHid = neuron.numHid;
    int numOut = neuron.numOut;

    /// free weight
    for(i = 0; i < numHid; i++){
        free(neuron.weightIn[i]);
    }
    free(neuron.weightIn);

    for(i = 0; i < numOut; i++){
        free(neuron.weightHid[i]);
    }
    free(neuron.weightHid);

    /// free delta
/*    for(i = 0; i < numHid; i++){
        free(neuron.preDeltaIn[i]);
    }
    free(neuron.preDeltaIn);

    for(i = 0; i < numOut; i++){
        free(neuron.preDeltaHid[i]);
    }
    free(neuron.preDeltaHid);
*/
}
