
#include <math.h>
#include "pre_procs.h"

/** masks for thinning image */
int A1[9] = {0, 0, 2,
             0, 1, 1,
             2, 1, 2
            };
int A2[9] = {2, 0, 0,
             1, 1, 0,
             2, 1, 2
            };
int A3[9] = {2, 1, 2,
             1, 1, 0,
             2, 0, 0
            };
int A4[9] = {2, 1, 2,
             0, 1, 1,
             0, 0, 2
            };
int B1[9] = {0, 0, 0,
             2, 1, 2,
             1, 1, 2
            };
int B2[9] = {1, 2, 0,
             1, 1, 0,
             2, 2, 0
            };
int B3[9] = {2, 1, 1,
             2, 1, 2,
             0, 0, 0
            };
int B4[9] = {0, 2, 2,
             0, 1, 1,
             0, 2, 1
            };

// remove slant
Image RemoveSlant(Image inputImage, double slantAngle)
{
	int h = inputImage.h;
	int w = inputImage.w;
	Image resultImage;
	double factor;
	int offset;
	int new_x;
	POINT point;

	factor = tan(slantAngle);
	offset = h * factor;
	resultImage = CreateImage(h, w + h * factor);
	for (int x = 0; x < w; x++)
	{
		for (int y = 0; y < h; y++)
		{
			new_x = x - y * factor + offset;
			SetPoint(&point, new_x, y);
			if (inputImage.pixels[y][x] == 1)
			{
				SetPixelPoint(&resultImage, point);
			}
		}
	}

	return resultImage;
}

Image ExtractContour(Image inputImage)
{
	int h = inputImage.h;
	int w = inputImage.w;
	Image resultImage;
	POINT currentPoint;
	POINT neighborPoint;
	bool dummy;

	resultImage = CreateImage(h + 2, w + 2);
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			if (inputImage.pixels[y][x] != 0)
			{
				SetPoint(&currentPoint, x, y);
				for (int dir = 0; dir < NUM_DIR; dir++)
				{
					neighborPoint = GetNeighborPoint(currentPoint, dir);
					if (!IsPointInImage(neighborPoint, h, w))
					{
						SetPoint(&currentPoint, x + 1, y + 1);
						SetPixelPoint(&resultImage, currentPoint);
						break;
					}
					else if (GetPixelValue(inputImage, neighborPoint) == 0)
					{
						SetPoint(&currentPoint, x + 1, y + 1);
						SetPixelPoint(&resultImage, currentPoint);
						break;
					}
				}
			}
		}
	}

	return resultImage;
}

void SmoothContour(Image *pContourImage)
{
	int h = pContourImage->h;
	int w = pContourImage->w;
	POINT currentPoint;
	POINT p1;
	POINT p2;
	POINT p3;
	POINT p4;

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			if (pContourImage->pixels[y][x] == 0)
			{
				// remove pattern
				//
				//     0              0
				//     1              0
				// 1 1 0 1   -->  1 1 1 1
				//
				SetPoint(&currentPoint, x, y);
				if ((GetNeighborPixelValue(*pContourImage, currentPoint, LEFT) == 1) &&
						(GetNeighborPixelValue(*pContourImage, currentPoint, RIGHT) == 1))
				{
					SetPoint(&p1, x - 2, y);
					SetPoint(&p2, x + 2, y);
					SetPoint(&p3, x, y - 2);
					SetPoint(&p4, x, y + 2);
					if ((GetPixelValue(*pContourImage, p1) == 1) ||
							(GetPixelValue(*pContourImage, p2) == 1))
					{
						if ((GetNeighborPixelValue(*pContourImage, currentPoint, TOP) == 1) &&
								(GetPixelValue(*pContourImage, p3) < 1))
						{
							pContourImage->pixels[y - 1][x] = 0;
							pContourImage->pixels[y][x] = 1;
						}
						
						if ((GetNeighborPixelValue(*pContourImage, currentPoint, BOT) == 1) &&
								(GetPixelValue(*pContourImage, p4) < 1))
						{
							pContourImage->pixels[y + 1][x] = 0;
							pContourImage->pixels[y][x] = 1;
						}
					}
				}
				
				if ((GetNeighborPixelValue(*pContourImage, currentPoint, TOP) == 1) &&
						(GetNeighborPixelValue(*pContourImage, currentPoint, BOT) == 1))
				{
					SetPoint(&p1, x - 2, y);
					SetPoint(&p2, x + 2, y);
					SetPoint(&p3, x, y - 2);
					SetPoint(&p4, x, y + 2);
					if ((GetPixelValue(*pContourImage, p3) == 1) ||
							(GetPixelValue(*pContourImage, p4) == 1))
					{
						if ((GetNeighborPixelValue(*pContourImage, currentPoint, LEFT) == 1) &&
								(GetPixelValue(*pContourImage, p1) < 1))
						{
							pContourImage->pixels[y][x - 1] = 0;
							pContourImage->pixels[y][x] = 1;
						}
						
						if ((GetNeighborPixelValue(*pContourImage, currentPoint, RIGHT) == 1) &&
								(GetPixelValue(*pContourImage, p2) < 1))
						{
							pContourImage->pixels[y][x + 1] = 0;
							pContourImage->pixels[y][x] = 1;
						}
					}
				}
			}
		}
	}
}

