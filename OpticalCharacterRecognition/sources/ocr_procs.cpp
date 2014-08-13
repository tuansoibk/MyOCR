#include "bmp.h"
#include "pre_procs.h"
#include "segment.h"
#include "neuron.h"
#include "common.h"
#include "feature.h"
#include "char_recog.h"
#include "ocr_procs.h"
#include <string.h>

#define IMG_TRAIN 0
#define IMG_TEST 1

int group_type[NUM_CHAR_MAX];
int char_count;

extern int NUM_HID_CHAR;

extern Rect *charRect;

char str_train[NUM_CHAR_TRAIN];
char str_test[NUM_CHAR_TEST];

#define MAX_TRAIN_FILE 7
#define MAX_TRAIN_CHAR 45
char trainStr[] = "thequickbrownfoxjumpsoverthelazydog0123456789";

void getCharFromInt(char* charInt, int n) {

    charInt[0] = (n / 100) % 10 + '0';
    charInt[1] = (n / 10) % 10 + '0';
    charInt[2] = (n % 10) + '0';
    charInt[3] = '\0';
}

/**
 * OCR process from gray image input
 */
int ocrProcess(Image binImg) {

    int i, h, w;
    h = binImg.h;
    w = binImg.w;
    int count =0, lCount = 0;

    int start = 0;
    int endLine = 0;
    int nLine;
    Rect rect;

    int* charIndex[20];

    system("rm -rf data");
    system("mkdir -p data");
    system("rm -rf leven");
    system("mkdir -p leven");


    createNetworkFromFile();

    /// segmentation processing
    while((nLine = ccLineLabeling(binImg, start))!= 0) {
        if(nLine < 3) continue;
        printf("%d\n",nLine);
        int baseLine = getBaselineHeight(nLine);

//        int avgSpace = getAverageSpace(nLine);
        /// get slant deg
        int slantDeg = getSlantDeg(binImg, baseLine, nLine);
//        printf("slantDeg = %d\n", slantDeg);

        int nChar = 0;

        for(i = 0; i < nLine; i++) {
            int index = getSegmentIndex(i);
//            printf("%d ", index);
            rect = getCharRect(index);
            endLine = endLine > (rect.y + rect.h) ? endLine :(rect.y + rect.h);
            Image charImg = getImageFromRect(binImg, rect, index);
            if(slantDeg) {
                charImg = slantRemoval(charImg, slantDeg);
            }
//            char* name = (char*) malloc(20* sizeof(char));
//            sprintf(name, "out_s_%d_%d.txt", lCount, i);
//            printImage(charImg, name);

            charImg = contourFinding(charImg);
            smoothContour(charImg);
            thinImageOnMatch(charImg);
            double* input = getFeature(charImg);
            double prop, maxProp = 0.0;

            /// recognition
            nChar++;
            charIndex[nChar-1] = charRecognition(input, &prop);


            /// check space
            //int space = getSpace(i, nLine, binImg);
            //if(space > baseLine/2) {//printf(" ");
            //    createWordFst(charIndex, nChar);
            //    nChar = 0;
            //    printf(" ");
            //}
            deleteImage(charImg);
            free(input);
            count++;
//            }
//            printf(" ");
//            start_w = end_w;
//            deleteImage(wordImg);
        }
        printf("\n");
        start = endLine;
        lCount++;
    }
    deleteImage(binImg);
    deleteNetwork();
    printf("%d\n", count);
    return 0;
}

int ocrProcessFile(char* fileName) {

    int i, h, w;
    uint32 **pix;

    createNetworkFromFile();

    pix = ReadBmp(fileName, &h, &w);
    Image inputImg = getGrayLevel(pix, h, w);
    for(i = 0; i < h; i++) {
        free(pix[i]);
    }
    free(pix);
    ocrProcess(inputImg);
    return 0;
}

/**
 * Get feature image
 */
double** getFeatureFromImage(Image binImg, int nChar) {
    int i, h, w;
    double **outFeature;

    /// alloc output
    outFeature = (double **)malloc(nChar * sizeof(double *));

    h = binImg.h;
    w = binImg.w;


    int count =0;


    /// segmentation processing
//    int nSegment = ccLabeling(binImg);
    //printf("%d\n", nSegment);
    int start = 0;
    int endLine = 0;
    int nLine;
    int lCount = 0;
    Rect rect;
    while((nLine = ccLineLabeling(binImg, start))!= 0) {

//        printf("line = %d\n", nLine);
        int baseLine = getBaselineHeight(nLine);
        /// get slant deg
        int slantDeg = getSlantDeg(binImg, baseLine, nLine);
        for(i = 0; i < nLine; i++) {
            int index = getSegmentIndex(i);
            rect = getCharRect(index);
            endLine = endLine > (rect.y + rect.h) ? endLine :(rect.y + rect.h);
            Image charImg = getImageFromRect(binImg, rect, index);
            if(slantDeg) {
                charImg = slantRemoval(charImg, slantDeg);
            }
            //if(lCount==5&&i==0) printf("%d %d\n", rect.x, rect.y);
//            char* name = (char*) malloc(20* sizeof(char));
//            sprintf(name, "out_s_%d_%d.txt", lCount, i);
//            printImage(charImg, name);

            /// charImg processing
            /// slant correction


            charImg = contourFinding(charImg);
            smoothContour(charImg);
            thinImageOnMatch(charImg);

            outFeature[count] = getFeature(charImg);

            count++;
            deleteImage(charImg);

        }
        start = endLine;
        lCount++;
    }
    printf("count = %d\n", count);
    char_count = count;
    return outFeature;
}

