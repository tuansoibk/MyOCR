#include "segment.h"
#include "common.h"
#include <math.h>
#define MIN_LINE_HEIGHT 6
#define MIN_LINE_WIDTH 10
#define NO_LINE_FOUND -1

// define min, max char's with, height base on 
// image's width, height
#define MIN_CHAR_WIDTH(w) ((w) / 16)
#define MAX_CHAR_WIDTH(w) ((w) / 4)
#define MIN_CHAR_HEIGHT(h) ((h) / 8)
#define MAX_CHAR_HEIGHT(h) ((h) / 2)

int curRow = 0;
Rect *charRect;
Rect *segmentRect;
int *lineGroup;
extern int *walkedPos;
int nWalked = 0;

/** Local functions */
int ccLabeling(Image input);
int checkRightConnect(Image input, int col);

// check if a char is belong to a line & return the line number
// if no line found, return -1
int GetLineOfChar(RECT charRect, PRECT *pLineRectList, int *pCharNumberList, int nLines)
{
	int resultLine = NO_LINE_FOUND;
	RECT tempCharRect;
	int hDistance;
	int vDistance;

	for (int line = 0; line < nLines; line++)
	{
		for (int chr = 0; chr < pCharNumberList[line]; chr++)
		{
			CopyRect(&tempCharRect, pLineRectList[line][chr]);
			//if ((charRect.y >= tempCharRect.y) && 
			//		(charRect.y <= (tempCharRect.y + tempCharRect.h - 1)))
			//{
			//	hDistance = (charRect.x - tempCharRect.x);
			//	hDistance = (hDistance > 0) ? hDistance : -hDistance;
			//	if (hDistance < (charRect.w * 2))
			//	{
			//		// line found
			//		resultLine = line;
			//		break;
			//	}
			//}
			//else if ((charRect.y <= tempCharRect.y) &&
			//		((charRect.y + charRect.h - 1) >= tempCharRect.y))
			//{
			//	hDistance = (charRect.x - tempCharRect.x);
			//	hDistance = (hDistance > 0) ? hDistance : -hDistance;
			//	if (hDistance < (charRect.w * 2))
			//	{
			//		// line found
			//		resultLine = line;
			//		break;
			//	}
			//}

			vDistance = charRect.y - tempCharRect.y;
			vDistance = (vDistance > 0) ? vDistance : -vDistance;
			hDistance = charRect.x - tempCharRect.x;
			hDistance = (hDistance > 0) ? hDistance : -hDistance;
			if ((vDistance < (charRect.y / 2)) && (hDistance < charRect.w * 2))
			{
				resultLine = line;
				break;
			}
		}

		if (resultLine != NO_LINE_FOUND)
		{
			break;
		}
	}

	return resultLine;
}

// sort the char rect in a line
void SortCharRect(PRECT pRectList, Image *pCharImageList, int nRects)
{
	RECT tempRect;
	Image tempImage;

	for (int i = 0; i < nRects; i++)
	{
		for (int j = i + 1; j < nRects; j++)
		{
			if (pRectList[i].x > pRectList[j].x)
			{
				CopyRect(&tempRect, pRectList[i]);
				CopyRect(&(pRectList[i]), pRectList[j]);
				CopyRect(&(pRectList[j]), tempRect);
				tempImage = CopyImage(pCharImageList[i]);
				pCharImageList[i] = CopyImage(pCharImageList[j]);
				pCharImageList[j] = CopyImage(tempImage);
			}
		}
	}
}

