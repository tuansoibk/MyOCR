#include "common.h"
#include "feature.h"
#include "segment.h"
#include "char_recog.h"

int* walkedPos;//[7000];
int numWalkedPos;

// prototypes
int checkWalked(int x, int y, int BOX_W);
int checkDirection(Image input, int dir, int x, int y);
void setChainCode(unsigned char **pixels, int dir, int* x, int* y);
int getLeftBot(Image input, int* x, int* y);
void getFeatureRecursive(Image input, int x, int y);

PPOINT GetLeftBotPoint(Image inputImage)
{
	PPOINT pResult = NULL;

	for (int y = inputImage.h - 1; y >= 0; y--)
	{
		for (int x = 0; x < inputImage.w; x++)
		{
			if (inputImage.pixels[y][x] == 1)
			{
				pResult = (PPOINT)malloc(sizeof(POINT));
				SetPoint(pResult, x, y);
				break;
			}
		}

		if (pResult != NULL)
		{
			break;
		}
	}

	return pResult;
}

// set chain code from a specific point
void SetChainCode(Image inputImage, POINT point)
{
	POINT currentPoint;
	PPOINT pNeighborPoint;
	int neighborDir;

	SetPoint(&currentPoint, point.x, point.y);
	while (true)
	{
		pNeighborPoint = GetNeighborPixelPointOnMatch(inputImage, currentPoint, 1, &neighborDir);
		if (pNeighborPoint != NULL)
		{
			switch (neighborDir)
			{
				case RIGHT:
				case LEFT:
					SetPixelValue(inputImage, *pNeighborPoint, HORIZONTAL);
					break;
				case TOP:
				case BOT:
					SetPixelValue(inputImage, *pNeighborPoint, VERTICAL);
					break;
				case RIGHT_TOP:
				case LEFT_BOT:
					SetPixelValue(inputImage, *pNeighborPoint, BDIAGONAL);
					break;
				case LEFT_TOP:
				case RIGHT_BOT:
					SetPixelValue(inputImage, *pNeighborPoint, FDIAGONAL);
					break;
				default:
					break;
			}
			
			SetPoint(&currentPoint, pNeighborPoint->x, pNeighborPoint->y);
		}
		else
		{
			break;
		}
	}
}

