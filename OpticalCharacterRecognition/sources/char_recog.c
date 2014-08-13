#include "char_recog.h"
#include "neuron.h"
#include "common.h"

Neuron 	charNetwork;

/// character groups
char charStr[NUM_OUT_CHAR + 1] = "abcdefghijklmnopqrstuvwxyz";//"bdfhkltABCDEFGHIJKLMNOPQRSTUVWXYZ";

extern int NUM_HID_CHAR;

/// prototypes

char indexToChar(int index);


int* getMax(double* input, int len, double* max) {
    int i, j, k;
    int skip;
    int nOut = 5;
    double maxVal;
    int *index = (int*)calloc(nOut, sizeof(int));
    for(i = 0; i < nOut; i++){
        maxVal = 0;
        for(j = 0; j < len; j++) {
            skip = 0;
            for(k = 0; k < i; k++) {
                if(j == index[k]) {
                    skip = 1;
                    break;
                }
            }
            if(!skip && (input[j] > maxVal)){
                index[i] = j;
                maxVal = input[j];
            }
        }
    }
    printf("%c", indexToChar(index[0]));
    //printf("|%c|%c|-", indexToChar(index[2]), indexToChar(index[1]));
    return index;
}

/**
 * Init weight randomly
 */
void initNetwork() {
    charNetwork.numIn = NUM_IN_CHAR;
    charNetwork.numHid = NUM_HID_CHAR;
    charNetwork.numOut = NUM_OUT_CHAR;

    initWeight(&charNetwork, 1);

    initNode();
//	printf("bias = %lf", upperNetwork.bias);
}

/**
 * Init weight from file
 */
void createNetworkFromFile() {
    charNetwork.numIn = NUM_IN_CHAR;
    charNetwork.numHid = NUM_HID_CHAR;
    charNetwork.numOut = NUM_OUT_CHAR;

    getWeightFromFile(&charNetwork, "char.wdb");
    initNode();
}

/**
 * Delete network
 */
void deleteNetwork() {
    deleteNeuron(charNetwork);
    releaseNode();
}

/**
 * Save network to file
 */
void saveNetworkToFile(char *fileName) {
    weightToFile(charNetwork, fileName);
}

/**
 * Recognize char using networks
 */
int* charRecognition(double* input, double* prop) {
    double *output;

    output = forward(charNetwork, input);
    return getMax(output, NUM_OUT_CHAR, prop);
}

/**
 * Get char from index
 */
char indexToChar(int index) {
    return charStr[index];
}

/**
 * Create desire output from char input
 */
double* getOutFromChar(char cInput) {
    int i;
    double *out;

    for (i = 0; i < NUM_OUT_CHAR; i++) {
        if (charStr[i] == cInput) {
            out = (double*) calloc(NUM_OUT_CHAR , sizeof(double));
            out[i] = 1;
            return out;
        }
    }
    printf("null\n");

    return NULL;
}

//int getGroupTypeFromChar(char cInput) {
//    int i;
//
//    for (i = 0; i < NUM_OUT_UPPER; i++) {
//        if (upperStr[i] == cInput) {
//            return UPPER;
//        }
//    }
//
//    for (i = 0; i < NUM_OUT_NORM; i++) {
//        if (normStr[i] == cInput) {
//            return NORM;
//        }
//    }
//
//    for (i = 0; i < NUM_OUT_LOWER; i++) {
//        if (lowerStr[i] == cInput) {
//            return LOWER;
//        }
//    }
//    return -1;
//}

/**
 * Means square error calc
 */
double testError(double* input, double* output) {
    return meanSquareError(charNetwork, input, output);
}

/**
 * Traning network using backpropagate
 */
void trainNetwork(double* input, double* output, double learnRate) {
    backPropagation(&charNetwork, input, output, learnRate);
}