/**
 * Training process
 */
int trainProcess(Image trainImg, Image testImg, double *mse) {
    int i, cnt = 0;
    double *outValue;

    /// training varian
    int trainComplete = 0;
    double charMse;
    double minCharMse = 1000;

    FILE *f = fopen("train.txt", "r");
    char r;
    i = 0;
    while (fscanf(f, "%c", &r) != EOF) {
        str_train[i] = r;
        i++;
    }
    fclose(f);

    f = fopen("test.txt", "r");
    i = 0;
    while (fscanf(f, "%c", &r) != EOF) {
        str_test[i] = r;
        i++;
    }
    fclose(f);

    /// init network randomly
    initNetwork();
//    createNetworkFromFile();
    /// get feature from image
    double **trainImgFeature = getFeatureFromImage(trainImg, NUM_CHAR_TRAIN);
    double **testImgFeature = getFeatureFromImage(testImg, NUM_CHAR_TEST);
//    return 0;
    int nOver = 0;
    int saved = 0;
    printf("start training\n");
    while (cnt < NUM_LOOP) {
        cnt++;

        /** processing feature data */
        /// training network
        for (i = 0; i < NUM_CHAR_TRAIN; i ++) {
            outValue = getOutFromChar(str_train[i]);
            if (!trainComplete) {
                trainNetwork(trainImgFeature[i], outValue, 0.1);
            }
            free(outValue);
        }

        /// testing network
        charMse = 0;
        for (i = 0; i < NUM_CHAR_TEST; i ++) {
            outValue = getOutFromChar(str_test[i]);
            charMse += testError(testImgFeature[i], outValue);
            free(outValue);
        }
        /// check if train complete
        if (charMse < minCharMse) {
            nOver = 0;
            minCharMse = charMse;
        } else {
            if(nOver == 0) {
                saved = 1;
                ///save network to file
                saveNetworkToFile("train.wdb");
            }
            printf("over\n");
            nOver ++;
            if(nOver > 100)
                trainComplete = 1;
        }

        printf("%.2lf %d\n", charMse, cnt);

        if (trainComplete) break;
    }

    if(!saved) {
        saveNetworkToFile("train.wdb");
    }

    if (mse != NULL)
    {
        *mse = minCharMse;
    }
    for (i = 0; i < NUM_CHAR_TRAIN; i++)
        free(trainImgFeature[i]);
    for (i = 0; i < NUM_CHAR_TEST; i++)
        free(testImgFeature[i]);

    return 0;
}

/// train without validation
int trainLoopProcess(Image trainImg, char* txtFileName, int nLoop) {
    int i, cnt = 0;
    double *outValue;

    /// read text file

    FILE *f = fopen(txtFileName, "r");
    fscanf(f, "%s", str_train);
    int nChar = strlen(str_train);
    printf("%d", nChar);

    /// init network randomly
//    initNetwork();
    createNetworkFromFile();
    /// get feature from image
    double **trainImgFeature = getFeatureFromImage(trainImg, nChar);
//    return 0;
    printf("start training\n");
    while (cnt < nLoop) {
        cnt++;
        printf("%d\n", cnt);
        /** processing feature data */
        /// training network
        for (i = 0; i < nChar; i ++) {
            outValue = getOutFromChar(str_train[i]);

            trainNetwork(trainImgFeature[i], outValue, 0.1);
            free(outValue);
        }
    }

    /** save network to file */
    saveNetworkToFile("trainloop.wdb");

    for (i = 0; i < nChar; i++)
        free(trainImgFeature[i]);

    return 0;
}