// remove all component with invalid size
// out of the binary image
Image FilterByCharSize(Image inputImage, Image **pLineImageList,
		Image ***pCharImageList, int **pCharNumberList, int *nLines)
{
	int h = inputImage.h;
	int w = inputImage.w;
	Image resultImage;
	PRECT *pLineRectList;
	PPOINT pWalkedList;
	bool *pFullyWalkedList;
	int walkedStep;
	int maxWalkedStep;
	RECT charRect;
	POINT currentPoint;
	PPOINT pNextPoint;

	POINTER_VALUE(pLineImageList) = NULL;
	POINTER_VALUE(pCharImageList) = NULL;
	POINTER_VALUE(pCharNumberList) = NULL;
	pLineRectList = NULL;
	POINTER_VALUE(nLines) = 0;

	h = inputImage.h;
	w = inputImage.w;
	pWalkedList = (PPOINT)malloc(h * w * sizeof(POINT));
	pFullyWalkedList = (bool*)calloc(h * w, sizeof(bool));
	resultImage = CreateImage(h, w);

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			if (inputImage.pixels[y][x] == 1) // found a point that has not walked yet
			{
				walkedStep = 0;
				maxWalkedStep = 0;
				SetPoint(&currentPoint, x, y);
				SetRect(&charRect, currentPoint, 1, 1);
				SetPoint(&(pWalkedList[walkedStep]), x, y);
				while (1) // loop
				{
					if (walkedStep < 0)
					{
						// we have found all pixel connected to current component
						// get out of the loop
						break;
					}
					// mark the point as walked
					inputImage.pixels[currentPoint.y][currentPoint.x] = 2;
					pNextPoint = GetNeighborPixelPoint(inputImage, currentPoint, &(pFullyWalkedList[walkedStep]));
					if (pNextPoint != NULL)
					{
						walkedStep = maxWalkedStep + 1;
						SetPoint(&currentPoint, pNextPoint->x, pNextPoint->y);
						SetPoint(&(pWalkedList[walkedStep]), currentPoint.x, currentPoint.y);
						AddPointToRect(&charRect, currentPoint);
						walkedStep++;
						maxWalkedStep++;
					}
					else
					{
						// point is fully walked
						pFullyWalkedList[walkedStep] = true;
						// go back 1 step to find a not fully walked point
						walkedStep--;
						SetPoint(&currentPoint, pWalkedList[walkedStep].x, pWalkedList[walkedStep].y);
					}
				}

				if ((maxWalkedStep <= (MAX_CHAR_HEIGHT(h) * MAX_CHAR_WIDTH(w))) &&
					(charRect.h <= MAX_CHAR_HEIGHT(h)) && (charRect.h >= MIN_CHAR_HEIGHT(h)) &&
					(charRect.w <= MAX_CHAR_WIDTH(w)) && (charRect.w >= MIN_CHAR_WIDTH(w)))
				{
					// found a character rect
					// copy this char to result image
					/*for (int i = 0; i < maxWalkedStep; i++)
					{
						resultImage.pixels[pWalkedList[i].y][pWalkedList[i].x] = 
							inputImage.pixels[pWalkedList[i].y][pWalkedList[i].x];
					}*/
					CopyImageWithinList(&resultImage, inputImage, pWalkedList, maxWalkedStep, true);
					
					// search char line & store
					if (POINTER_VALUE(nLines) == 0)
					{
						POINTER_VALUE(nLines)++;
						POINTER_VALUE(pLineImageList) = (Image*)malloc(sizeof(Image));
						POINTER_VALUE(pLineImageList)[0] = CreateImage(h, w);
						CopyImageWithinList(&(POINTER_VALUE(pLineImageList)[0]), inputImage,
								pWalkedList, maxWalkedStep, true);
						POINTER_VALUE(pCharImageList) = (Image**)malloc(sizeof(Image*));
						POINTER_VALUE(pCharImageList)[0] = (Image*)malloc(sizeof(Image));						
						POINTER_VALUE(pCharImageList)[0][0] = CropImageWithinList(inputImage,
								pWalkedList, maxWalkedStep, charRect, true);
						pLineRectList = (PRECT*)malloc(sizeof(PRECT));
						pLineRectList[0] = (PRECT)malloc(sizeof(RECT));
						CopyRect(&(pLineRectList[0][0]), charRect);
						POINTER_VALUE(pCharNumberList) = (int*)malloc(sizeof(int));
						POINTER_VALUE(pCharNumberList)[0] = 1;
					}
					else
					{
						int charLine;

						charLine = GetLineOfChar(charRect, pLineRectList,
								POINTER_VALUE(pCharNumberList), POINTER_VALUE(nLines));
						if (charLine != NO_LINE_FOUND)
						{
							POINTER_VALUE(pCharNumberList)[charLine]++;
							CopyImageWithinList(&(POINTER_VALUE(pLineImageList)[charLine]),
									inputImage, pWalkedList, maxWalkedStep, true);
							POINTER_VALUE(pCharImageList)[charLine] =
									(Image*)realloc(POINTER_VALUE(pCharImageList)[charLine],
									POINTER_VALUE(pCharNumberList)[charLine] * sizeof(Image));
							POINTER_VALUE(pCharImageList)[charLine][POINTER_VALUE(pCharNumberList)[charLine] - 1] =
									CropImageWithinList(inputImage, pWalkedList, maxWalkedStep, charRect, true);
							pLineRectList[charLine] =
									(PRECT)realloc(pLineRectList[charLine],
									POINTER_VALUE(pCharNumberList)[charLine] * sizeof(RECT));
							CopyRect(&(pLineRectList[charLine][POINTER_VALUE(pCharNumberList)[charLine] - 1]),
									charRect);
						}
						else
						{
							// create a new line
							POINTER_VALUE(nLines)++;
							POINTER_VALUE(pLineImageList) = (Image*)realloc(POINTER_VALUE(pLineImageList),
									POINTER_VALUE(nLines) * sizeof(Image));
							POINTER_VALUE(pLineImageList)[POINTER_VALUE(nLines) - 1] = CreateImage(h, w);
							CopyImageWithinList(&(POINTER_VALUE(pLineImageList)[POINTER_VALUE(nLines) - 1]),
									inputImage, pWalkedList, maxWalkedStep, true);
							POINTER_VALUE(pCharImageList) = (Image**)realloc(POINTER_VALUE(pCharImageList),
									POINTER_VALUE(nLines) * sizeof(Image*));
							POINTER_VALUE(pCharImageList)[POINTER_VALUE(nLines) - 1] =
									(Image*)malloc(sizeof(Image));
							POINTER_VALUE(pCharImageList)[POINTER_VALUE(nLines) - 1][0] =
									CropImageWithinList(inputImage, pWalkedList, maxWalkedStep, charRect, true);
							pLineRectList = (PRECT*)realloc(pLineRectList,
									POINTER_VALUE(nLines) * sizeof(PRECT));
							pLineRectList[POINTER_VALUE(nLines) - 1] =
									(PRECT)malloc(sizeof(RECT));
							CopyRect(&(pLineRectList[POINTER_VALUE(nLines) - 1][0]),
									charRect);
							POINTER_VALUE(pCharNumberList) = (int*)realloc(POINTER_VALUE(pCharNumberList),
									POINTER_VALUE(nLines) * sizeof(int));
							POINTER_VALUE(pCharNumberList)[POINTER_VALUE(nLines) - 1] = 1;
						}
					}
				}
			}
		}
	}

	if (nLines > 0)
	{
		for (int i = 0; i < POINTER_VALUE(nLines); i++)
		{
			SortCharRect(pLineRectList[i], POINTER_VALUE(pCharImageList)[i], POINTER_VALUE(pCharNumberList)[i]);
		}
	}

	free(pWalkedList);
	free(pFullyWalkedList);

	return resultImage;
}