void ThinOutContour(Image *pContourImage)
{
	int h = pContourImage->h;
	int w = pContourImage->w;
	POINT currentPoint;
	bool cont = true;

	while (cont)
	{
		cont = false;
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				if (pContourImage->pixels[y][x] == 1)
				{
					SetPoint(&currentPoint, x, y);
					// type 1
					// matching pattern
					// 0 0 x        0 0 x
					// 0 1 1   -->  0 0 1
					// x 1 x        x 1 x
					for (int dir = 0; dir < NUM_DIR; dir += 2)
					{
						if ((GetNeighborPixelValue(*pContourImage, currentPoint, dir) == 1) &&
								(GetNeighborPixelValue(*pContourImage, currentPoint, (dir + 2) % 8) == 1) &&
								(GetNeighborPixelValue(*pContourImage, currentPoint, (dir + 4) % 8) < 1) &&
								(GetNeighborPixelValue(*pContourImage, currentPoint, (dir + 5) % 8) < 1) &&
								(GetNeighborPixelValue(*pContourImage, currentPoint, (dir + 6) % 8) < 1))
						{
							pContourImage->pixels[y][x] = 0;
							cont = true;
						}
					}

					// type 2
					// matching pattern
					// 0 x x        0 x x
					// 0 1 1   -->  0 0 1
					// 0 x 1        0 x 1
					for (int dir = 0; dir < NUM_DIR; dir += 2)
					{
						if ((GetNeighborPixelValue(*pContourImage, currentPoint, dir) == 1) &&
								(GetNeighborPixelValue(*pContourImage, currentPoint, (dir + 1) % 8) == 1) &&
								(GetNeighborPixelValue(*pContourImage, currentPoint, (dir + 3) % 8) < 1) &&
								(GetNeighborPixelValue(*pContourImage, currentPoint, (dir + 4) % 8) < 1) &&
								(GetNeighborPixelValue(*pContourImage, currentPoint, (dir + 5) % 8) < 1))
						{
							pContourImage->pixels[y][x] = 0;
							cont = true;
						}
					}

					// type 3
					// matching pattern
					// 0 0 x        0 0 x
					// 0 1 1   -->  0 0 1
					// 0 0 x        0 0 x
					for (int dir = 0; dir < NUM_DIR; dir += 2)
					{
						if ((GetNeighborPixelValue(*pContourImage, currentPoint, dir) == 1) &&
								(GetNeighborPixelValue(*pContourImage, currentPoint, (dir + 2) % 8) < 1) &&
								(GetNeighborPixelValue(*pContourImage, currentPoint, (dir + 3) % 8) < 1) &&
								(GetNeighborPixelValue(*pContourImage, currentPoint, (dir + 4) % 8) < 1) &&
								(GetNeighborPixelValue(*pContourImage, currentPoint, (dir + 5) % 8) < 1) &&
								(GetNeighborPixelValue(*pContourImage, currentPoint, (dir + 6) % 8) < 1))
						{
							pContourImage->pixels[y][x] = 0;
							cont = true;
						}
					}
				}
			}
		}
	}
}

/**
 * Convert image to gray image
 */

Image getGrayLevel(uint32 **bmpPix, int h, int w){
	int i, j;
	Image grayImg = CreateImage(h, w);

	for(i = 0; i < h; i++){
		for(j = 0; j < w; j++){
			grayImg.pixels[i][j] = (bmpPix[i][j] >> 16 & 0xff)*0.3 +
									(bmpPix[i][j] >> 8 & 0xff)*0.59 +
									(bmpPix[i][j] & 0xff)*0.11;
		}
	}
	return grayImg;
}


///--------------------BINARIZE IMAGE------------------------


/**
 * Binarize using local threshold method
 */

