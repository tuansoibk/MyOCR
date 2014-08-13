#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bmp.h"
#include "common.h"
#define BPP 24

int charArray2Int(unsigned char * b, int s, int f)
{
	int ret = 0;
	int i;
	int shift = 0;

	for (i = s; i <= f; i++) {
		ret = ret | ((b[i] & 0xff) << shift);
		shift += 8;
	}
	return ret;
}

void intToCharArray(int val, unsigned char * data, int size) {
    int i;
    int shift = 0;

    for(i = 0; i < size; i++) {
        data[i] = (val >> shift) & 0xff;
        shift += 8;
    }
}

uint32** ReadBmp(char *fileName, sint32 *height, sint32 *width)
{
	uint32 **pPixels = NULL;
	FILE *pFile = NULL;
	PBITMAPFILEHEADER pBitmapHeader = NULL;
	PBITMAPINFOHEADER pBitmapInfo = NULL;
	uint16 nBytes;
	uint8 nBytePerPixel;
	uint8 nPadBytes;
	uint8 *pBuf = NULL;
	bool success = true;

	*height = 0;
	*width = 0;
	fopen_s(&pFile, fileName, "rb");
	if (pFile == NULL)
	{
		printf_s("Unable to open specific file: %s.\n", fileName);
		success = false;
	}
	else
	{
		pBitmapHeader = (PBITMAPFILEHEADER)malloc(BITMAPFILEHEADER_SIZE);
		nBytes = fread_s(pBitmapHeader, BITMAPFILEHEADER_SIZE, 1, BITMAPFILEHEADER_SIZE, pFile);
		if (nBytes != BITMAPFILEHEADER_SIZE) // unable to read file header
		{
			printf_s("Cannot read file header.\n");
			success = false;
		}
		else if (pBitmapHeader->bfType != WINDOW_BITMAP_TYPE) // not window bitmap format
		{
			printf_s("Invalid bitmap header: 0x%04x.\n", pBitmapHeader->bfType);
			success = false;
		}
		else if (pBitmapHeader->bfOffBits != SUPPORTED_OFFSET)
		{
			printf_s("Unsupported bitmap format, header size = %d.\n", pBitmapHeader->bfOffBits);
			success = false;
		}
		else
		{
			pBitmapInfo = (PBITMAPINFOHEADER)malloc(BITMAPINFOHEADER_SIZE);
			nBytes = fread_s(pBitmapInfo, BITMAPINFOHEADER_SIZE, 1, BITMAPINFOHEADER_SIZE, pFile);
			if (nBytes != BITMAPINFOHEADER_SIZE) // unable to read info header
			{
				printf_s("Cannot read bitmap info.\n");
				success = false;
			}
			// skip checking the info header size 
			// cause we've already checked the pixel data offset
			else if (pBitmapInfo->biPlanes != SUPPORTED_COLOR_PLANE)
			{
				printf_s("Invalid color planes: %d.\n", pBitmapInfo->biPlanes);
				success = false;
			}
			else if ((pBitmapInfo->biBitCount != 8) && (pBitmapInfo->biBitCount != 16) &&
					(pBitmapInfo->biBitCount != 24) && (pBitmapInfo->biBitCount != 32))
			{
				printf_s("Unsupported bits per pixel: %d.\n", pBitmapInfo->biBitCount);
				success = false;
			}
			else if (pBitmapInfo->biCompression != NO_COMPRESSION)
			{
				printf_s("Unsupported compression mode: %d.\n", pBitmapInfo->biCompression);
				success = false;
			}
			else
			{
				// start reading pixel data
				*height = pBitmapInfo->biHeight;
				*width = pBitmapInfo->biWidth;
				nBytePerPixel = pBitmapInfo->biBitCount / 8;
				nPadBytes = (4 - ((pBitmapInfo->biWidth % 4) * (nBytePerPixel % 4)) % 4) % 4;
				pBuf = (uint8*)malloc(nBytePerPixel);
				pPixels = (uint32**)malloc(pBitmapInfo->biHeight * sizeof(uint32*));
				for (int y = pBitmapInfo->biHeight - 1; y >= 0; y--)
				{
					pPixels[y] = (uint32*)calloc(pBitmapInfo->biWidth, sizeof(uint32));
					for (int x = 0; x < pBitmapInfo->biWidth; x++)
					{
						nBytes = fread_s(pBuf, nBytePerPixel, 1, nBytePerPixel, pFile);
						if (nBytes != nBytePerPixel) // error occurred
						{
							printf_s("Cannot read pixel at x = %d, y = %d.\n", x, y);
							success = false;
							break;
						}
						pPixels[y][x] = ByteArrayToInt(pBuf, nBytePerPixel);
					}
					if (nBytes != nBytePerPixel) // error occurred
					{
						success = false;
						break;
					}
					else
					{
						// read padding bytes
						nBytes = 0;
						nBytes = fread_s(pBuf, nBytePerPixel, 1, nPadBytes, pFile);
						if (nBytes != nPadBytes)
						{
							printf_s("Cannot read padding bytes at row %d.\n", y);
							success = false;
							break;
						}
					}
				}
			}
		}
	}

	// clean up
	if (pFile != NULL)
	{
		fclose(pFile);
		pFile = NULL;
	}
	if (pBitmapHeader != NULL)
	{
		free(pBitmapHeader);
		pBitmapHeader = NULL;
	}
	if (pBitmapInfo != NULL)
	{
		free(pBitmapInfo);
		pBitmapInfo = NULL;
	}
	if (pBuf != NULL)
	{
		free(pBuf);
		pBuf = NULL;
	}
	if (!success)
	{
		if (pPixels != NULL)
		{
			for (int i = 0; i < *height; i++)
			{
				if (pPixels[i] != NULL)
				{
					free(pPixels[i]);
				}
			}
			free(pPixels);
			pPixels = NULL;
		}

		*height = 0;
		*width = 0;
	}

	return pPixels;
}