int GetBaseLine(Image **pCharImageList, int *pCharNumberList, int nLines)
{
	int sum = 0;
	int nChars = 0;
	int resultBaseLine;

	for (int line = 0; line < nLines; line++)
	{
		for (int chr = 0; chr < pCharNumberList[line]; chr++)
		{
			sum += pCharImageList[line][chr].h;
			nChars++;
		}
	}
	resultBaseLine = sum / nChars;

	sum = 0;
	nChars = 0;
	for (int line = 0; line < nLines; line++)
	{
		for (int chr = 0; chr < pCharNumberList[line]; chr++)
		{
			if (pCharImageList[line][chr].h < resultBaseLine)
			{
				sum += pCharImageList[line][chr].h;
				nChars++;
			}
		}
	}
	resultBaseLine = sum / nChars;

	return resultBaseLine;
}

// get max column density in a specific angle
int RateSlantAngle(Image lineImage, double angle, int threshold)
{
	int colDen = 0;
	int maxDen = 0;
	int new_x;
	double factor;
	POINT point;
	PPOINT pPoint;
	int result = 0;

	factor = tan(angle);
	pPoint = (PPOINT)malloc(sizeof(POINT));
	for (int x = 0; x < lineImage.w; x++)
	{
		colDen = 0;
		maxDen = 0;
		for (int y = 0; y < lineImage.h; y++)
		{
			new_x = x + y * factor;
			SetPoint(&point, new_x, y);
			pPoint = GetPixelPoint(lineImage, point);
			if (pPoint != NULL)
			{
				colDen++;
			}
			else
			{
				maxDen = (colDen > maxDen) ? colDen : maxDen;
				colDen = 0;
			}
		}
		
		if (maxDen > threshold)
		{
			result += maxDen;
		}
	}

	return result;
}

double GetSlantAngle(Image *pLineImageList, int nLines, int baseLine)
{
	double resultAngle = 0;
	double slantAngle;
	int rate = 0;
	int maxRate = 0;

	for (int i = -30; i <= 30; i += 5)
	{
		slantAngle = (((double)i) * PI) / 180;
		rate = 0;
		for (int line = 0; line < nLines; line++)
		{
			rate += RateSlantAngle(pLineImageList[line], slantAngle, baseLine * 3 / 4);
		}

		if (rate > maxRate)
		{
			maxRate = rate;
			resultAngle = slantAngle;
		}
	}

	return resultAngle;
}

int checkRectWalked(int index) {
    int i;
    for(i = 0; i < nWalked; i++){
        if(walkedPos[i] == index) return 1;
    }
    return 0;
}

void addToWalkedList(int index) {
    nWalked++;
    walkedPos = (int*)realloc(walkedPos, nWalked*sizeof(int));
    walkedPos[nWalked - 1] = index;
}

double getDistance(int x1, int y1, int x2, int y2) {
    return sqrt((double)(x2 - x1)*(x2-x1) + (y2 - y1)*(y2 - y1));
}

int* lineDensity(Image binImg) {
    int i, j;

    int* lnDen = (int*) malloc(binImg.h * sizeof(int));
    for (i = 0; i < binImg.h; i ++) {
        lnDen[i] = 0;
        for (j = 0; j < binImg.w; j++)
            lnDen[i] += binImg.pixels[i][j];
        //printf("%d ",lnDen[i]);
    }
    return lnDen;
}

int getWordSegment(int* colDen, int maxCol, int boxHeight, int *start_w, int * end_w) {
    int i, count;

    count = 0;
    i = *start_w;
    while ((i < maxCol) && (colDen[i] == 0))
        i++;
    if(i >= maxCol) return 0;
    *start_w = i;

    i++;
    while (i < maxCol) {
        if (colDen[i] != 0) {
            count = 0;
        } else
            count ++;

        if (count > boxHeight/ 2) {
            break;
        }
        i++;
    }
    *end_w = i + 1 - count;
    if ((*end_w - *start_w) > 1)
        return 1;
    return 0;
}

//int getCharSegment(int* colDen, int* curCol, int maxCol, int boxHeight, int boxWidth, int *start, int *end)
//{
//	int i = *curCol;
//	int min, max, loop;
//
//	loop = 1;
//	while (loop) {
//		loop = 0;
//
//		while ((i < maxCol) && (colDen[i] == 0)) i++;
//		// calc max min density
//		max = min = colDen[i];
//		//printf("%d, %d\n", i, min);
//		*start = i;
//		while ((i < maxCol)&&(colDen[i] > 0)&&(i - *start < boxWidth)) {
//			i++;
//			if (colDen[i] < min) {
//				*end = i;
//				min = colDen[i];
//			}
//			max = max > colDen[i] ? max : colDen[i];
//		}
//		//printf("max = %d\n", max);
//
//		// recheck
//		if ((*end <= *start)&& min > 0) {
//			loop = 1;
//			i = *start + 1;
//		} else if ((*end > *start)&&(max < boxHeight/4)) {	// reject that box
//			loop = 1;
//			i = *end;
//		}
//	}
//	*curCol = *end;
//	return (*end > *start)? 1: 0;
//}