Image binarize(uint32 **bmpPix, int h, int w, int localSize){
	int i, j, k, l;
	int maxGrayLvl, minGrayLvl, grayLvl, threshold;
	Image grayImg = getGrayLevel(bmpPix, h, w);
	Image binImg = CreateImage(h, w);

	for(i = 0; i < h; i++){
		for(j = 0; j < w; j++){
//			printf("%d %d, ", i, j);
			maxGrayLvl = minGrayLvl = grayImg.pixels[i][j];
			/// check local pixel square
			for(k = 0; k < localSize; k++){
				for(l = 0; l < localSize; l++){
///					if(inImage(j + l - localSize/2, i + k - localSize/2, h, w)){
						grayLvl = grayImg.pixels[i + k - localSize/2][j + l - localSize/2];
						maxGrayLvl = grayLvl > maxGrayLvl ? grayLvl: maxGrayLvl;
						minGrayLvl = grayLvl < minGrayLvl ? grayLvl: minGrayLvl;

///					}
				}
			}
			threshold = (maxGrayLvl + minGrayLvl) / 2;
//			printf("th = %d ", threshold);
			if(grayImg.pixels[i][j] < threshold) binImg.pixels[i][j] = 1;
			else binImg.pixels[i][j] = 0;
		}
	}
	return binImg;

}

/**
 * Binarize using OTSU method
 */

void binarizeOtsu(Image grayImg, Image binImg, int start_x, int end_x, int start_y, int end_y){
	int i, j;
	int numPix = (end_x - start_x) * (end_y - start_y);
	double prop[MAX_GRAY], propSum = 0; // Pi
	double mean[MAX_GRAY], deltaMean;
	double maxSigma, btSigma;
	int threshold = 0, invert = 0;

	for(i = 0; i < MAX_GRAY; i++){
		prop[i] = 0;
		mean[i] = 0;
	}

	// calc Pi
	for(i = start_y; i < end_y; i++){
		for(j = start_x; j < end_x; j++){
			prop[(grayImg.pixels[i][j])] ++;
		}
	}

	prop[0] /= numPix;
	for(i = 1; i < MAX_GRAY; i++){
		prop[i] /= numPix;
		// calc mean
		mean[i] = mean[i-1] + i* prop[i];

	}

	maxSigma = 0;
	for(i = 0; i < MAX_GRAY; i++){
		propSum += prop[i];

		// cal sigma
		deltaMean = mean[i] / propSum - (mean[MAX_GRAY-1] - mean[i]) / (1- propSum);
		btSigma = propSum* (1 - propSum)* deltaMean * deltaMean;
		if(btSigma > maxSigma){
//			printf("props = %.2lf",propSum);
			maxSigma = btSigma;
			threshold = i;
			if(propSum > 0.5)
				invert = 1;
		}
	}
//	printf("threshold = %d\n", threshold);

	// binarize using threshold
	if(!invert){
		for(i = start_y; i < end_y; i++){
			for(j = start_x; j < end_x; j++){
				if(grayImg.pixels[i][j] <= threshold) binImg.pixels[i][j] = 1;
				else binImg.pixels[i][j] = 0;
			}
		}
	} else {
		for(i = start_y; i < end_y; i++){
			for(j = start_x; j < end_x; j++){
				if(grayImg.pixels[i][j] > threshold) binImg.pixels[i][j] = 1;
				else binImg.pixels[i][j] = 0;
			}
		}
	}
}

/**
 * Adaptive OTSU method
 */

Image binarizeAdaptOtsu(uint32 ** bmpPix, int h, int w){
//	int i, j;
	Image grayImg = getGrayLevel(bmpPix, h, w);
	Image binImg = CreateImage(h, w);

//	for(i = 0; i < h/WIN_SIZE; i++){
//		for(j = 0; j < w/WIN_SIZE; j++){
//			binarizeOtsu(grayImg, binImg, j*WIN_SIZE, (j+1)*WIN_SIZE,
//											i*WIN_SIZE, (i+1)*WIN_SIZE);
//		}
//		binarizeOtsu(grayImg, binImg, j*WIN_SIZE, w, i* WIN_SIZE, (i+1)*WIN_SIZE);
//	}
//
//	for(j = 0; j < w/WIN_SIZE; j++){
//		binarizeOtsu(grayImg, binImg, j*WIN_SIZE, (j+1)*WIN_SIZE,
//										i*WIN_SIZE, h);
//	}
//	binarizeOtsu(grayImg, binImg, j*WIN_SIZE, w, i* WIN_SIZE, h);
	binarizeOtsu(grayImg, binImg, 0, w, 0, h);

	deleteImage(grayImg);
	return binImg;
}