int WriteBmp(char *fileName, uint32 **pPixels, sint32 height, sint32 width)
{
	FILE *pFile = NULL;
	PBITMAPFILEHEADER pBitmapHeader = NULL;
	PBITMAPINFOHEADER pBitmapInfo = NULL;
	uint16 nBytes;
	uint8 nBytePerPixel;
	uint8 nPadBytes;
	uint8 *pBuf = NULL;

	fopen_s(&pFile, fileName, "wb");
	if (pFile == NULL)
	{
		printf_s("Unable to write to specific file: %s.\n", fileName);
	}
	else
	{
		nBytePerPixel = SUPPORTED_BIT_COUNT / 8;
		nPadBytes = (4 - ((width % 4) * nBytePerPixel) % 4) % 4;
		pBitmapHeader = (PBITMAPFILEHEADER)malloc(BITMAPFILEHEADER_SIZE);
		pBitmapHeader->bfType = WINDOW_BITMAP_TYPE;
		pBitmapHeader->bfOffBits = SUPPORTED_OFFSET;
		pBitmapHeader->bfSize = (width * nBytePerPixel + nPadBytes) * height + SUPPORTED_OFFSET;

		nBytes = fwrite(pBitmapHeader, 1, BITMAPFILEHEADER_SIZE, pFile);
		if (nBytes != BITMAPFILEHEADER_SIZE)
		{
			printf_s("Cannot write file header.\n");
		}
		else
		{
			pBitmapInfo = (PBITMAPINFOHEADER)malloc(BITMAPINFOHEADER_SIZE);
			pBitmapInfo->biSize = BITMAPINFOHEADER_SIZE;
			pBitmapInfo->biWidth = width;
			pBitmapInfo->biHeight = height;
			pBitmapInfo->biPlanes = SUPPORTED_COLOR_PLANE;
			pBitmapInfo->biBitCount = SUPPORTED_BIT_COUNT;
			pBitmapInfo->biCompression = NO_COMPRESSION;
			pBitmapInfo->biXPelsPerMeter = 0;
			pBitmapInfo->biYPelsPerMeter = 0;
			pBitmapInfo->biClrUsed = 0;
			pBitmapInfo->biClrImportant = 0;

			nBytes = fwrite(pBitmapInfo, 1, BITMAPINFOHEADER_SIZE, pFile);
			if (nBytes != BITMAPINFOHEADER_SIZE)
			{
				printf_s("Cannot write bitmap info.\n");
			}
			else
			{
				// start writing pixel data
				for (int y = height - 1; y >= 0; y--)
				{
					for (int x = 0; x < width; x++)
					{
						IntToByteArray(&pBuf, pPixels[y][x], nBytePerPixel);
						nBytes = fwrite(pBuf, 1, nBytePerPixel, pFile);
						if (nBytes != nBytePerPixel)
						{
							printf_s("Cannot write pixel at x = %d, y = %d.\n", x, y);
							break;
						}
					}
					if (nBytes != nBytePerPixel)
					{
						break;
					}
					else
					{
						// write padding bytes
						nBytes = 0;
						IntToByteArray(&pBuf, 0, nPadBytes);
						nBytes = fwrite(pBuf, 1, nPadBytes, pFile);
						if (nBytes != nPadBytes)
						{
							printf_s("Cannot write padding bytes at row %d.\n", y);
							break;
						}
					}
				}
			}
		}
	}

	// clean up
	if (pFile != NULL)
	{
		fclose(pFile);
		pFile = NULL;
	}
	if (pBitmapHeader != NULL)
	{
		free(pBitmapHeader);
		pBitmapHeader = NULL;
	}
	if (pBitmapInfo != NULL)
	{
		free(pBitmapInfo);
		pBitmapInfo = NULL;
	}
	if (pBuf != NULL)
	{
		free(pBuf);
		pBuf = NULL;
	}

	return 0;
}

int WriteImageToText(char *fileName, Image img)
{
	FILE *pFile = NULL;
	char pBuf[4];
	char* pBlank = "   ";
	int nBytes;

	fopen_s(&pFile, fileName, "w");
	if (pFile == NULL)
	{
		printf_s("Unable to write to specific file: %s.\n", fileName);
	}
	else
	{
		for (int y = 0; y < img.h; y++)
		{
			for (int x = 0; x < img.w; x++)
			{
				if (img.pixels[y][x] != 0)
				{
					sprintf_s(pBuf, "%3d", img.pixels[y][x]);
					nBytes = fwrite(pBuf, 1, 3, pFile);
				}
				else
				{
					nBytes = fwrite(pBlank, 1, 3, pFile);
				}
				if (nBytes != 3)
				{
					printf_s("Cannot write pixel at x = %d, y = %d.\n", x, y);
					break;
				}
			}
			if (nBytes != 3)
			{
				break;
			}
			else
			{
				sprintf_s(pBuf, "  \n");
				nBytes = fwrite(pBuf, 1, 3, pFile);
			}
		}
	}

	// clean up
	if (pFile != NULL)
	{
		fclose(pFile);
		pFile = NULL;
	}

	return 0;
}