/*
int getCharSegment(Image wordImg, int *colDen, int boxHeight, int* start_c, int* end_c) {
    int i, maxDen;
    int maxCol = wordImg.w;

    i = *start_c;
    while (1) {
        while ((i < maxCol) && (colDen[i] == 0))
            i++;
//        printf("i = %d\n", i);
        if (i >= maxCol) break;

        maxDen = colDen[i];

        *start_c = i;
        while ((i < maxCol)&&(colDen[i] > 0)) {
            maxDen = maxDen > colDen[i] ? maxDen : colDen[i];
            if (!checkRightConnect(wordImg, i))
                break;
            i++;
        }

        *end_c = i + 1;

//        printf("start = %d, end = %d\n", *start_c, *end_c);
        /// checking the segmented character
        if (maxDen > 0) {
            return 1;
        }

        if ((i >= maxCol) || (colDen[i] == 0)) {
            break;
        }
        i++;
    }
    return 0;
}
*/

/*
Rect getRectCharSegment(Image wordImg, int label) {
    int i, j, cont;
    Rect rect;

    j = 0;

    /// find x
    cont = 1;
    while((j < wordImg.w) && cont) {
        for(i = 0; i < wordImg.h; i++) {
            if(wordImg.pixels[i][j] == label){
                rect.x = j;
                cont = 0;
                break;
            }
        }
        j++;
    }

    /// find w
    cont = 1;
    while((j < wordImg.w) && cont) {
        cont = 0;
        for(i = 0; i < wordImg.h; i++) {
            if(wordImg.pixels[i][j] == label){
                cont = 1;
                break;
            }
        }
        j++;
    }

    rect.w = j - rect.x;

    /// find y
    cont = 1;
    while((i < wordImg.h) && cont) {
        for(j = 0; j < wordImg.w; j++) {
            if(wordImg.pixels[i][j] == label){
                rect.x = j;
                cont = 0;
                break;
            }
        }
        j++;
    }

    /// find w
    cont = 1;
    while((j < wordImg.w) && cont) {
        cont = 0;
        for(i = 0; i < wordImg.h; i++) {
            if(wordImg.pixels[i][j] == label){
                cont = 1;
                break;
            }
        }
        j++;
    }

    rect.w = j - rect.x;


    for(j = rect.x; j < input.w; j++) {
        for(
}
*/
/*
int calcCharSegment(Image wordImg) {
    int i, j, label, index = 0;
    int nLabel;

    /// connected component labeling
    nLabel = ccLabeling(wordImg);

    charRect = (Rect*) malloc(nLabel * sizeof(Rect));
    /// calc number of segments
    for(label = 2; label <= nLabel; label++) {
        for (j = 0; j < wordImg.w; j++) {
            for (i = 0; i < wordImg.h; i++) {
                if (input.pixels[i][j] == label) {
                    charRect[index].x = j;
                    break;
                }
            }
        }
    }
    return 0;
}
*/

int getlineSegment(int* lnDen, int maxRow, int *start, int *end, int* extraHeight) {
    int i;
    int max, min;

    i = *start;
    while (1) {
//		printf("i = %d, max = %d\n", i, maxRow);
        while ((i < maxRow)&&(lnDen[i] <= MIN_LINE_WIDTH)) i++;
        if (i >= maxRow) return 0;
        // calc max and min density
        max = min = lnDen[i];
        *start = i;
        while ((i++ < maxRow)&&(lnDen[i] > MIN_LINE_WIDTH)) {
            max = max > lnDen[i] ? max : lnDen[i];
            min = min < lnDen[i] ? min : lnDen[i];
        }
        *end = i;

        // calc start and end of line
        int threshold = (max + 2*min)/3;
//        printf("threshold = %d\n", threshold);

        i = *start;
        while (lnDen[i] < threshold) i++;
        *extraHeight = i - *start;
        *start = i;

        i = *end;
        while (lnDen[i] < threshold) i--;
        *end = i + 1;

        if (*end - *start > MIN_LINE_HEIGHT) {
            return 1;
        }
        i = *end + 1;
    }
    return 0;
}

int* colDensity(Image binImg, int start, int end) {
    int i, j;
    int* clDen = (int*) malloc(binImg.w *sizeof(int));
    for (j = 0; j < binImg.w; j ++) {
        clDen[j] = 0;
        for (i = start; i < end; i ++)
            clDen[j] += binImg.pixels[i][j];
        //printf("%d ", clDen[j]);
    }
    return clDen;
}

/**
 * Get box image: hStart <= y < hEnd, wStart <= x < wEnd
 */
Image getBoxImage(Image binImg, int hStart, int hEnd, int wStart, int wEnd) {
    Image boxImg;
    int h = hEnd - hStart;
    int w = wEnd - wStart;

    int i, j;

    boxImg = CreateImage(h, w);

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            if(inImage(hStart +i, wStart +j, binImg.h, binImg.w))
                boxImg.pixels[i][j] = binImg.pixels[hStart + i][wStart +j];
            else
                boxImg.pixels[i][j] = 0;
        }
    }
    return boxImg;
}


