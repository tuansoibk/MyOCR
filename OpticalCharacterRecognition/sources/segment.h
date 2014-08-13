#ifndef _SEGMENT_H
#define _SEGMENT_H

#include "common.h"
#define MAX_SEG_LINE 10


int* lineDensity(Image binImg);

int getlineSegment(int* lnDen, int maxRow, int *start, int *end, int* boxHeight);

int* colDensity(Image binImg, int start, int end);

int getWordSegment(int* colDen, int maxCol, int boxHeight, int *start_w, int *end_w);

//int getCharSegment(Image binImg, int* colDen, int maxCol, int start_l, int boxHeight, int *start_c, int *end_c);

int getCharSegment(Image wordImg, int * colDen, int boxHeight, int* start_c, int* end_c);

int wordOverSegment(Image wordImg) ;

Image getBoxImage(Image binImg, int hStart, int hEnd, int wStart, int wEnd);

Image getCharImage(Image binImg, int hStart, int hEnd, int wStart, int wEnd);

Image getImageFromRect(Image binImg, Rect rect, int index);
int lineGrouping(int nLabel) ;
int ccLineLabeling(Image input, int start);
int getSlantDeg(Image binImg, int baseLine, int nLine);

Rect getCharRect(int label);
int getSegmentIndex(int index);
int getBaselineHeight(int nLine);
int getSpace(int index, int nLine, Image input);
int getAverageSpace(int nLine);
int ccLabeling(Image input);

int checkConnect(Image input, int col);

int classifyChar(Rect rect, int extraHeight, int boxHeight);

Image FilterByCharSize(Image inputImage, Image **pLineImageList,
		Image ***pCharImageList, int **pCharNumberList, int *nLines);

int GetBaseLine(Image **pCharImageList, int *pCharNumberList, int nLines);

double GetSlantAngle(Image *pLineImageList, int nLines, int baseLine);

#endif