/**
 * Binarize gray image Otsu
 */
Image binarizeGrayImageOtsu(Image grayImg){
        int i, j;
        int h = grayImg.h;
        int w = grayImg.w;
        Image binImg = CreateImage(h, w);

        for(i = 0; i < h/WIN_SIZE; i++){
                for(j = 0; j < w/WIN_SIZE; j++){
                        binarizeOtsu(grayImg, binImg, j*WIN_SIZE, (j+1)*WIN_SIZE,
                                                                                        i*WIN_SIZE, (i+1)*WIN_SIZE);
                }
                binarizeOtsu(grayImg, binImg, j*WIN_SIZE, w, i* WIN_SIZE, (i+1)*WIN_SIZE);
        }

        for(j = 0; j < w/WIN_SIZE; j++){
                binarizeOtsu(grayImg, binImg, j*WIN_SIZE, (j+1)*WIN_SIZE,
                                                                                i*WIN_SIZE, h);
        }
        binarizeOtsu(grayImg, binImg, j*WIN_SIZE, w, i* WIN_SIZE, h);

        deleteImage(grayImg);
        return binImg;
}


/**
 * Kmeans method
 */

void KmeansClutter(Image grayImg, int *m1, int *m2) {
    int i, j,
    new_m1 = 0, new_m2 = 0,
    dif_m1, dif_m2,
    num_m1 = 0, num_m2 = 0;
    int h = grayImg.h;
    int w = grayImg.w;

    /// assignment step
    for(i = 0; i < h; i++) {
        for(j = 0; j < w; j++) {
            dif_m1 = grayImg.pixels[i][j] > *m1 ? grayImg.pixels[i][j] - *m1 : *m1 - grayImg.pixels[i][j];
            dif_m2 = grayImg.pixels[i][j] > *m2 ? grayImg.pixels[i][j] - *m2 : *m2 - grayImg.pixels[i][j];

            if(dif_m1 < dif_m2){ /// belong to 1 set
                new_m1 += grayImg.pixels[i][j];
                num_m1 ++;
            }
            else { /// belong to set 2
                new_m2 += grayImg.pixels[i][j];
                num_m2 ++;
            }
        }
    }

    /// update step
	if (num_m1 != 0)
	{
		*m1 = new_m1 / num_m1;
	}
	if (num_m2 != 0)
	{
		*m2 = new_m2 / num_m2;
	}

}


Image binarizeKmeans(uint32 ** bmpPix, int h, int w) {
    Image grayImg = getGrayLevel(bmpPix, h, w);
    int i, j;
    int m1 = 0, m2 = 255,
    last_m1, last_m2,
    dif_m1, dif_m2;

    do{
        last_m1 = m1;
        last_m2 = m2;

        KmeansClutter(grayImg, &m1, &m2);
    }while (last_m1 != m1);

    /// binarize image
    for(i = 0; i < h; i++) {
        for(j = 0; j < w; j++) {
            dif_m1 = grayImg.pixels[i][j] > m1 ? grayImg.pixels[i][j] - m1 : m1 - grayImg.pixels[i][j];
            dif_m2 = grayImg.pixels[i][j] > m2 ? grayImg.pixels[i][j] - m2 : m2 - grayImg.pixels[i][j];

            if(dif_m1 < dif_m2){ /// belong to 1 set
                grayImg.pixels[i][j] = 1;
            }
            else { /// belong to set 2
                grayImg.pixels[i][j] = 0;
            }
        }
    }
    return grayImg;

}

Image binarizeGrayImageKmeans(Image grayImg) {
    int h = grayImg.h;
    int w = grayImg.w;

    int i, j;
    int m1 = 0, m2 = 255,
    last_m1, last_m2,
    dif_m1, dif_m2;

    do{
        last_m1 = m1;
        last_m2 = m2;

        KmeansClutter(grayImg, &m1, &m2);
    }while (last_m1 != m1);

    /// binarize image
    for(i = 0; i < h; i++) {
        for(j = 0; j < w; j++) {
            dif_m1 = grayImg.pixels[i][j] > m1 ? grayImg.pixels[i][j] - m1 : m1 - grayImg.pixels[i][j];
            dif_m2 = grayImg.pixels[i][j] > m2 ? grayImg.pixels[i][j] - m2 : m2 - grayImg.pixels[i][j];

            if(dif_m1 < dif_m2){ /// belong to 1 set
                grayImg.pixels[i][j] = 1;
            }
            else { /// belong to set 2
                grayImg.pixels[i][j] = 0;
            }
        }
    }
    return grayImg;

}

///--------------SLANT REMOVE---------------