Image getCharImage(Image binImg, int hStart, int hEnd, int wStart, int wEnd) {
    int h = hEnd - hStart;
    int w = wEnd - wStart;
    int wPad = 0;
    /// modify boxwidth
    if (w < h/2) {
        wPad = h/2 - w;
        w = h/2;
    }
    int i, j;

    Image out = CreateImage(h, w);

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            if (j >= wPad/2 && j < w - wPad/2) {
                out.pixels[i][j] = binImg.pixels[hStart + i][wStart +j -wPad/2];
            } else out.pixels[i][j] = 0;
        }
    }
    return out;
}

Image getImageFromRect(Image binImg, Rect rect, int index) {
    int i, j;
    int h = rect.h;
    int w = rect.w;

    int wPad = 0;
    /// modify boxwidth
    if (w < h/2) {
        wPad = h/2 - w;
        w = h/2;
    }

    Image out = CreateImage(h, w);

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            if (j >= wPad/2 && j < w - wPad/2) {
                if(binImg.pixels[rect.y+i][rect.x + j - wPad/2] == 2+(index)%100)
                    out.pixels[i][j] = 1;
                else
                    out.pixels[i][j] = 0;
            }else
                out.pixels[i][j] = 0;
        }
    }
    return out;
}

int checkLeftConnect(Image input, int col) {
    int x, y, result;

    x = col;
    result = 0;

    for (y = 1; y < input.h -1; y++) {
        if (input.pixels[y-1][x-1] == 1) result = 1;
        if (input.pixels[y][x-1] == 1) result = 1;
        if (input.pixels[y+1][x-1] == 1) result = 1;
        if (result == 1) break;
    }
    return result;
}

int checkRightConnect(Image input, int col) {
    int x, y, result;

    x = col;
    result = 0;

    for (y = 1; y < input.h -1; y++) {
        if (input.pixels[y][x] == 1) {
            if (input.pixels[y-1][x+1] == 1) result = 1;
            if (input.pixels[y][x+1] == 1) result = 1;
            if (input.pixels[y+1][x+1] == 1) result = 1;
        }
        if (result == 1) break;
    }
    return result;
}

///--------------CONNECTED COMPONENT LABELING---------------


void recursiveLabeling(Image input, int x, int y, int label) {
    int dir;

    /// label it
    input.pixels[y][x] = 2 + label%100;
    addPointToRect(&charRect[label], x, y);

    for (dir = 0; dir < NUM_DIR; dir++) {
        if (getNeighbor(input,dir, x, y) == 1) {
//              printf("%d %d %d %d\n", charRect[label-2].x, charRect[label-2].y, charRect[label-2].w, charRect[label-2].h);
            switch (dir) {
            case RIGHT:
                recursiveLabeling(input, x+1, y, label);
                break;
            case RIGHT_TOP:
                recursiveLabeling(input, x+1, y-1, label);
                break;
            case TOP:
                recursiveLabeling(input, x, y-1, label);
                break;
            case LEFT_TOP:
                recursiveLabeling(input, x-1, y-1, label);
                break;
            case LEFT:
                recursiveLabeling(input, x-1, y, label);
                break;
            case LEFT_BOT:
                recursiveLabeling(input, x-1, y+1, label);
                break;
            case BOT:
                recursiveLabeling(input, x, y+1, label);
                break;
            case RIGHT_BOT:
                recursiveLabeling(input, x+1, y+1, label);
                break;
            }
        }
    }
}

int ccLabeling(Image input) {
    int i, j;
    int nLabel = 0;

    for (j = 0; j < input.w; j++) {
        for (i = 0; i < input.h; i++) {
            if (input.pixels[i][j] == 1) {
                nLabel ++;
                /// start component labeling
                charRect = (Rect*) realloc( charRect, nLabel*sizeof(Rect));
                setRect(&charRect[nLabel - 1], j, i, 0, 0);
                recursiveLabeling(input, j, i, nLabel-1);
//                printf("%d\n", nLabel);
            }
        }
    }
    /// remove noise
    for(i = 0; i < nLabel; i++){
        if(charRect[i].h < MIN_LINE_HEIGHT)
            addToWalkedList(i);
    }
    return nLabel;
}

