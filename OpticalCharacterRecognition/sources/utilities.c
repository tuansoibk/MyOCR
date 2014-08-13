#include "utilities.h"

uint32 ByteArrayToInt(uint8 *pBytes, uint8 nBytes)
{
	uint32 result = 0;

	for (int i = nBytes - 1; i >= 0; i--)
	{
		result <<= 8;
		result |= pBytes[i];
	}

	return result;
}

void IntToByteArray(uint8 **pBytes, uint32 n, uint8 nBytes)
{
	if (*pBytes == NULL)
	{
		*pBytes = (uint8*)calloc(nBytes, 1);
	}
	else
	{
		memset(*pBytes, 0, nBytes);
	}
	for (int i = 0; i < nBytes; i++)
	{
		(*pBytes)[i] = n & 0xFF;
		n >>= 8;
	}
}