/**
 * Rate angle to detect slant
 *

int ratingAngle(Image input, int start_l, int end_l, int start_w, int end_w, double angle)
{
	int i, j, new_j;
//	int wSize = end_w - start_w;
	int height = end_l - start_l;
	int colDen;
	int rate = 0;

	for (j = start_w; j < end_w; j ++) {
		colDen = 0;
		for (i = start_l; i <= end_l; i ++){
			new_j = j - (int)((i - start_l)* tan(angle));

			if(new_j >= start_w && new_j < end_w &&(input.pixels[i][new_j])){
				colDen ++;
			}
		}
//		printf("colD = %d", colDen);
		if(colDen > height) rate ++;
	}

	return rate;
}*/

/**
 * Slant removal
 */

Image slantRemoval(Image input, int slantDeg)
{
    int i, j, box_i, box_j;
	double angle;
	angle = PI * slantDeg/ 180;
    int h = input.h;
    int w = input.w;
	/// slant remove
	int addWidth = (int)(h * tan(angle));
	int shift;
	Image box;
    if(slantDeg > 0) {
        box = CreateImage(h, w + addWidth);

        for(j = 0; j < h; j ++){
            box_j = j;
            shift = (int)((h - 1 - box_j)* tan(angle));
            for(i = 0; i < w; i++){
                if(input.pixels[j][i]){
                    box_i = i - shift + addWidth;
                    if(box_i >=0){
                        box.pixels[box_j][box_i] = 1;
                        //printf("%d %d\n", box_i, box_j);
                    }
                }
            }
        }
    }
    else {
        box = CreateImage(h, w - addWidth);
        for(j = 0; j < h; j ++){
            box_j = j;
            shift = (int)((box_j)* tan(angle));
            for(i = 0; i < w; i++){
                if(input.pixels[j][i]){

                    box_i = i + shift - addWidth;

                    if(box_i >=0){
                        box.pixels[box_j][box_i] = 1;
                        //printf("%d %d\n", box_i, box_j);
                    }
                }
            }
        }
    }
    return box;
}

///--------------SMOOTH IMAGE---------------

/**
 * Check mask for smooth image
 */

void checkMaskRecursive(Image input, int i, int j)
{
    int k, type, dir, setVal;

    for(type = 0; type < 4; type++){
        dir = type * 2;
        setVal = getNeighbor(input, dir, i, j);
        for(k = 1; k < 5; k++){
            if(getNeighborForMatch(input, (dir + k)%8, i, j) != setVal){
                setVal = -1;
                break;
            }
        }

        if((setVal != -1) && setVal != input.pixels[j][i]){ // mask matched
            input.pixels[j][i] = setVal;
            // check neighbor
            switch(type){
            case 0:
                checkMaskRecursive(input, i, j+1);
                checkMaskRecursive(input, i-1, j+1);
                checkMaskRecursive(input, i+1, j+1);
                break;
            case 1:
                checkMaskRecursive(input, i+1, j);
                checkMaskRecursive(input, i+1, j-1);
                checkMaskRecursive(input, i+1, j+1);
                break;
            case 2:
                checkMaskRecursive(input, i, j-1);
                checkMaskRecursive(input, i-1, j-1);
                checkMaskRecursive(input, i+1, j-1);
                break;
            case 3:
                checkMaskRecursive(input, i-1, j);
                checkMaskRecursive(input, i-1, j-1);
                checkMaskRecursive(input, i-1, j+1);
                break;
            }
        }
    }
}

/**
 * smooth image
 */

void smoothImage(Image input)
{
    int i, j;

    for(j = 0; j < input.h; j++){
        for(i = 0; i < input.w; i++){
            // check mask
            checkMaskRecursive(input, i, j);
        }
    }
}


///--------------SCALE IMAGE---------------

void ScaleLine(unsigned char *Target, unsigned char *Source, int SrcWidth, int TgtWidth)
{
	int NumPixels = TgtWidth;
	int IntPart = SrcWidth / TgtWidth;
	int FractPart = SrcWidth % TgtWidth;
	int E = 0;
	while (NumPixels-- > 0) {
		*Target++ = *Source;
		Source += IntPart;

		E += FractPart;
		if (E >= TgtWidth) {
			E -= TgtWidth;
			Source++;
		}
	}
}