int ccLineLabeling(Image input, int start) {
    int i, j, outLoop = 0;
    int nLabel = 0;
    int nSeg = 0;
    for (j = start; j < input.h; j++) {
        for(i = 0; i < input.w; i++) {
            if(input.pixels[j][i] == 1) {
                nLabel++;
                /// start component labeling
                charRect = (Rect*) realloc( charRect, nLabel*sizeof(Rect));
                setRect(&charRect[nLabel - 1], i, j, 0, 0);
                recursiveLabeling(input, i, j, nLabel-1);
                //printf("%d %d %d\n", j,i,charRect[nLabel -1].h);
                if(charRect[nLabel - 1].h < MIN_LINE_HEIGHT || charRect[nLabel-1].h/charRect[nLabel-1].w > 20) {
                    nLabel--;
                    continue;
                }
                else {
                    outLoop = 1;
                    break;
                }
            }
        }
        if(outLoop) break;
    }

    if(!outLoop) return nLabel;

    int cont = 1;
    int minHeight  = charRect[nLabel-1].h/6;
    if(minHeight < MIN_LINE_HEIGHT) minHeight = MIN_LINE_HEIGHT;

    while(cont) {
        cont = 0;
        outLoop = 0;
        for(i = charRect[nLabel - 1].x + charRect[nLabel - 1].w/3 - 1; i >=0; i--) {
            for(j = charRect[nLabel - 1].y; j < charRect[nLabel -1].y+charRect[nLabel-1].h; j++){
                if(input.pixels[j][i] ==1){
                    cont = 1;
                    nLabel++;
                    /// start component labeling
                    charRect = (Rect*) realloc( charRect, nLabel*sizeof(Rect));
                    setRect(&charRect[nLabel - 1], i, j, 0, 0);
                    recursiveLabeling(input, i, j, nLabel-1);
                    if(charRect[nLabel - 1].h < minHeight) {
                        nLabel--;
                        continue;
                    }
                    else {
                        outLoop = 1;
                        break;
                    }
                }
            }
            if(outLoop) break;
        }
    }
    /// save to line array
    nSeg = nLabel;
    lineGroup = (int*)realloc(lineGroup, nLabel*sizeof(int));
    for(i = nLabel-1; i >= 0; i--) {
        lineGroup[nLabel-1-i] = i;
    }

    outLoop = 0;
    for(i = charRect[0].x + charRect[0].w*2/3 + 1; i < input.w; i++) {
        for(j = charRect[0].y; j < charRect[0].y+charRect[0].h; j++){
            if(input.pixels[j][i] ==1){
                cont = 1;
                nLabel++;
                /// start component labeling
                charRect = (Rect*) realloc( charRect, nLabel*sizeof(Rect));
                setRect(&charRect[nLabel - 1], i, j, 0, 0);
                recursiveLabeling(input, i, j, nLabel-1);
                if(charRect[nLabel - 1].h < minHeight) {
                    nLabel--;
                    continue;
                }
                else {
                    outLoop = 1;
                    break;
                }
            }
        }
        if(outLoop) break;
    }

    if(!outLoop) return nLabel;

    /// continue from start label
    cont = 1;
    while(cont) {
        cont = 0;
        outLoop = 0;
//            printf("nLabel = %d\n", nLabel);

        for(i = charRect[nLabel - 1].x + charRect[nLabel - 1].w *2/3 - 1; i < input.w; i++) {
            for(j = charRect[nLabel - 1].y; j < charRect[nLabel -1].y+charRect[nLabel-1].h; j++){
                if(input.pixels[j][i] ==1){
                    cont = 1;
                    nLabel++;
                    /// start component labeling
                    charRect = (Rect*) realloc( charRect, nLabel*sizeof(Rect));
                    setRect(&charRect[nLabel - 1], i, j, 0, 0);
                    recursiveLabeling(input, i, j, nLabel-1);
                    if(charRect[nLabel - 1].h < minHeight) {
                        nLabel--;
                        continue;
                    }
                    else {
                        outLoop = 1;
                        break;
                    }
                }
            }
            if(outLoop) break;
        }
    }

    /// save to line array
    lineGroup = (int*)realloc(lineGroup, nLabel*sizeof(int));
    for(i = nSeg; i < nLabel; i++) {
        lineGroup[i] = i;
    }
    return nLabel;
}


Rect getCharRect(int label) {
    return charRect[label];
}

int getLeftTopRect(int nLabel) {
    double minDistance, distance;
    int i = 0;
    while((i < nLabel)&&checkRectWalked(i)) i++;
    if(i >= nLabel) return -1;
    int out = i;

    minDistance = getDistance(0, 0, charRect[i].x+charRect[i].w/2, charRect[i].y);

    i++;
    while(i < nLabel) {
        if(!checkRectWalked(i)&&
        minDistance > charRect[i].x&&
        minDistance > charRect[i].y){
            distance = getDistance(0, 0, charRect[i].x, charRect[i].y);
            if(minDistance > distance){
                out = i;
                minDistance = distance;
            }
        }
        i++;
    }

    return out;
}

int findNextLineSegment(int index, int nLabel){
    int i = index + 1;
    int threshold = charRect[index].h/4;
    int gX1 = charRect[index].x + charRect[index].w/2;
    int gY1 = charRect[index].y + charRect[index].h/2;
    int gX2, gY2;
    int out = -1;
    double skewAngle;
    double minDistance = 1000, distance;

    while(i < nLabel){
        gX2 = charRect[i].x + charRect[i].w/2;
        gY2 = charRect[i].y + charRect[i].h/2;
        if(checkRectWalked(i)||
            (charRect[i].x - charRect[index].x-charRect[index].w) > minDistance||
            (gY2- gY1) > minDistance) {
            i++;
            continue;
        }
        if(gX2 - gX1 > 0) {
            skewAngle = (double) (gY2 - gY1)/ (gX2 - gX1);
            if(skewAngle < 2 && skewAngle > -2) {/// in line
                if(charRect[i].h > threshold) {
                    distance = getDistance(charRect[index].x +charRect[index].w, gY1,
                    charRect[i].x, gY2);
                    if(distance < minDistance) {
                        minDistance = distance;
                        out = i;
                    }
                }
                else
                    addToWalkedList(i);
            }
        }
        i++;
    }
    return out;
}

int lineGrouping(int nLabel) {
    int index;
    int nSeg = 0;
    if((index = getLeftTopRect(nLabel)) != -1) {
        do {
            //printf("%d\n", index);
            nSeg++;
            lineGroup = (int*)realloc(lineGroup, nSeg*sizeof(int));
            lineGroup[nSeg - 1] = index;

            addToWalkedList(index);
        } while((index = findNextLineSegment(index, nLabel)) != -1);

    }
    return nSeg;
}

int getSegmentIndex(int index){
    return lineGroup[index];
}

