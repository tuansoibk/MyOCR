#include "common.h"

Image CropImage(Image inputImage, RECT rect)
{
	Image resultImage;

	resultImage = CreateImage(rect.h, rect.w);
	for (int y = 0; y < rect.h; y++)
	{
		for (int x = 0; x < rect.w; x++)
		{
			resultImage.pixels[y][x] = inputImage.pixels[y + rect.y][x + rect.x];
		}
	}

	return resultImage;
}

Image CreateImage(int h, int w)
{
	Image img;

	img.h = h;
	img.w = w;
	img.pixels = (uint8**)calloc(h, sizeof(uint8*));

	for(int i = 0; i < h; i++)
	{
		img.pixels[i] = (uint8*)calloc(w, sizeof(uint8));
	}

	return img;
}

Image CopyImage(Image src)
{
	Image resultImage;

	resultImage = CreateImage(src.h, src.w);
	for (int y = 0; y < src.h; y++)
	{
		for (int x = 0; x < src.w; x++)
		{
			resultImage.pixels[y][x] = src.pixels[y][x];
		}
	}

	return resultImage;
}

void copyImage(Image dst, Image src) {
	int i, j, h, w;
	h = src.h;
	w = src.w;

    for(i = 0; i < h; i++){
		for(j = 0; j < w; j++){
			dst.pixels[i][j] = src.pixels[i][j];
		}
	}
}

int inImage(int y, int x, int h, int w)
{
        return (y >= 0 && y < h && x >=0 && x < w);
}

void deleteImage(Image input) {
        int i;
        for(i = 0; i < input.h; i ++){
                free(input.pixels[i]);
        }
        free(input.pixels);
}

void printImage(Image input, char* fileName){
	int i, j;
	/// open file
	FILE * f = fopen(fileName, "w");

	/// write to file
	for (i = 0; i < input.h; i ++) {
		for (j = 0; j < input.w; j++)

			if(input.pixels[i][j]) fprintf(f, "%d", 1);
			else fprintf(f, ".");
		fprintf(f, "\n");
	}
	fflush(f);
	fclose(f);
}

void printFeatureImage(Image input, char* fileName) {
	unsigned char pixVal;
	int i, j;
	/// open file
	FILE * f = fopen(fileName, "w");

	/// write to file
	for (i = 0; i < input.h; i ++) {
		for (j = 0; j < input.w; j++) {
			pixVal = input.pixels[i][j];

			if(pixVal == '.') fprintf(f, ".");
			else fprintf(f, "%d", pixVal);
		}
		fprintf(f, "\n");
	}
	fclose(f);
}


int getNeighbor(Image input, int dir, int x, int y)
{
	switch (dir) {
	case RIGHT:
		x += 1;
		break;
	case RIGHT_TOP:
		x += 1;
		y -= 1;
		break;
	case TOP:
		y -= 1;
		break;
	case LEFT_TOP:
		x -= 1;
		y -= 1;
		break;
	case LEFT:
		x -= 1;
		break;
	case LEFT_BOT:
		x -= 1;
		y += 1;
		break;
	case BOT:
		y += 1;
		break;
	case RIGHT_BOT:
		x += 1;
		y += 1;
		break;
	}
	if (inImage(y, x, input.h, input.w)) {
		return input.pixels[y][x];
	}
	return -1;
}

int getNeighborForMatch(Image input, int dir, int x, int y){
	return getNeighbor(input, dir, x, y) > 0 ? 1:0;
}

void setNeighbor(Image input, int dir, int x, int y, int val)
{
	switch (dir) {
	case RIGHT:
		x += 1;
		break;
	case RIGHT_TOP:
		x += 1;
		y -= 1;
		break;
	case TOP:
		y -= 1;
		break;
	case LEFT_TOP:
		x -= 1;
		y -= 1;
		break;
	case LEFT:
		x -= 1;
		break;
	case LEFT_BOT:
		x -= 1;
		y += 1;
		break;
	case BOT:
		y += 1;
		break;
	case RIGHT_BOT:
		x += 1;
		y += 1;
		break;
	}
	if (inImage(y, x, input.h, input.w)) {
		input.pixels[y][x] = val;
	}
}