void ScaleRect(unsigned char **Target, unsigned char **Source, int SrcWidth, int SrcHeight,
               int TgtWidth, int TgtHeight)
{
	int NumPixels = TgtHeight;
	int IntPart = (SrcHeight / TgtHeight);
	int FractPart = SrcHeight % TgtHeight;
	int E = 0;
	//char **PrevSource = NULL;
	int i, j = 0;

	for (i = 0; i < NumPixels; i++) {
//		if (Source == PrevSource) {
//			memcpy(Target, Target-TgtWidth, TgtWidth*sizeof(*Target));
//		} else {

		ScaleLine(Target[i], Source[j], SrcWidth, TgtWidth);

//		PrevSource = Source;
//		} /* if */
//		Target += TgtWidth;
		j += IntPart;
		E += FractPart;
		if (E >= TgtHeight) {
			E -= TgtHeight;
			j ++;
		}
	}
}

Image scaleImage(Image input, int h, int w)
{
	Image output;
	output.h = h;
	output.w = w;
	int i;

	output.pixels = (unsigned char**) malloc(h*sizeof(unsigned char*));
	for (i = 0; i < h; i++)
		output.pixels[i] = (unsigned char*) malloc(w*sizeof(unsigned char));
	ScaleRect(output.pixels, input.pixels, input.w, input.h, output.w, output.h);
	return output;

}

///--------------THINNING IMAGE---------------


/**
 * check removable for thinning
 */
int checkRemovableOnMatch(Image input, int x, int y, int type)
{
	int result;
	int* comparePattern;
	switch (type) {
	case 0:
		comparePattern = A1;
		break;
	case 1:
		comparePattern = A2;
		break;
	case 2:
		comparePattern = A3;
		break;
	case 3:
		comparePattern = A4;
		break;
	case 4:
		comparePattern = B1;
		break;
	case 5:
		comparePattern = B2;
		break;
	case 6:
		comparePattern = B3;
		break;
	case 7:
		comparePattern = B4;
		break;

	}
	result = 1;
	if ((comparePattern[5] != 2)&&(getNeighborForMatch(input, 0, x, y)!= comparePattern[5]))
		result = 0;

	if ((comparePattern[2] != 2)&&(getNeighborForMatch(input, 1, x, y)!= comparePattern[2]))
		result = 0;

	if ((comparePattern[1] != 2)&&(getNeighborForMatch(input, 2, x, y)!= comparePattern[1]))
		result = 0;

	if ((comparePattern[0] != 2)&&(getNeighborForMatch(input, 3, x, y)!= comparePattern[0]))
		result = 0;

	if ((comparePattern[3] != 2)&&(getNeighborForMatch(input, 4, x, y)!= comparePattern[3]))
		result = 0;

	if ((comparePattern[6] != 2)&&(getNeighborForMatch(input, 5, x, y)!= comparePattern[6]))
		result = 0;

	if ((comparePattern[7] != 2)&&(getNeighborForMatch(input, 6, x, y)!= comparePattern[7]))
		result = 0;

	if ((comparePattern[8] != 2)&&(getNeighborForMatch(input, 7, x, y)!= comparePattern[8]))
		result = 0;
	return result;
}

/**
 * Thining image

void thinImageOnMatch(Image input)
{
	int i, j, k, cont = 1;

    Image workImg = createImage(input.h, input.w);
	copyImage(workImg, input);

	while (cont) {
		cont = 0;
		for (k = 0; k < 8; k++) {
			/// check removable for type k
			for (j = 0; j < input.h; j++) {
				for (i = 0; i < input.w; i++) {
					if (input.pixels[j][i] && checkRemovableOnMatch(input, i, j, k)) {
						workImg.pixels[j][i] = 0;
						cont = 1;
					}
				}
			}
			copyImage(input, workImg);
		}
	}
	deleteImage(workImg);
}*/

void thinImageOnMatch(Image input) {
    int i, j, k, dir, cont = 1;

    while (cont) {
		cont = 0;
        /// check removable for direction
        for (k = 0; k < 4; k++) {
            dir = 2*k;

            for (j = 0; j < input.h; j++) {
                for (i = 0; i < input.w; i++) {
                    if (input.pixels[j][i]) {

                        /// if match one of these patterns
                        if(getNeighbor(input, dir, i, j) == 1 &&

                        getNeighbor(input,(dir +2)%8 , i, j) == 1 &&
                        getNeighborForMatch(input,(dir +4)%8 , i, j) == 0 &&
                        getNeighborForMatch(input,(dir +5)%8 , i, j) == 0 &&
                        getNeighborForMatch(input,(dir +6)%8 , i, j) == 0) /// type 1
                        {
                            input.pixels[j][i] = 0;
                            cont = 1;
                        }
                    }
                }
            }
        }

        for (k = 0; k < 4; k++) {
            dir = 2*k;

            for (j = 0; j < input.h; j++) {
                for (i = 0; i < input.w; i++) {
                    if (input.pixels[j][i]) {
                        if(getNeighbor(input, dir, i, j) == 1 &&
                        getNeighbor(input,(dir + 7)%8 , i, j) == 1 &&
                        getNeighborForMatch(input,(dir + 3)%8 , i, j) == 0 &&
                        getNeighborForMatch(input,(dir + 4)%8 , i, j) == 0 &&
                        getNeighborForMatch(input,(dir + 5)%8 , i, j) == 0) /// type 2
                        {
                            input.pixels[j][i] = 0;
                            cont = 1;
                        }
                    }
                }
            }
        }
    }
}

