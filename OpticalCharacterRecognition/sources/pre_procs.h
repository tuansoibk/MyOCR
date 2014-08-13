#include "common.h"

#define THRESHOLD 0x3f

#define MAX_GRAY 256

#define WIN_SIZE 80

Image RemoveSlant(Image inputImage, double slantAngle);

Image ExtractContour(Image inputImage);

void SmoothContour(Image *pContourImage);

void ThinOutContour(Image *pContourImage);

Image getGrayLevel(uint32 **bmpPix, int h, int w);

Image binarize(uint32** bmpPix, int h, int w, int localSize);

Image binarizeAdaptOtsu(uint32 **bmpPix, int h, int w);

Image binarizeGrayImageOtsu(Image grayImg);

Image binarizeKmeans(uint32 ** bmpPix, int h, int w);

Image binarizeGrayImageKmeans(Image grayImg);

void smoothImage(Image input);

Image slantRemoval(Image input, int slantDeg);

Image scaleImage(Image input, int h, int w);

Image contourFinding(Image input);
Image lineSkewCorrection(Image input) ;


void thinImageOnMatch(Image input);

void skeletonImage(Image input, Image skeleton);

void smoothContour(Image input);
