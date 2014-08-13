#ifndef _CHAR_RECOG_H
#define _CHAR_RECOG_H

#define MAXMDF          4
#define FEATUREH        5
#define NFEATURE        160                 /// 2 * 4 * MAXMDF * FEATUREH

/// neuron for upper group
#define NUM_IN_CHAR 	(NFEATURE+1)                 /// NFEATURE + 1
//#define NUM_HID_UPPER   30
#define NUM_OUT_CHAR 	36



void initNetwork();
void createNetworkFromFile();
void saveNetworkToFile(char *fileName);
int* charRecognition(double *input, double *prop);
double* getOutFromChar(char cInput);
double testError(double* input, double* output);
void trainNetwork(double *input, double *output, double learnRate);
void deleteNetwork();

#endif