/** thining by dialation and erosion

void dilation (Image input, int mask[5][5], Image filter) {
    int x, y, i, j, smax;
    for(y = 2; y < input.h-2 ; y ++) {
        for(x = 2; x < input.w -2; x ++){
            smax = 0;
            for(j = -2; j <= 2; j++) {
                for(i = -2; i <= 2; i++) {
                    if(mask[j+2][i+2] == 1) {
                        //if(inImage(y+j, x+i, input.h, input.w)){
                            if(input.pixels[y+j][x+i] > smax) {
                                smax = input.pixels[y+j][x+i];
                            }
                        //}
                    }
                }
            }
            filter.pixels[y][x] = smax;
        }
    }
}

void erosion (Image input, int mask[5][5], Image filter) {
    int x, y, i, j, smin;
    for(y = 2; y < input.h-2; y ++) {
        for(x = 2; x < input.w-2; x ++){
            smin = 1;
            for(j = -2; j <= 2; j++) {
                for(i = -2; i <= 2; i++) {
                    if(mask[j+2][i+2] == 1) {
                        //if(inImage(y+j, x+i, input.h, input.w)){
                            if(input.pixels[y+j][x+i] < smin) {
                                smin = input.pixels[y+j][x+i];
                            }
                        //}
                    }
                }
            }
            filter.pixels[y][x] = smin;
        }
    }
}

void skeletonImage(Image input, Image skeleton){
    int x, y, i, j, mask[5][5], cont;
    Image filter = createImage(input.h, input.w);
    Image temp = createImage(input.h, input.w);

    /// create mask
    for(j = 0; j < 5; j++)
        for(i = 0; i < 5; i++)
            mask[j][i] = 1;
    mask[0][0] = 0;
    mask[0][4] = 0;
    mask[4][0] = 0;
    mask[4][4] = 0;

    for(y = 0; y < input.h; y ++)
        for(x = 0; x < input.w; x++)
            skeleton.pixels[y][x] = 0;

    while(cont) {
        cont = 0;
        erosion(input, mask, temp);
        dilation(temp, mask, filter);
        for(y = 0; y < input.h; y ++)
            for(x = 0; x < input.w; x++) {
                if(input.pixels[y][x] - filter.pixels[y][x] == 1 ){
                    skeleton.pixels[y][x] = 1;
                }
            }
        erosion(input, mask, input);
    }
} */

///--------------CONTOUR FINDING---------------

Image contourFinding(Image input) {
    int h = input.h;
    int w = input.w;
    int i, j, dir;
    Image contourOut = CreateImage(h+2, w+2);

    for (j = 0; j < h; j++) {
        for(i = 0; i < w; i++) {
            if(input.pixels[j][i]){
                /// check 4 directions
                for(dir = 0; dir < 8; dir= dir+2) {
                    if(getNeighborForMatch(input, dir, i, j) == 0){
                        //contourOut.pixels[j][i] = 1;
                        setNeighbor(contourOut, dir, i+1, j+1, 1);
                        //break;
                    }
                }
            }
        }
    }
    return contourOut;
}

///--------------LINE SKEW CORRECTION---------------

int * getLineDensity(Image input, double angle){
    int i, j, new_j;
    int h = input.h;
    int w = input.w;
    int* lnDen = (int*) calloc(h, sizeof(int));

    for (j = 0; j < h; j ++) {
		for (i = 0; i < w; i ++){
			new_j = j - (int)(i* tan(angle));

			if(new_j >= 0 && new_j < h && input.pixels[new_j][i]){
				lnDen[j] ++;
			}
//			else break;
		}
//		printf("%d ", lnDen[j]);
	}
	return lnDen;
}

