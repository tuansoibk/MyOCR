#ifndef __BMP_H__
#define __BMP_H__

#include "common.h"
#include "types.h"
#include "utilities.h"

#define WINDOW_BITMAP_TYPE		0x4D42	// "BM"
#define SUPPORTED_OFFSET		54		// 14 bytes file header + 40 bytes info header
#define SUPPORTED_COLOR_PLANE	1
#define SUPPORTED_BIT_COUNT		24
#define NO_COMPRESSION			0

typedef struct {
	char bfType[2];
	unsigned char bfSize[4];
	unsigned char bfReserved1[2];
	unsigned char bfReserved2[2];
	unsigned char bfOffBits[4];
} bmfh;

#pragma pack(push, 2)
typedef struct tagBITMAPFILEHEADER {
  uint16 bfType;
  uint32 bfSize;
  uint16 bfReserved1;
  uint16 bfReserved2;
  uint32 bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;
#pragma pack(pop)

#define BITMAPFILEHEADER_SIZE sizeof(BITMAPFILEHEADER)

typedef struct {
	unsigned char biSize[4];
	unsigned char biWidth[4];
	unsigned char biHeight[4];
	unsigned char biPlanes[2];
	unsigned char biBitCount[2];
	unsigned char stuff1[16];
	unsigned char biClrUsed[4];
	unsigned char biClrImportant[4];
} bmih;

#pragma pack(push, 2)
typedef struct tagBITMAPINFOHEADER {
  sint32 biSize;
  sint32 biWidth;
  sint32 biHeight;
  uint16 biPlanes;
  uint16 biBitCount;
  uint32 biCompression;
  uint32 biSizeImage;
  sint32 biXPelsPerMeter;
  sint32 biYPelsPerMeter;
  uint32 biClrUsed;
  uint32 biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#pragma pack(pop)

#define BITMAPINFOHEADER_SIZE sizeof(BITMAPINFOHEADER)

uint32** ReadBmp(char *fileName, sint32 *height, sint32 *width);
int WriteBmp(char *fileName, uint32 **pPixels, sint32 height, sint32 width);
int WriteImageToText(char *fileName, Image img);

#endif // __BMP_H__