int autoTrainProcess(int nTrainImg, int nTestImg, double *mse) {
    int i, j, h, w, cnt = 0;
    //int nTrainChar[nTrainImg];
    //int nTestChar[nTestImg];
	int *nTrainChar = (int*)calloc(nTrainImg, sizeof(int));
	int *nTestChar = (int*)calloc(nTestImg, sizeof(int));

    //double ** trainFeature[nTrainImg];
	double ***trainFeature = (double***)calloc(nTrainImg, sizeof(double**));
    //double ** testFeature[nTestImg];
	double ***testFeature = (double***)calloc(nTestImg, sizeof(double**));
    uint32 **pix;
    FILE *f;
    //char *str_train[nTrainImg];
	char **str_train = (char**)calloc(nTrainImg, sizeof(char*));
    //char *str_test[nTestImg];
	char **str_test = (char**)calloc(nTestImg, sizeof(char*));

    char trainImgName[100];
    char trainTxtName[100];
    char testImgName[100];
    char testTxtName[100];


    /// read train image + text

    for(i = 0; i < nTrainImg; i++) {

        sprintf(trainTxtName, "train/%d.txt", i+1);
        f = fopen(trainTxtName, "r");
        fscanf(f, "%d\n", &(nTrainChar[i]));

        str_train[i] = (char *) malloc(nTrainChar[i] *sizeof(char));
        for(j = 0; j < nTrainChar[i]; j++) {
            fscanf(f, "%c", &(str_train[i][j]));
        }
        fclose(f);

        sprintf(trainImgName, "train/%d.bmp", i+1);
        pix = ReadBmp(trainImgName, &h, &w);
        if (pix == NULL)
            return -1;
        Image trainImg = getGrayLevel(pix, h, w);
        for(j = 0; j < h; j++) {
            free(pix[j]);
        }
        free(pix);
        trainImg = binarizeGrayImageKmeans(trainImg);

        trainFeature[i] = getFeatureFromImage(trainImg, nTrainChar[i]);
    }
    printf("start read test img\n");
    /// read test img + text
    for(i = 0; i < nTestImg; i++) {

        sprintf(testTxtName, "test/%d.txt", i+1);
        f = fopen(testTxtName, "r");
        fscanf(f, "%d\n", &(nTestChar[i]));

        str_test[i] = (char *) malloc(nTestChar[i] *sizeof(char));
        for(j = 0; j < nTestChar[i]; j++) {
            fscanf(f, "%c", &(str_test[i][j]));
            printf("%c", str_test[i][j]);
        }
        fclose(f);

        sprintf(testImgName, "test/%d.bmp", i+1);
        pix = ReadBmp(testImgName, &h, &w);
        if (pix == NULL)
            return -1;
        Image testImg = getGrayLevel(pix, h, w);
        for(j = 0; j < h; j++) {
            free(pix[j]);
        }
        free(pix);
        testImg = binarizeGrayImageKmeans(testImg);

        testFeature[i] = getFeatureFromImage(testImg, nTestChar[i]);
    }

    /// init network randomly
    initNetwork();

    double *outValue;
    double charMse, minCharMse = 10000;
    int trainComplete = 0;
    /// training process
    int nOver = 0;
    int saved = 0;
    char savedFileName[100];
    printf("start training\n");
    while (cnt < NUM_LOOP) {
        cnt++;

        /** processing feature data */
        /// training network
        for (i = 0; i < nTrainImg; i++) {
            for(j = 0; j < nTrainChar[i]; j++) {
                outValue = getOutFromChar(str_train[i][j]);
                trainNetwork(trainFeature[i][j], outValue, 0.1);
                free(outValue);
            }
        }

        /// testing network
        charMse = 0;
        for (i = 0; i < nTestImg; i ++) {
            for (j = 0; j < nTestChar[i]; j++) {
                outValue = getOutFromChar(str_test[i][j]);
                charMse += testError(testFeature[i][j], outValue);
                free(outValue);
            }
        }

        /// check if train complete
        if (charMse < minCharMse) {
            nOver = 0;
            minCharMse = charMse;
        } else {
            if(nOver == 0) {
                ///save network to file
                saved = 1;
                sprintf(savedFileName, "%d_%.4lf.wdb", NUM_HID_CHAR, minCharMse);
                saveNetworkToFile(savedFileName);
            }
            printf("over\n");
            nOver ++;
            if(nOver > 100)
                trainComplete = 1;
        }

        printf("%.2lf %d\n", charMse, cnt);

        if (trainComplete) break;
    }
    if(!saved) {
        sprintf(savedFileName, "%d_%.4lf.wdb", NUM_HID_CHAR, minCharMse);
        saveNetworkToFile(savedFileName);
    }
    if (mse != NULL)
    {
        *mse = minCharMse;
    }
    for (i = 0; i < nTrainImg; i++) {
        for (j = 0; j < nTrainChar[i]; j++) {
            free(trainFeature[i][j]);
        }
    }
    for (i = 0; i < nTestImg; i++) {
        for (j = 0; j < nTestChar[i]; j++) {
            free(testFeature[i][j]);
        }
    }

    return 0;
}