Image lineSkewCorrection(Image input) {
	Image out;
    int i, j, max, min, threshold;
    int nHigh, nLow, vHigh, vLow;
    double angle;
    int *lnDen;
    int dif, maxDif = 0;
    double deg, skewDeg;
    int h = input.h;
    int w = input.w;

    ///
    for(deg = -5; deg <= 5; deg = deg + 0.2){
		angle = PI * deg / 180;
		lnDen = getLineDensity(input, angle);
		/// find angle with highest different

        max = min = lnDen[0];
		/// find threshold
		for(i = 1; i < h; i++) {
	        max = max > lnDen[i] ? max : lnDen[i];
            min = min < lnDen[i] ? min : lnDen[i];
		}
        threshold = (max + min*3)/4;
        //printf("max = %d, min = %d\n", max, min);
        /// calc different
        vHigh = vLow = 0;
        nHigh = nLow = 0;
        for(i = 0; i < h; i++) {
            if(lnDen[i] > threshold) {
                vHigh += lnDen[i];
                nHigh ++;
            }
            else {
                vLow += lnDen[i];
                nLow ++;
            }
        }
        //printf("%d %d\n", vHigh/nHigh, vLow/nLow);
        dif = vHigh /nHigh - vLow/nLow;
        //printf("%d %.2lf\n", dif, deg);
        if (dif > maxDif) {
            skewDeg = deg;
            maxDif = dif;
        }
	}
    //printf("deg = %.2lf\n",skewDeg);
    angle = PI * skewDeg/180;
	/// skew correction
	int shift;
	//Image out = createImage(h, w);
	out = CreateImage(h, w);
	for(i = 0; i < w; i++) {
        shift = (int)(i * tan(angle));
	    for(j = 0; j < h; j++) {
            if((input.pixels[j][i])&&(j + shift >= 0)&&(j+shift < h)){
                out.pixels[j+shift][i] = 1;
            }
	    }
	}

    return out;
}

void smoothContour(Image input)
{
    int i, j;

    for (i = 1; i < input.h-1; i++)
    {
        for (j = 1; j < input.w-1; j++)
        {
            if (input.pixels[i][j] == 0)
            {
                if (input.pixels[i][j-1]&& input.pixels[i][j+1])
                {
                    if ((inImage(i, j-2, input.h, input.w) && input.pixels[i][j-2]) || (inImage(i, j+2, input.h, input.w) && input.pixels[i][j+2]))
                    {
                        if (input.pixels[i-1][j] && input.pixels[i+1][j])
                            continue;
                        if ((!inImage(i-2, j, input.h, input.w) || (input.pixels[i-2][j] == 0)) && input.pixels[i-1][j])
                        {
                            input.pixels[i][j] = 1;
                            input.pixels[i-1][j] = 0;
                        }
                        if ((!inImage(i+2, j, input.h, input.w) || (input.pixels[i+2][j] == 0)) && input.pixels[i+1][j])
                        {
                            input.pixels[i][j] = 1;
                            input.pixels[i+1][j] = 0;
                        }
                    }
                }
                else if (input.pixels[i-1][j] && input.pixels[i+1][j])
                {
                    if ((inImage(i-2, j, input.h, input.w) && input.pixels[i-2][j]) || (inImage(i+2, j, input.h, input.w) && input.pixels[i+2][j]))
                    {
                        if (input.pixels[i][j-1] && input.pixels[i][j+1])
                            continue;
                        if ((!inImage(i, j-2, input.h, input.w) || (input.pixels[i][j-2] == 0)) && input.pixels[i][j-1])
                        {
                            input.pixels[i][j] = 1;
                            input.pixels[i][j-1] = 0;
                        }
                        if ((!inImage(i, j+2, input.h, input.w) || (input.pixels[i][j+2] == 0)) && input.pixels[i][j+1])
                        {
                            input.pixels[i][j] = 1;
                            input.pixels[i][j+1] = 0;
                        }
                    }
                }
            }
        }
    }

//    for (j = 1; j < input.w-1; j++)
//    {
//        for (i = 1; i < input.h-1; i++)
//        {
//            if (input.pixels[i][j] == 0)
//            {
//                if (input.pixels[i-1][j] && input.pixels[i+1][j])
//                {
//                    if (input.pixels[i-2][j] || input.pixels[i+2][j])
//                    {
//                        if (input.pixels[i][j-1] && input.pixels[i][j+1])
//                            continue;
//                        if (input.pixels[i][j-1])
//                        {
//                            input.pixels[i][j] = 1;
//                            input.pixels[i][j-1] = 0;
//                        }
//                        if (input.pixels[i][j+1])
//                        {
//                            input.pixels[i][j] = 1;
//                            input.pixels[i][j+1] = 0;
//                        }
//                    }
//                }
//            }
//        }
//    }
}