double* GetFeature(Image inputImage)
{
	int h = inputImage.h;
	int w = inputImage.w;
	int x;
	int y;
	PPOINT leftBotPoint;
	int i, j, k, r, s, t;
	double **LT, **DT;
	double *featureVector;

	while (true)
	{
		leftBotPoint = GetLeftBotPoint(inputImage);
		if (leftBotPoint != NULL)
		{
			SetChainCode(inputImage, *leftBotPoint);
		}
		else
		{
			break;
		}
	}
	
    t = 0;
	featureVector = (double*) malloc(NFEATURE*sizeof(double));
    /// get row's MDF
    LT = (double**) malloc(h*sizeof(double*));
	DT = (double**) malloc(h*sizeof(double*));
	for (i = 0; i < h; i++)
	{
	    LT[i] = (double*) malloc(MAXMDF*sizeof(double));
	    DT[i] = (double*) malloc(MAXMDF*sizeof(double));
    }
    /// left to right phase
    for (i = 0; i < h; i++)
    {
        k = 0;
        for (j = 0; j < w; j++)
        {
            if (k >= MAXMDF)
                break;
            if (inputImage.pixels[i][j] && ((j == 0) || (inputImage.pixels[i][j-1] == 0)))
            {
                LT[i][k] = (double) (w - j - 1) / w;
                DT[i][k] = (double) inputImage.pixels[i][j] / 10;
                k++;
            }
        }
        for (; k < MAXMDF; k++)
        {
            LT[i][k] = 0;
            DT[i][k] = 0;
        }
    }

    r = h % FEATUREH;
    s = 0;
    while (s < h)
    {
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] = 0;
        }
        k = h / FEATUREH;
        k += (r > 0) ? 1 : 0;
        for (i = 0; i < k; i++)
        {
            for (j = 0; j < MAXMDF; j++)
            {
                featureVector[t+j] += LT[s+i][j];
                featureVector[t+j+MAXMDF] += DT[s+i][j];
            }
        }
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] /= k;
        }
        t += 2*MAXMDF;
        s += k;
        r--;
        //printf("t = %d s = %d r = %d k = %d\n", t, s, r, k);
    }

    /// right to left phase
    for (i = 0; i < h; i++)
    {
        k = 0;
        for (j = w-1; j >= 0; j--)
        {
            if (k >= MAXMDF)
                break;
            if (inputImage.pixels[i][j] && ((j == w-1) || (inputImage.pixels[i][j+1] == 0)))
            {
                LT[i][k] = (double) j / w;
                DT[i][k] = (double) inputImage.pixels[i][j] / 10;
                k++;
            }
        }
        for (; k < MAXMDF; k++)
        {
            LT[i][k] = 0;
            DT[i][k] = 0;
        }
    }

    r = h % FEATUREH;
    s = 0;
    while (s < h)
    {
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] = 0;
        }
        k = h / FEATUREH;
        k += (r > 0) ? 1 : 0;
        for (i = 0; i < k; i++)
        {
            for (j = 0; j < MAXMDF; j++)
            {
                featureVector[t+j] += LT[s+i][j];
                featureVector[t+j+MAXMDF] += DT[s+i][j];
            }
        }
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] /= k;
        }
        t += 2*MAXMDF;
        s += k;
        r--;
    }
    for (i = 0; i < h; i++)
    {
        free(LT[i]);
        free(DT[i]);
    }
    free(LT);
    free(DT);

    /// get column's MDF
    LT = (double**) malloc(w*sizeof(double*));
	DT = (double**) malloc(w*sizeof(double*));
	for (i = 0; i < w; i++)
	{
	    LT[i] = (double*) malloc(MAXMDF*sizeof(double));
	    DT[i] = (double*) malloc(MAXMDF*sizeof(double));
    }
    /// up to down phase
    for (j = 0; j < w; j++)
    {
        k = 0;
        for (i = 0; i < h; i++)
        {
            if (k >= MAXMDF)
                break;
            if (inputImage.pixels[i][j] && ((i == 0) || (inputImage.pixels[i-1][j] == 0)))
            {
                LT[j][k] = (double) (h - i - 1) / h;
                DT[j][k] = (double) inputImage.pixels[i][j] / 10;
                k++;
            }
        }
        for (; k < MAXMDF; k++)
        {
            LT[j][k] = 0;
            DT[j][k] = 0;
        }
    }

    r = w % FEATUREH;
    s = 0;
    while (s < w)
    {
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] = 0;
        }
        k = w / FEATUREH;
        k += (r > 0) ? 1 : 0;
        for (i = 0; i < k; i++)
        {
            for (j = 0; j < MAXMDF; j++)
            {
                featureVector[t+j] += LT[s+i][j];
                featureVector[t+j+MAXMDF] += DT[s+i][j];
            }
        }
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] /= k;
        }
        t += 2*MAXMDF;
        s += k;
        r--;
    }

    /// down to up phase
    for (j = 0; j < w; j++)
    {
        k = 0;
        for (i = h-1; i >= 0; i--)
        {
            if (k >= MAXMDF)
                break;
            if (inputImage.pixels[i][j] && ((i == h-1) || (inputImage.pixels[i+1][j] == 0)))
            {
                LT[j][k] = (double) i / h;
                DT[j][k] = (double) inputImage.pixels[i][j] / 10;
                k++;
            }
        }
        for (; k < MAXMDF; k++)
        {
            LT[j][k] = 0;
            DT[j][k] = 0;
        }
    }

    r = w % FEATUREH;
    s = 0;
    while (s < w)
    {
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] = 0;
        }
        k = w / FEATUREH;
        k += (r > 0) ? 1 : 0;
        for (i = 0; i < k; i++)
        {
            for (j = 0; j < MAXMDF; j++)
            {
                featureVector[t+j] += LT[s+i][j];
                featureVector[t+j+MAXMDF] += DT[s+i][j];
            }
        }
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] /= k;
        }
        t += 2*MAXMDF;
        s += k;
        r--;
    }
    for (i = 0; i < w; i++)
    {
        free(LT[i]);
        free(DT[i]);
    }
    free(LT);
    free(DT);

	return featureVector;
}

int getLeftBot(Image input, int* x, int* y)
{
	int i, j;
	int BOX_H = input.h;
	int BOX_W = input.w;

	for (j = BOX_H - 1; j >= 0; j--) {
		for (i = 0; i < BOX_W; i ++) {
			if ((input.pixels[j][i] == 1) && (checkWalked(i, j, BOX_W) == 0)) {
				*x = i;
				*y = j;
				return 1;
			}
		}
	}
	return 0;
}

void shift_vector(int *vector, int len)
{
	int i;
	for (i = len -1; i > 0; i-- ) {
		vector[i] = vector[i-1];
	}
}

