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

extern int NUM_HID_UPPER;
extern int NUM_HID_NORM;
extern int NUM_HID_LOWER;

extern Rect *charRect;

//char str_train[NUM_CHAR_TRAIN] = "quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG\
//quickbrownfoxjumpsoverthelazydog\
//QUICKBROWNFOXJUMPSOVERTHELAZYDOG";
char str_train[NUM_CHAR_TRAIN + 1] = "quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog\
quickbrownfoxjumpsoverthelazydog";

//char str_test[NUM_CHAR_TEST] = "Ineithertypeofformreaders\
//thesystemextractsthesignature\
//suchasapreprintedlogoora\
//characterstringofeachtraining\
//formduringthetrainingphase\
//andstorestheinformationinthelong\
//termmemoryoftheknowledgebase\
//Intheworkingphasethe\
//signatureofaninputform\
//isextractedandcomparedstatistically\
//andsyntacticallytothe\
//knowledgeinthedatabase\
//Bydefiningasimilaritybetween\
//signaturesfromthetwoformimages\
//wewillbeabletoidentifythe\
//formatoftheinputform\
//IftheinputformIsofknowntype\
//accordingtotheformregistration\
//Informationthepertinentitems\
//canbeextracteddirectlyfrom\
//approximatepositionsOtherwise\
//thesignatureofthisunknownformcanbe\
//registeredthroughhumanintervention";

char str_test[NUM_CHAR_TEST + 1] = "ineithertypeofformreaders\
thesystemextractsthesignature\
suchasapreprintedlogoora\
characterstringofeachtraining\
formduringthetrainingphase\
andstorestheinformationinthelong\
termmemoryoftheknowledgebase\
intheworkingphasethe\
signatureofaninputform\
isextractedandcomparedstatistically\
andsyntacticallytothe\
knowledgeinthedatabase\
bydefiningasimilaritybetween\
signaturesfromthetwoformimages\
wewillbeabletoidentifythe\
formatoftheinputform\
iftheinputformisofknowntype\
accordingtotheformregistration\
informationthepertinentitems\
canbeextracteddirectlyfrom\
approximatepositionsotherwise\
thesignatureofthisunknownformcanbe\
registeredthroughhumanintervention";


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

    char recChar, outChar;

    createNetworkFromFile();

    /// segmentation processing
    while((nLine = ccLineLabeling(binImg, start))!= 0) {
        if(nLine < 3) continue;
        printf("%d\n",nLine);
        int baseLine = getBaselineHeight(nLine);
        int avgSpace = getAverageSpace(nLine);
        /// get slant deg
        int slantDeg = getSlantDeg(binImg, baseLine, nLine);
//        printf("slantDeg = %d\n", slantDeg);
        for(i = 0; i < nLine; i++) {
            int index = getSegmentIndex(i);
//            printf("%d ", index);
            rect = getCharRect(index);
            endLine = endLine > (rect.y + rect.h) ? endLine :(rect.y + rect.h);
            Image charImg = getImageFromRect(binImg, rect, index);
            if(slantDeg) {
                charImg = slantRemoval(charImg, slantDeg);
            }
            char* name = (char*) malloc(20* sizeof(char));
            sprintf(name, "out_s_%d_%d.txt", lCount, i);
            printImage(charImg, name);

            charImg = contourFinding(charImg);
            smoothContour(charImg);
            thinImageOnMatch(charImg);
            double* input = getFeature(charImg);
            double prop, maxProp = 0.0;

            /// recognition
            recChar = charRecognition(input, &prop)[0] + 'a';
            if(prop > maxProp) {
                outChar = recChar;
                printf("%c", outChar);
            }
            /// add space
            if(getSpace(i, nLine, binImg) > avgSpace) printf(" ");
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
    return 0;
}

int ocrProcessFile(char* fileName) {

    int i, h, w;
    unsigned long ** pix;

    createNetworkFromFile();

    pix = read_BMP(fileName, &h, &w);
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

        printf("line = %d\n", nLine);
//        int baseLine = getBaselineHeight(nLine);
        /// get slant deg
        int slantDeg = 0;//getSlantDeg(binImg, baseLine, nLine);
        for(i = 0; i < nLine; i++) {
            int index = getSegmentIndex(i);
            rect = getCharRect(index);
            endLine = endLine > (rect.y + rect.h) ? endLine :(rect.y + rect.h);
            Image charImg = getImageFromRect(binImg, rect, index);
            if(slantDeg) {
                charImg = slantRemoval(charImg, slantDeg);
            }
            //if(lCount==5&&i==0) printf("%d %d\n", rect.x, rect.y);
            char* name = (char*) malloc(20* sizeof(char));
            sprintf(name, "out_s_%d_%d.txt", lCount, i);
            printImage(charImg, name);

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

    /// init network randomly
    initNetwork();
//    createNetworkFromFile();
    /// get feature from image
    double **trainImgFeature = getFeatureFromImage(trainImg, NUM_CHAR_TRAIN);
    double **testImgFeature = getFeatureFromImage(testImg, NUM_CHAR_TEST);
//    return 0;
    int nOver = 0;
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
                ///save network to file
                saveNetworkToFile("weight.txt");
            }
            printf("over\n");
            nOver ++;
            if(nOver > 100)
                trainComplete = 1;
        }

        printf("%.2lf %d\n", charMse, cnt);

        if (trainComplete) break;
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
    saveNetworkToFile("weight.txt");

    for (i = 0; i < nChar; i++)
        free(trainImgFeature[i]);

    return 0;
}
