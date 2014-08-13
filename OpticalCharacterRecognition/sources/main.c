#include "ocr_procs.h"
#include "pre_procs.h"
#include "bmp.h"
#include "common.h"
#include "char_recog.h"
#include "segment.h"
#include "feature.h"
#include <time.h>

int NUM_HID_CHAR = 20;

int NUM_TRAIN_FILE = 1;
int NUM_TEST_FILE = 1;

int main(int argc, char* argv[])
{
	uint32 **pix;
	int h;
	int w;
	time_t startTime, endTime;
	Image *pLineImageList = NULL;
	Image **pCharImageList = NULL;
	int *pCharNumberList = NULL;
	int nLines;
	int baseLine;
	double slantAngle;

	time(&startTime);
	printf("start reading bmp\n");	
	//pix = ReadBmp("D:\\02_Workspace\\Optical_Character_Recognition\\OpticalCharacterRecognition\\Debug\\test.bmp", &h, &w);
	pix = ReadBmp("D:\\02_Workspace\\Optical_Character_Recognition\\OpticalCharacterRecognition\\Debug\\test.bmp", &h, &w);
	if (pix != NULL)
	{
		printf_s("finish reading bmp, width = %d, height = %d.\n", w, h);
		Image grayImg = getGrayLevel(pix, h, w);
		grayImg = binarizeGrayImageKmeans(grayImg);
		//ccLineLabeling(grayImg, 0);		
		grayImg = FilterByCharSize(grayImg, &pLineImageList, &pCharImageList, &pCharNumberList, &nLines);
		//WriteImageToText("C:\\temp\\test1.txt", grayImg);

		if (nLines != 0)
		{
			char *fileName;
			Image charImage;

			baseLine = GetBaseLine(pCharImageList, pCharNumberList, nLines);
			slantAngle = GetSlantAngle(pLineImageList, nLines, baseLine);
			printf_s("slant angle = %.2f\n", slantAngle);

			fileName = (char*)malloc(255);
			for (int line = 0; line < nLines; line++)
			{
				for (int chr = 0; chr < pCharNumberList[line]; chr++)
				{
					//charImage = CropImage(pLineImageList[line], pLineRectList[line][chr]);
					charImage = RemoveSlant(pCharImageList[line][chr], slantAngle);
					charImage = ExtractContour(charImage);
					SmoothContour(&charImage);
					ThinOutContour(&charImage);
					GetFeature(charImage);
					sprintf(fileName, "C:\\temp\\line%d_char%d.txt", line, chr);
					WriteImageToText(fileName, charImage);
				}
			}
		}
		
		time(&endTime);
		printf("%f\n", difftime(endTime, startTime));
	}
	getchar();
}

int main1(int argc, char* argv[]) {

    int i;
    int h, w;
    uint32 **pix;
    char* testFile = "testImg.bmp";
    char* trainFile = "trainImg.bmp";

    uint16 c;
    double mse;
    double minMse, totalMse, avgMse;
    int autorun, test, train, loopTrain, trainingCase, trainingLoop, nLoop;

    autorun = test = train = loopTrain = trainingCase = 0;
    trainingLoop = 10;
    //opterr = 0;

    //while ((c = getopt(argc, argv, "a:t:r:l:")) != -1)
    {
        switch (c)
        {
            case 'a':
                autorun = 1;
                //trainingLoop = atoi(optarg);
                if (trainingLoop < 0)
                    trainingLoop = 10;
                break;
            case 't':
                //testFile = optarg;
                test = 1;
                break;
            case 'r':
//                trainFile = optarg;
                train = 1;
                break;
            case 'l':
                //nLoop = atoi(optarg);
                loopTrain = 1;
                break;
            default:
                abort();
        }
    }

    if (train)
    {
        /** read train + test bmp file */
        pix = ReadBmp(trainFile, &h, &w);
        if (pix == NULL)
            return -1;
        Image trainImg = getGrayLevel(pix, h, w);
        for(i = 0; i < h; i++) {
            free(pix[i]);
        }
        free(pix);
        trainImg = binarizeGrayImageKmeans(trainImg);

        pix = ReadBmp(testFile, &h, &w);
        if (pix == NULL)
            return -1;
        Image testImg = getGrayLevel(pix, h, w);
        for(i = 0; i < h; i++) {
            free(pix[i]);
        }
        free(pix);
        testImg = binarizeGrayImageKmeans(testImg);


        if(loopTrain)
            trainLoopProcess(trainImg, "test2.txt", nLoop);
        else
            trainProcess(trainImg, testImg, NULL);
    }
    else if (autorun)
    {
        /// train upper network
        FILE *f = fopen("upperhid.txt", "w");
        printf("Training upper network process\n");
        minMse = 1000;
        NUM_HID_CHAR = 20;
        while (NUM_HID_CHAR < 21)
        {
            printf("Hidden node: %d\n", NUM_HID_CHAR);
            totalMse = 0;
            for (i = 0; i < trainingLoop; i++)
            {
                printf("Loop: %d\n", i);
                autoTrainProcess(NUM_TRAIN_FILE, NUM_TEST_FILE, &mse);
                totalMse += mse;
            }
            avgMse = totalMse / trainingLoop;
            fprintf(f, "hidden node = %d, average mse = %f\n", NUM_HID_CHAR, avgMse);
            NUM_HID_CHAR++;
        }
        fclose(f);
    }
    else if (test)
    {
        /** read input bmp file */
        pix = ReadBmp(testFile, &h, &w);
        if (pix == NULL)
            return -1;
    //    write_BMP("test_out.bmp",pix, h, w);
        Image inputImg = getGrayLevel(pix, h, w);
        for(i = 0; i < h; i++) {
            free(pix[i]);
        }
        free(pix);
        inputImg = binarizeGrayImageKmeans(inputImg);
        printImage(inputImg, "out.txt");
        ocrProcess(inputImg);
    }

    return 0;
}