int checkWalked(int x, int y, int BOX_W)
{
	int i;
	int pos = x + y * BOX_W;
	for (i = 0; i < numWalkedPos; i++) {
		if (walkedPos[i] == pos) return 1;
	}
	return 0;
}

int checkDirection(Image input, int dir, int x, int y)
{
	//printf("dir = %d\n", dir);
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
	//printf("%d, %d, %d\n", in_box(x, y), (pixels[y][x] == 1), checkWalked(pixels, x, y));
	if (inImage(y, x, input.h, input.w)) {
		if ((input.pixels[y][x] == 1) && !checkWalked(x, y, input.w))
			return 1;
	}
	return 0;
}

void setChainCode(unsigned char **pixels, int dir, int* x, int* y)
{
	switch (dir) {
	case RIGHT:
		*x += 1;
		pixels[*y][*x] = HORIZONTAL;
		break;
	case RIGHT_TOP:
		*x += 1;
		*y -= 1;
		pixels[*y][*x] = BDIAGONAL;
		break;
	case TOP:
		*y -= 1;
		pixels[*y][*x] = VERTICAL;
		break;
	case LEFT_TOP:
		*x -= 1;
		*y -= 1;
		pixels[*y][*x] = FDIAGONAL;
		break;
	case LEFT:
		*x -= 1;
		pixels[*y][*x] = HORIZONTAL;
		break;
	case LEFT_BOT:
		*x -= 1;
		*y += 1;
		pixels[*y][*x] = BDIAGONAL;
		break;
	case BOT:
		*y += 1;
		pixels[*y][*x] = VERTICAL;
		break;
	case RIGHT_BOT:
		*x += 1;
		*y += 1;
		pixels[*y][*x] = FDIAGONAL;
		break;
	}
}

void getFeatureRecursive(Image input, int x, int y)
{
	int i, x0, y0;
	int BOX_W = input.w;
	x0 = x;
	y0 = y;
	for (i = 0; i < NUM_DIR; i++) {
		if (checkDirection(input, i, x0, y0)) {
			x = x0;
			y = y0;
			/// set value
			setChainCode(input.pixels, i, &x, &y);
			//printf("pix = %d, x = %d, y = %d\n", pixels[y][x], x, y);
			/// save x y to indicate modified
			walkedPos = (int*) realloc(walkedPos, (numWalkedPos+1) * sizeof(int));
			walkedPos[numWalkedPos] = x + y * BOX_W;
			numWalkedPos++;

			getFeatureRecursive(input, x, y);

		}
	}

	if(!checkWalked(x0, y0, BOX_W)) {
	    walkedPos = (int*) realloc(walkedPos, (numWalkedPos+1) * sizeof(int));
		walkedPos[numWalkedPos] = x + y * BOX_W;
		numWalkedPos++;
	}
}

/**
 * Get feature vectore from input image
 */
