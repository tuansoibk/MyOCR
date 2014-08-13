#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <stdlib.h>
#include <string.h>
#include "types.h"

uint32 ByteArrayToInt(uint8 *pBytes, uint8 nBytes);
void IntToByteArray(uint8 **pBytes, uint32 n, uint8 nBytes);

#endif // __UTILITIES_H__
