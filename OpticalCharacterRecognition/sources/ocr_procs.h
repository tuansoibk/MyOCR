#ifndef OCR_PROCS_H
#define OCR_PROCS_H
#include "common.h"

#define NUM_LOOP 2000
#define NUM_CHAR_TRAIN 39*32
#define NUM_CHAR_TEST 624
#define NUM_CHAR_MAX (NUM_CHAR_TEST > NUM_CHAR_TRAIN ? NUM_CHAR_TEST : NUM_CHAR_TRAIN)

int ocrProcess(Image grayImg);
int ocrProcessFile(char* fileName);
int trainProcess(Image trainImg, Image testImg, double *mse);
int trainLoopProcess(Image trainImg, char* txtFileName, int nLoop);
int autoTrainProcess(int nTrainImg, int nTestImg, double *mse);
double** getFeatureFromImage(Image binImg, int nChar);

#endif // OCR_PROCS_H