double* getFeature(Image input)
{
	int i, j, k, r, s, t;
	double **LT, **DT;
	double *featureVector = (double*) malloc(NFEATURE*sizeof(double));

	/// set chaincode
	while (getLeftBot(input, &i, &j)) {
		getFeatureRecursive(input, i, j);
	}

    t = 0;
    /// get row's MDF
    LT = (double**) malloc(input.h*sizeof(double*));
	DT = (double**) malloc(input.h*sizeof(double*));
	for (i = 0; i < input.h; i++)
	{
	    LT[i] = (double*) malloc(MAXMDF*sizeof(double));
	    DT[i] = (double*) malloc(MAXMDF*sizeof(double));
    }
    /// left to right phase
    for (i = 0; i < input.h; i++)
    {
        k = 0;
        for (j = 0; j < input.w; j++)
        {
            if (k >= MAXMDF)
                break;
            if (input.pixels[i][j] && ((j == 0) || (input.pixels[i][j-1] == 0)))
            {
                LT[i][k] = (double) (input.w - j - 1) / input.w;
                DT[i][k] = (double) input.pixels[i][j] / 10;
                k++;
            }
        }
        for (; k < MAXMDF; k++)
        {
            LT[i][k] = 0;
            DT[i][k] = 0;
        }
    }

    r = input.h % FEATUREH;
    s = 0;
    while (s < input.h)
    {
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] = 0;
        }
        k = input.h / FEATUREH;
        k += (r > 0) ? 1 : 0;
        for (i = 0; i < k; i++)
        {
            for (j = 0; j < MAXMDF; j++)
            {
                featureVector[t+j] += LT[s+i][j];
                featureVector[t+j+MAXMDF] += DT[s+i][j];
            }
        }
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] /= k;
        }
        t += 2*MAXMDF;
        s += k;
        r--;
        //printf("t = %d s = %d r = %d k = %d\n", t, s, r, k);
    }

    /// right to left phase
    for (i = 0; i < input.h; i++)
    {
        k = 0;
        for (j = input.w-1; j >= 0; j--)
        {
            if (k >= MAXMDF)
                break;
            if (input.pixels[i][j] && ((j == input.w-1) || (input.pixels[i][j+1] == 0)))
            {
                LT[i][k] = (double) j / input.w;
                DT[i][k] = (double) input.pixels[i][j] / 10;
                k++;
            }
        }
        for (; k < MAXMDF; k++)
        {
            LT[i][k] = 0;
            DT[i][k] = 0;
        }
    }

    r = input.h % FEATUREH;
    s = 0;
    while (s < input.h)
    {
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] = 0;
        }
        k = input.h / FEATUREH;
        k += (r > 0) ? 1 : 0;
        for (i = 0; i < k; i++)
        {
            for (j = 0; j < MAXMDF; j++)
            {
                featureVector[t+j] += LT[s+i][j];
                featureVector[t+j+MAXMDF] += DT[s+i][j];
            }
        }
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] /= k;
        }
        t += 2*MAXMDF;
        s += k;
        r--;
    }
    for (i = 0; i < input.h; i++)
    {
        free(LT[i]);
        free(DT[i]);
    }
    free(LT);
    free(DT);

    /// get column's MDF
    LT = (double**) malloc(input.w*sizeof(double*));
	DT = (double**) malloc(input.w*sizeof(double*));
	for (i = 0; i < input.w; i++)
	{
	    LT[i] = (double*) malloc(MAXMDF*sizeof(double));
	    DT[i] = (double*) malloc(MAXMDF*sizeof(double));
    }
    /// up to down phase
    for (j = 0; j < input.w; j++)
    {
        k = 0;
        for (i = 0; i < input.h; i++)
        {
            if (k >= MAXMDF)
                break;
            if (input.pixels[i][j] && ((i == 0) || (input.pixels[i-1][j] == 0)))
            {
                LT[j][k] = (double) (input.h - i - 1) / input.h;
                DT[j][k] = (double) input.pixels[i][j] / 10;
                k++;
            }
        }
        for (; k < MAXMDF; k++)
        {
            LT[j][k] = 0;
            DT[j][k] = 0;
        }
    }

    r = input.w % FEATUREH;
    s = 0;
    while (s < input.w)
    {
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] = 0;
        }
        k = input.w / FEATUREH;
        k += (r > 0) ? 1 : 0;
        for (i = 0; i < k; i++)
        {
            for (j = 0; j < MAXMDF; j++)
            {
                featureVector[t+j] += LT[s+i][j];
                featureVector[t+j+MAXMDF] += DT[s+i][j];
            }
        }
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] /= k;
        }
        t += 2*MAXMDF;
        s += k;
        r--;
    }

    /// down to up phase
    for (j = 0; j < input.w; j++)
    {
        k = 0;
        for (i = input.h-1; i >= 0; i--)
        {
            if (k >= MAXMDF)
                break;
            if (input.pixels[i][j] && ((i == input.h-1) || (input.pixels[i+1][j] == 0)))
            {
                LT[j][k] = (double) i / input.h;
                DT[j][k] = (double) input.pixels[i][j] / 10;
                k++;
            }
        }
        for (; k < MAXMDF; k++)
        {
            LT[j][k] = 0;
            DT[j][k] = 0;
        }
    }

    r = input.w % FEATUREH;
    s = 0;
    while (s < input.w)
    {
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] = 0;
        }
        k = input.w / FEATUREH;
        k += (r > 0) ? 1 : 0;
        for (i = 0; i < k; i++)
        {
            for (j = 0; j < MAXMDF; j++)
            {
                featureVector[t+j] += LT[s+i][j];
                featureVector[t+j+MAXMDF] += DT[s+i][j];
            }
        }
        for (j = 0; j < 2*MAXMDF; j++)
        {
            featureVector[t+j] /= k;
        }
        t += 2*MAXMDF;
        s += k;
        r--;
    }
    for (i = 0; i < input.w; i++)
    {
        free(LT[i]);
        free(DT[i]);
    }
    free(LT);
    free(DT);

	return featureVector;
}