int getBaselineHeight(int nLine){
    int i, h, avgH;
    int sum = 0, num = 0;
    for(i = 0; i < nLine; i++) {
        sum += charRect[(lineGroup[i])].h;
        num++;
    }
    avgH = sum/num;

    sum = 0;
    num = 0;

    for(i = 0; i < nLine; i++) {
        h = charRect[(lineGroup[i])].h;
        if(h <= avgH) {
            sum += h;
            num++;
        }
    }
    return sum /num;
}

int getAverageSpace(int nLine) {
    int i, maxS, minS;
	//int space[nLine-1];
	int space[10];
    /// calc space
    for(i = 1; i < nLine; i++) {
        space[i] = charRect[(lineGroup[i])].x - charRect[(lineGroup[i-1])].x - charRect[(lineGroup[i-1])].w;
        if(space[i] <0) space[i] = -space[i];
    }

    maxS = minS = space[0];
    for(i = 2; i < nLine; i++) {
        maxS = maxS > space[i] ? maxS :space[i];
        minS = minS < space[i] ? minS :space[i];
    }
    /// clustering
    float m1 = minS;
    float m2 = maxS;
    float new_m1 = m1, new_m2 = m2, dif_m1, dif_m2;
    int num_m1, num_m2;

    do {
        m1 = new_m1;
        m2 = new_m2;

        new_m1 = 0;
        new_m2 = 0;
        num_m1 = 0;
        num_m2 = 0;
        for (i = 1; i < nLine; i++) {
            dif_m1 = fabs(space[i] - m1);
            dif_m2 = fabs(space[i] - m2);

            if(dif_m1 < dif_m2){ /// belong to 1 set
                new_m1 += space[i];
                num_m1 ++;
            }
            else { /// belong to set 2
                new_m2 += space[i];
                num_m2 ++;
            }
        }
        if(num_m1)
            new_m1 /= num_m1;
        if(num_m2)
            new_m2 /= num_m2;
    }while (new_m1 != m1);

    return (int )(m1 + m2) /2;
}

int getSpace(int index, int nLine, Image input) {
    if(index >= nLine - 1) return 1000;
    int i, j;
    int rectIndex1 = lineGroup[index];
    int rectIndex2 = lineGroup[index+1];

    int startY = charRect[rectIndex1].y > charRect[rectIndex2].y ?
                charRect[rectIndex1].y : charRect[rectIndex2].y;
    int endY = charRect[rectIndex1].y + charRect[rectIndex1].h < charRect[rectIndex2].y + charRect[rectIndex2].h ?
                charRect[rectIndex1].y + charRect[rectIndex1].h :charRect[rectIndex2].y + charRect[rectIndex2].h;
    int startX1 = charRect[rectIndex1].x;
    int endX1 = charRect[rectIndex1].x + charRect[rectIndex1].w;
    int startX2 = charRect[rectIndex2].x;
    int endX2 = charRect[rectIndex2].x + charRect[rectIndex2].w;
    int rightX1, leftX2, space = 1000;

    for (i = startY; i < endY; i++) {
        j = endX1 -1;
        while((j > startX1) && (input.pixels[i][j] != 2+(rectIndex1)%100)) j--;
        rightX1 = j;

        j = startX2;
        while((j < endX2-1) && (input.pixels[i][j] != 2+(rectIndex2)%100)) j++;
        leftX2 = j;
        if(space > leftX2 - rightX1)
            space = leftX2 - rightX1;
    }

    return space;
}

int ratingAngle(Image input, double angle, int threshold)
{
	int i, j, new_j;
	int colDen, maxDen = 0, rate = 0;

	for (j = 0; j < input.w; j ++) {
		colDen = 0;
		for (i = 0; i < input.h; i ++){
			new_j = j - (int)(i* tan(angle));

			if(new_j >= 0 && new_j < input.w &&(input.pixels[i][new_j])){
				colDen ++;
			} else {
                maxDen = colDen > maxDen? colDen:maxDen;
                colDen = 0;
			}
		}
//		printf("colD = %d", colDen);
		if(maxDen > threshold) {
		    rate += colDen;
		}
	}
	return rate;
}

/**
 * Slant removal
 */

//Image slantRemoval(Image input, int start_l, int end_l, int start_w, int end_w)
//{
//	double angle;
//	int deg, slantDeg = 0;
//	int rate, max = 0;
//	int i, j, box_i, box_j;
//	int boxHeight = end_l - start_l;
//	int boxWidth = end_w - start_w;
//
//    /// find angle
//	for(deg = -30; deg <= 30; deg = deg + 5){
//		angle = PI * deg / 180;
//		rate = ratingAngle(input, start_l, end_l, start_w, end_w, angle);
////		printf("rate = %d\n", rate);
//		// calc num of max dens
//		if(max < rate){
//			max = rate;
//			slantDeg = deg;
//		}
//	}
//	printf("slantDeg = %d\n", slantDeg);
//	angle = PI * slantDeg/ 180;
//
//	/// slant remove
//	boxWidth += (int)(boxHeight*tan(angle));
//	Image box = createImage(boxHeight * 3, boxWidth);
//
//	for(i = start_l - boxHeight; i < end_l + boxHeight; i ++){
//		for(j = start_w; j < end_w; j++){
//			if(inImage(i, j, input.h, input.w)){
//                if(input.pixels[i][j]){
//                    box_i = i - start_l + boxHeight;
//                    box_j = j - start_w + (int)((box_i)* tan(angle));
//
//                    if(box_j >=0 && box_j < boxWidth){
//                        box.pixels[box_i][box_j] = 1;
//                        //printf("%d %d\n", box_i, box_j);
//                    }
//                }
//			}
//		}
//	}
//	return box;
//}