void setRect(Rect *rect, int x, int y, int w, int h) {
    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = h;
}

void addPointToRect(Rect *rect, int x, int y) {
    int right = rect->x + rect->w - 1;
    int bottom = rect->y + rect->h - 1;

    rect->x = rect->x > x ? x: rect->x;
    rect->y = rect->y > y ? y: rect->y;

    rect->w = right < x ? x - rect->x + 1: right - rect->x + 1;
    rect->h = bottom < y ? y - rect->y + 1: bottom - rect->y + 1;
}

bool IsPointInImage(POINT point, int h, int w)
{
	bool result = false;

	if ((point.x >= 0) && (point.x < w) &&
		(point.y >= 0) && (point.y < h))
	{
		result = true;
	}

	return result;
}

int GetPixelValue(Image inputImage, POINT point)
{
	int result = -1;

	if (IsPointInImage(point, inputImage.h, inputImage.w))
	{
		result = inputImage.pixels[point.y][point.x];
	}

	return result;
}

void SetPixelValue(Image inputImage, POINT point, int pixelValue)
{
	if (IsPointInImage(point, inputImage.h, inputImage.w))
	{
		inputImage.pixels[point.y][point.x] = pixelValue;
	}
}

int GetNeighborPixelValue(Image inputImage, POINT point, int dir)
{
	int result = -1;
	POINT neighborPoint;
	
	neighborPoint = GetNeighborPoint(point, dir);
	result = GetPixelValue(inputImage, neighborPoint);

	return result;
}

// get a pixel point
PPOINT GetPixelPoint(Image inputImage, POINT point)
{
	PPOINT pResultPoint = NULL;

	if (IsPointInImage(point, inputImage.h, inputImage.w))
	{
		if (inputImage.pixels[point.y][point.x] == 1)
		{
			pResultPoint = (PPOINT)malloc(sizeof(POINT));
			SetPoint(pResultPoint, point.x, point.y);
		}
	}

	return pResultPoint;
}

// set a pixel point
void SetPixelPoint(Image *pImage, POINT point)
{
	if (IsPointInImage(point, pImage->h, pImage->w))
	{
		pImage->pixels[point.y][point.x] = 1;
	}
}

// clear a pixel point
void ClearPixelPoint(Image *pImage, POINT point)
{
	if (IsPointInImage(point, pImage->h, pImage->w))
	{
		pImage->pixels[point.y][point.x] = 0;
	}
}

// get a neighbor point in a specific direction
POINT GetNeighborPoint(POINT point, int dir)
{
	POINT resultPoint;

	switch (dir)
	{
		case RIGHT:
		case RIGHT_TOP:
		case RIGHT_BOT:
			resultPoint.x = point.x + 1;
			break;
		case LEFT:
		case LEFT_TOP:
		case LEFT_BOT:
			resultPoint.x = point.x - 1;
			break;
		default: // TOP & BOT
			resultPoint.x = point.x;
			break;
	}
	switch (dir)
	{
		case TOP:
		case LEFT_TOP:
		case RIGHT_TOP:
			resultPoint.y = point.y - 1;
			break;
		case BOT:
		case LEFT_BOT:
		case RIGHT_BOT:
			resultPoint.y = point.y + 1;
			break;
		default: // LEFT & RIGHT
			resultPoint.y = point.y;
			break;
	}

	return resultPoint;
}

// get a neighbor pixel point from 8 direction
// of a point in a binary image
PPOINT GetNeighborPixelPoint(Image inputImage, POINT point, bool *isFullyWalked)
{
	int dir;
	PPOINT pResultPoint = NULL;

	*isFullyWalked = false;
	for (dir = 0; dir < NUM_DIR; dir++)
	{
		pResultPoint = GetPixelPoint(inputImage, GetNeighborPoint(point, dir));
		if (pResultPoint != NULL)
		{
			break;
		}
	}
	if (dir >= (NUM_DIR-1))
	{
		// point not found or found at right bottom (the last direction to check)
		*isFullyWalked = true;
	}

	return pResultPoint;
}

