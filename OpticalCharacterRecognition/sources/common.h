#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

#define POINTER_VALUE(p) (*p)

#define RIGHT 		0
#define RIGHT_TOP 	1
#define TOP			2
#define LEFT_TOP	3
#define LEFT 		4
#define LEFT_BOT 	5
#define BOT			6
#define RIGHT_BOT	7
#define NUM_DIR		8

#define VERTICAL    2
#define BDIAGONAL   3
#define HORIZONTAL  4
#define FDIAGONAL   5

// character types
#define UPPER 	0
#define NORM 	1
#define LOWER 	2

#define PI 3.14159


typedef struct {
    int h;
    int w;
    uint8** pixels;
} Image;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} Rect;

typedef struct tagRECT{
	int x;
	int y;
	int h;
	int w;
} RECT, *PRECT;

typedef struct  tagPOINT{
	int x;
	int y;
} POINT, *PPOINT;

typedef unsigned short PIXEL;

/** Image function */
Image CropImage(Image inputImage, RECT rect);
Image CreateImage(int h, int w);
Image CopyImage(Image src);
void copyImage(Image dst, Image src);
int inImage(int y, int x, int h, int w);
int getNeighbor(Image input, int dir, int x, int y);
int getNeighborForMatch(Image input, int dir, int x, int y);
void setNeighbor(Image input, int dir, int x, int y, int val);
void printImage(Image input, char* fileName);
void printFeatureImage(Image input, char* fileName);
void deleteImage(Image input);

/** Rect function */
void setRect(Rect *rect, int x, int y, int w, int h);
void addPointToRect(Rect *rect, int x, int y);

bool IsPointInImage(POINT point, int h, int w);
int GetPixelValue(Image inputImage, POINT point);
void SetPixelValue(Image inputImage, POINT point, int pixelValue);
int GetNeighborPixelValue(Image inputImage, POINT point, int dir);
PPOINT GetPixelPoint(Image inputImage, POINT point);
void SetPixelPoint(Image *pImage, POINT point);
void ClearPixelPoint(Image *pImage, POINT point);
POINT GetNeighborPoint(POINT point, int dir);
PPOINT GetNeighborPixelPoint(Image inputImage, POINT point, bool *isFullyWalked);
PPOINT GetNeighborPixelPointOnMatch(Image inputImage, POINT point, int pixelValue, int *neighborDir);
void CopyImageWithinRect(Image *dst, Image src, RECT rect, bool binarize);
void CopyImageWithinList(Image *dst, Image src, PPOINT pPointList, int nPoints, bool binarize);
Image CropImageWithinList(Image src, PPOINT pPointList, int nPoints, RECT rect, bool binarize);
void SetRect(PRECT pRect, POINT point, int h, int w);
void CopyRect(PRECT pRect, RECT rect);
void AddPointToRect(PRECT pRect, POINT point);
void SetPoint(PPOINT pPoint, int x, int y);

#endif