int getSlantDeg(Image binImg, int baseLine, int nLine) {
    int i, rectIndex;
    int deg, slantDeg = 0;
    int rate, max = 0;
    double angle;

    //Image rectImg[nLine];
    Image *rectImg = (Image*)calloc(nLine, sizeof(Image));
    for(i = 0; i < nLine; i++) {
        rectIndex = lineGroup[i];
		rectImg[i] = getImageFromRect(binImg, charRect[rectIndex], rectIndex);
    }
    /// find angle
	for(deg = -30; deg <= 30; deg = deg + 5){
		rate = 0;
		angle = PI * deg / 180;
		for(i = 0; i < nLine; i++) {
            rate += ratingAngle(rectImg[i], angle, baseLine*5/4);
		}
		// calc num of max dens
		if(max < rate){
			max = rate;
			slantDeg = deg;
		}
	}
//	printf("slantDeg = %d\n", slantDeg);
	return slantDeg;

}

/**
 * Classify character into three groups

int classifyChar(Image charImg, int extraHeight) {
    int i, j;
    int upperLen = 0, lowerLen = 0;
    int h = charImg.h;
    int w = charImg.w;

    int threshold = extraHeight/3;

    int *lnDen = lineDensity(charImg);
    for (i = h/3; i >= 0; i--) {
        if (lnDen[i] > 0) upperLen ++;
        else break;
    }

    if (upperLen > threshold) {
        return UPPER;
    }

    for (i = h *2/3; i < h; i++) {
        if (lnDen[i] > 0) lowerLen ++;
        else break;
    }

    if (lowerLen > threshold) {
        return LOWER;
    }
    return NORM;
}

int classifyChar(Image charImg, int extraHeight, int boxHeight) {
    int start, end, charHeight;
    int h = charImg.h;
    int w = charImg.w;

    int threshold = extraHeight/3;
    int *lnDen = lineDensity(charImg);

    start = 0;

    while(1) {
        /// calc height of char
        while((start < h) && lnDen[start] == 0) start++;
        if(start >= h) return -1;

        end = start;
        while((end < h) && lnDen[end] > 0) end ++;

        charHeight = end - start;

        if(charHeight < boxHeight*4/5) {
            start = end;
            continue;
        }else {
//            printf("%d\n", charHeight);

            break;
        }
    }
    /// detect upper and lower
    if(charHeight > boxHeight + threshold){
        if(h - end > start ) return UPPER;
        else return LOWER;
    }
    return NORM;
}*/

int classifyChar(Rect rect, int extraHeight,  int boxHeight){
    int threshold = extraHeight/3;
    if(rect.h > boxHeight + threshold) {
        if(boxHeight *3 - rect.y - rect.h > rect.y) return UPPER;
        else return LOWER;
    }
    return NORM;
}

int wordOverSegment(Image wordImg) {
    int i, j;
    int h = wordImg.h;
    int w = wordImg.w;
    int segPos = 0;
    int segWidth = h / 20;
    int numSeg = w / segWidth;
    int firstSegPoint, cutWidth;
    int threshold = h / 10;

    //int segmentPoint[numSeg];
	int *segmentPoint = (int*)calloc(numSeg, sizeof(int));


    for(i = 0; i < numSeg; i ++) {
        segmentPoint[i] = 0;
        j = 0;
        while((j < h) && (wordImg.pixels[j][segPos] == 0)) j++;

        if(j >= h) {
            segmentPoint[i] = 1;
            segPos += segWidth;
            continue;
        }
        firstSegPoint = j;

        /// find the last segPoint
        j = h - 1;
        while((j > 0) && (wordImg.pixels[j][segPos] == 0)) j--;

        cutWidth = j - firstSegPoint;

        /// reject loop segment
        if(cutWidth > threshold)
            segmentPoint[i] = -1;
        else
            segmentPoint[i] = firstSegPoint;
        segPos+= segWidth;
    }

    /// choose segment base on valley
    i = 0;
    int valley, maxSeg;
    while (i++ < numSeg) {
        if(segmentPoint[i] > 0) {
            valley = i;
            maxSeg = segmentPoint[i];
            segmentPoint[i] = -1;
            i++;
            while ((segmentPoint[i] > 0)&&(i < numSeg)) {
                if(segmentPoint[i] > maxSeg){
                    valley = i;
                    maxSeg = segmentPoint[i];
                }
                segmentPoint[i] = -1;
                i++;
            }
            segmentPoint[valley] = 1;
        }
    }
/*
    /// save segment
    int count = 0;
    i = 0;
    while(i++ < numSeg) {
        if(segmentPoint[i] > 0 ){
            count++;
            segmentRect = (Rect*) realloc( segmentRect, count * sizeof(Rect));
            segmentRect[count]
*/
    segPos = 0;
    for(i = 0; i < numSeg; i++) {
        if(segmentPoint[i] > 0) {
            for(j = 0; j < h; j++) {
                wordImg.pixels[j][segPos] = 1;
            }
        }
        segPos += segWidth;
    }
    printImage(wordImg, "out_w.txt");
    return 0;
}

void skewCorrectImage(Image input) {
    int i, j;
    /// get lower projection
    for(i = 0; i < input.w; i++) {
        for(j = input.h - 1; j > 0; j++) {
        }
    }
    /// detect lower baseline

    /// correct
}