// get a neighbor pixel point from 8 direction
// of a point in a binary image
PPOINT GetNeighborPixelPointOnMatch(Image inputImage, POINT point, int pixelValue, int *neighborDir)
{
	int dir;
	PPOINT pResultPoint = NULL;
	POINT neighborPoint;

	*neighborDir = -1;
	for (dir = 0; dir < NUM_DIR; dir++)
	{
		neighborPoint = GetNeighborPoint(point, dir);
		if (GetPixelValue(inputImage, neighborPoint) == pixelValue)
		{
			pResultPoint = (PPOINT)malloc(sizeof(POINT));
			SetPoint(pResultPoint, neighborPoint.x, neighborPoint.y);
			*neighborDir = dir;
			break;
		}
	}

	return pResultPoint;
}

// copy an image within a rect
void CopyImageWithinRect(Image *dst, Image src, RECT rect, bool binarize)
{
	for (int y = 0; y < rect.h; y++)
	{
		for (int x = 0; x < rect.w; x++)
		{
			if (!binarize)
			{
				dst->pixels[y + rect.y][x + rect.x] =
						src.pixels[y + rect.y][x + rect.x];
			}
			else
			{
				dst->pixels[y + rect.y][x + rect.x] =
						(src.pixels[y + rect.y][x + rect.x] != 0) ? 1 : 0;
			}
		}
	}
}

// copy an image within a list of points
void CopyImageWithinList(Image *dst, Image src, PPOINT pPointList, int nPoints, bool binarize)
{
	for (int i = 0; i < nPoints; i++)
	{
		if (!binarize)
		{
			dst->pixels[pPointList[i].y][pPointList[i].x] =
					src.pixels[pPointList[i].y][pPointList[i].x];
		}
		else
		{
			dst->pixels[pPointList[i].y][pPointList[i].x] =
					(src.pixels[pPointList[i].y][pPointList[i].x] != 0) ? 1 : 0;
		}
	}
}

Image CropImageWithinList(Image src, PPOINT pPointList, int nPoints, RECT rect, bool binarize)
{
	Image resultImage;

	resultImage = CreateImage(rect.h, rect.w);

	for (int i = 0; i < nPoints; i++)
	{
		if (!binarize)
		{
			resultImage.pixels[pPointList[i].y - rect.y][pPointList[i].x - rect.x] =
					src.pixels[pPointList[i].y][pPointList[i].x];
		}
		else
		{
			resultImage.pixels[pPointList[i].y - rect.y][pPointList[i].x - rect.x] =
					(src.pixels[pPointList[i].y][pPointList[i].x] != 0) ? 1 : 0;
		}
	}

	return resultImage;
}

// set a rect to a specific position
// with specific size
void SetRect(PRECT pRect, POINT point, int h, int w) 
{
    pRect->x = point.x;
    pRect->y = point.y;
    pRect->h = h;
    pRect->w = w;
}

// copy rect details
void CopyRect(PRECT pRect, RECT rect)
{
	pRect->x = rect.x;
	pRect->y = rect.y;
	pRect->h = rect.h;
	pRect->w = rect.w;
}

// add a point to a rect
// update the coordinate as well as size of the rect
void AddPointToRect(PRECT pRect, POINT point)
{
	if (point.x > (pRect->x + pRect->w - 1)) // right side
	{
		pRect->w = point.x - pRect->x + 1;
	}
	else if (point.x < pRect->x) // left side
	{
		pRect->w += pRect->x - point.x + 1;
		pRect->x = point.x;
	}
	// else: inside

	if (point.y > (pRect->y + pRect->h - 1)) // bottom side
	{
		pRect->h = point.y - pRect->y + 1;
	}
	else if (point.y < pRect->y) // top side
	{
		pRect->h += pRect->y - point.y + 1;
		pRect->y = point.y;
	}
	// else: inside
}

// set a point to a specific position
void SetPoint(PPOINT pPoint, int x, int y)
{
	pPoint->x = x;
	pPoint->y = y;
}
