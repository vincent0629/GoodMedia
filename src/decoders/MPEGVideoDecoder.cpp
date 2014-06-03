#include "MPEGVideoDecoder.h"
#include "MPEGVideoInfo.h"
#include "bitstream.h"
#include "pixelconvert.h"
#include <string.h>
#include <math.h>

#define PICTURE_START_CODE 0x00
#define SLICE_START_CODE_BEGIN 0x01
#define SLICE_START_CODE_END 0xAF
#define USER_DATA_START_CODE 0xB2
#define SEQUENCE_HEADER_CODE 0xB3
#define EXTENSION_START_CODE 0xB5
#define SEQUENCE_END_CODE 0xB7
#define GROUP_START_CODE 0xB8

#define TREE_BAD -999
#define MACROBLOCK_STUFFING -1
#define MACROBLOCK_ESCAPE -2
#define MACROBLOCK_EXIT -3
#define END_OF_BLOCK -1
#define ESCAPE -2

#define RUNLEVEL(x, y) ((x << 8) | y)

typedef CMPEGVideoDecoder::DECODERPARAM DECODERPARAM;

typedef struct
{
	int nQuant, nMotionForward, nMotionBackward, nPattern, nIntra;
} MACROBLOCK;

static MACROBLOCK g_Macroblocks[21] = {
	{0, 0, 0, 0, 1}, {1, 0, 0, 0, 1},  //I-picture
	{0, 1, 0, 1, 0}, {0, 0, 0, 1, 0}, {0, 1, 0, 0, 0}, {0, 0, 0, 0, 1}, {1, 1, 0, 1, 0}, {1, 0, 0, 1, 0}, {1, 0, 0, 0, 1},  //P-picture
	{0, 1, 1, 0, 0}, {0, 1, 1, 1, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 1, 0}, {0, 1, 0, 0, 0}, {0, 1, 0, 1, 0}, {0, 0, 0, 0, 1}, {1, 1, 1, 1, 0}, {1, 1, 0, 1, 0}, {1, 0, 1, 1, 0}, {1, 0, 0, 0, 1},  //B-picture
	{0, 0, 0, 0, 1}  //D-picture
};
static int g_nScan[64] = {
	 0,  1,  5,  6, 14, 15, 27, 28,
	 2,  4,  7, 13, 16, 26, 29, 42,
	 3,  8, 12, 17, 25, 30, 41, 43,
	 9, 11, 18, 24, 31, 40, 44, 53,
	10, 19, 23, 32, 39, 45, 52, 54,
	20, 22, 33, 38, 46, 51, 55, 60,
	21, 34, 37, 47, 50, 56, 59, 61,
	35, 36, 48, 49, 57, 58, 62, 63
};
static unsigned char g_nIntraQuantizerMatrix[64] = {
	 8, 16, 19, 22, 26, 27, 29, 34,
	16, 16, 22, 24, 27, 29, 34, 37,
	19, 22, 26, 27, 29, 34, 34, 38,
	22, 22, 26, 27, 29, 34, 37, 40,
	22, 26, 27, 29, 32, 35, 40, 48,
	26, 27, 29, 32, 35, 40, 48, 58,
	26, 27, 29, 34, 38, 46, 56, 69,
	27, 29, 35, 38, 46, 56, 69, 83
};

static void initDecoder(DECODERPARAM *pParam)
{
	int i, j;
	double PI16;
	HUFFMANNODE *pRoot;

	pParam->nWidth = 0;
	pParam->nHeight = 0;
	pParam->pBuffer = NULL;
	PI16 = 3.14159265358979 / 16.0;
	for (i = 0; i < 8; i++)
		for (j = 0; j < 8; j++)
			pParam->pCos[i][j] = cos((2 * i + 1) * j * PI16);

	pParam->pMbAddrIncTree = pRoot = huffmanCreate(TREE_BAD);
	huffmanAddPath(pRoot, "1", 1);
	huffmanAddPath(pRoot, "011", 2);
	huffmanAddPath(pRoot, "010", 3);
	huffmanAddPath(pRoot, "0011", 4);
	huffmanAddPath(pRoot, "0010", 5);
	huffmanAddPath(pRoot, "00011", 6);
	huffmanAddPath(pRoot, "00010", 7);
	huffmanAddPath(pRoot, "0000111", 8);
	huffmanAddPath(pRoot, "0000110", 9);
	huffmanAddPath(pRoot, "00001011", 10);
	huffmanAddPath(pRoot, "00001010", 11);
	huffmanAddPath(pRoot, "00001001", 12);
	huffmanAddPath(pRoot, "00001000", 13);
	huffmanAddPath(pRoot, "00000111", 14);
	huffmanAddPath(pRoot, "00000110", 15);
	huffmanAddPath(pRoot, "0000010111", 16);
	huffmanAddPath(pRoot, "0000010110", 17);
	huffmanAddPath(pRoot, "0000010101", 18);
	huffmanAddPath(pRoot, "0000010100", 19);
	huffmanAddPath(pRoot, "0000010011", 20);
	huffmanAddPath(pRoot, "0000010010", 21);
	huffmanAddPath(pRoot, "00000100011", 22);
	huffmanAddPath(pRoot, "00000100010", 23);
	huffmanAddPath(pRoot, "00000100001", 24);
	huffmanAddPath(pRoot, "00000100000", 25);
	huffmanAddPath(pRoot, "00000011111", 26);
	huffmanAddPath(pRoot, "00000011110", 27);
	huffmanAddPath(pRoot, "00000011101", 28);
	huffmanAddPath(pRoot, "00000011100", 29);
	huffmanAddPath(pRoot, "00000011011", 30);
	huffmanAddPath(pRoot, "00000011010", 31);
	huffmanAddPath(pRoot, "00000011001", 32);
	huffmanAddPath(pRoot, "00000011000", 33);
	huffmanAddPath(pRoot, "00000001111", MACROBLOCK_STUFFING);
	huffmanAddPath(pRoot, "00000001000", MACROBLOCK_ESCAPE);
	huffmanAddPath(pRoot, "00000000", MACROBLOCK_EXIT);

	pParam->pMbTypeTree[0] = pRoot = huffmanCreate(TREE_BAD);
	huffmanAddPath(pRoot, "1", 0);
	huffmanAddPath(pRoot, "01", 1);
	pParam->pMbTypeTree[1] = pRoot = huffmanCreate(TREE_BAD);
	huffmanAddPath(pRoot, "1", 2);
	huffmanAddPath(pRoot, "01", 3);
	huffmanAddPath(pRoot, "001", 4);
	huffmanAddPath(pRoot, "00011", 5);
	huffmanAddPath(pRoot, "00010", 6);
	huffmanAddPath(pRoot, "00001", 7);
	huffmanAddPath(pRoot, "000001", 8);
	pParam->pMbTypeTree[2] = pRoot = huffmanCreate(TREE_BAD);
	huffmanAddPath(pRoot, "10", 9);
	huffmanAddPath(pRoot, "11", 10);
	huffmanAddPath(pRoot, "010", 11);
	huffmanAddPath(pRoot, "011", 12);
	huffmanAddPath(pRoot, "0010", 13);
	huffmanAddPath(pRoot, "0011", 14);
	huffmanAddPath(pRoot, "00011", 15);
	huffmanAddPath(pRoot, "00010", 16);
	huffmanAddPath(pRoot, "000011", 17);
	huffmanAddPath(pRoot, "000010", 18);
	huffmanAddPath(pRoot, "000001", 19);
	pParam->pMbTypeTree[3] = pRoot = huffmanCreate(TREE_BAD);
	huffmanAddPath(pRoot, "1", 20);

	pParam->pDCSizeLuminanceTree = pRoot = huffmanCreate(TREE_BAD);
	huffmanAddPath(pRoot, "100", 0);
	huffmanAddPath(pRoot, "00", 1);
	huffmanAddPath(pRoot, "01", 2);
	huffmanAddPath(pRoot, "101", 3);
	huffmanAddPath(pRoot, "110", 4);
	huffmanAddPath(pRoot, "1110", 5);
	huffmanAddPath(pRoot, "11110", 6);
	huffmanAddPath(pRoot, "111110", 7);
	huffmanAddPath(pRoot, "1111110", 8);

	pParam->pDCSizeChrominanceTree = pRoot = huffmanCreate(TREE_BAD);
	huffmanAddPath(pRoot, "00", 0);
	huffmanAddPath(pRoot, "01", 1);
	huffmanAddPath(pRoot, "10", 2);
	huffmanAddPath(pRoot, "110", 3);
	huffmanAddPath(pRoot, "1110", 4);
	huffmanAddPath(pRoot, "11110", 5);
	huffmanAddPath(pRoot, "111110", 6);
	huffmanAddPath(pRoot, "1111110", 7);
	huffmanAddPath(pRoot, "11111110", 8);

	pParam->pDCTCoeffTree = pRoot = huffmanCreate(TREE_BAD);
	huffmanAddPath(pRoot, "10", END_OF_BLOCK);
	huffmanAddPath(pRoot, "11", RUNLEVEL(0, 1));
	huffmanAddPath(pRoot, "011", RUNLEVEL(1, 1));
	huffmanAddPath(pRoot, "0100", RUNLEVEL(0, 2));
	huffmanAddPath(pRoot, "0101", RUNLEVEL(2, 1));
	huffmanAddPath(pRoot, "00101", RUNLEVEL(0, 3));
	huffmanAddPath(pRoot, "00111", RUNLEVEL(3, 1));
	huffmanAddPath(pRoot, "00110", RUNLEVEL(4, 1));
	huffmanAddPath(pRoot, "000110", RUNLEVEL(1, 2));
	huffmanAddPath(pRoot, "000111", RUNLEVEL(5, 1));
	huffmanAddPath(pRoot, "000101", RUNLEVEL(6, 1));
	huffmanAddPath(pRoot, "000100", RUNLEVEL(7, 1));
	huffmanAddPath(pRoot, "0000110", RUNLEVEL(0, 4));
	huffmanAddPath(pRoot, "0000100", RUNLEVEL(2, 2));
	huffmanAddPath(pRoot, "0000111", RUNLEVEL(8, 1));
	huffmanAddPath(pRoot, "0000101", RUNLEVEL(9, 1));
	huffmanAddPath(pRoot, "000001", ESCAPE);
	huffmanAddPath(pRoot, "00100110", RUNLEVEL(0, 5));
	huffmanAddPath(pRoot, "00100001", RUNLEVEL(0, 6));
	huffmanAddPath(pRoot, "00100101", RUNLEVEL(1, 3));
	huffmanAddPath(pRoot, "00100100", RUNLEVEL(3, 2));
	huffmanAddPath(pRoot, "00100111", RUNLEVEL(10, 1));
	huffmanAddPath(pRoot, "00100011", RUNLEVEL(11, 1));
	huffmanAddPath(pRoot, "00100010", RUNLEVEL(12, 1));
	huffmanAddPath(pRoot, "00100000", RUNLEVEL(13, 1));
	huffmanAddPath(pRoot, "0000001010", RUNLEVEL(0, 7));
	huffmanAddPath(pRoot, "0000001100", RUNLEVEL(1, 4));
	huffmanAddPath(pRoot, "0000001011", RUNLEVEL(2, 3));
	huffmanAddPath(pRoot, "0000001111", RUNLEVEL(4, 2));
	huffmanAddPath(pRoot, "0000001001", RUNLEVEL(5, 2));
	huffmanAddPath(pRoot, "0000001110", RUNLEVEL(14, 1));
	huffmanAddPath(pRoot, "0000001101", RUNLEVEL(15, 1));
	huffmanAddPath(pRoot, "0000001000", RUNLEVEL(16, 1));
	huffmanAddPath(pRoot, "000000011101", RUNLEVEL(0, 8));
	huffmanAddPath(pRoot, "000000011000", RUNLEVEL(0, 9));
	huffmanAddPath(pRoot, "000000010011", RUNLEVEL(0, 10));
	huffmanAddPath(pRoot, "000000010000", RUNLEVEL(0, 11));
	huffmanAddPath(pRoot, "000000011011", RUNLEVEL(1, 5));
	huffmanAddPath(pRoot, "000000010100", RUNLEVEL(2, 4));
	huffmanAddPath(pRoot, "000000011100", RUNLEVEL(3, 3));
	huffmanAddPath(pRoot, "000000010010", RUNLEVEL(4, 3));
	huffmanAddPath(pRoot, "000000011110", RUNLEVEL(6, 2));
	huffmanAddPath(pRoot, "000000010101", RUNLEVEL(7, 2));
	huffmanAddPath(pRoot, "000000010001", RUNLEVEL(8, 2));
	huffmanAddPath(pRoot, "000000011111", RUNLEVEL(17, 1));
	huffmanAddPath(pRoot, "000000011010", RUNLEVEL(18, 1));
	huffmanAddPath(pRoot, "000000011001", RUNLEVEL(19, 1));
	huffmanAddPath(pRoot, "000000010111", RUNLEVEL(20, 1));
	huffmanAddPath(pRoot, "000000010110", RUNLEVEL(21, 1));
	huffmanAddPath(pRoot, "0000000011010", RUNLEVEL(0, 12));
	huffmanAddPath(pRoot, "0000000011001", RUNLEVEL(0, 13));
	huffmanAddPath(pRoot, "0000000011000", RUNLEVEL(0, 14));
	huffmanAddPath(pRoot, "0000000010111", RUNLEVEL(0, 15));
	huffmanAddPath(pRoot, "0000000010110", RUNLEVEL(1, 6));
	huffmanAddPath(pRoot, "0000000010101", RUNLEVEL(1, 7));
	huffmanAddPath(pRoot, "0000000010100", RUNLEVEL(2, 5));
	huffmanAddPath(pRoot, "0000000010011", RUNLEVEL(3, 4));
	huffmanAddPath(pRoot, "0000000010010", RUNLEVEL(5, 3));
	huffmanAddPath(pRoot, "0000000010001", RUNLEVEL(9, 2));
	huffmanAddPath(pRoot, "0000000010000", RUNLEVEL(10, 2));
	huffmanAddPath(pRoot, "0000000011111", RUNLEVEL(22, 1));
	huffmanAddPath(pRoot, "0000000011110", RUNLEVEL(23, 1));
	huffmanAddPath(pRoot, "0000000011101", RUNLEVEL(24, 1));
	huffmanAddPath(pRoot, "0000000011100", RUNLEVEL(25, 1));
	huffmanAddPath(pRoot, "0000000011011", RUNLEVEL(26, 1));
	huffmanAddPath(pRoot, "00000000011111", RUNLEVEL(0, 16));
	huffmanAddPath(pRoot, "00000000011110", RUNLEVEL(0, 17));
	huffmanAddPath(pRoot, "00000000011101", RUNLEVEL(0, 18));
	huffmanAddPath(pRoot, "00000000011100", RUNLEVEL(0, 19));
	huffmanAddPath(pRoot, "00000000011011", RUNLEVEL(0, 20));
	huffmanAddPath(pRoot, "00000000011010", RUNLEVEL(0, 21));
	huffmanAddPath(pRoot, "00000000011001", RUNLEVEL(0, 22));
	huffmanAddPath(pRoot, "00000000011000", RUNLEVEL(0, 23));
	huffmanAddPath(pRoot, "00000000010111", RUNLEVEL(0, 24));
	huffmanAddPath(pRoot, "00000000010110", RUNLEVEL(0, 25));
	huffmanAddPath(pRoot, "00000000010101", RUNLEVEL(0, 26));
	huffmanAddPath(pRoot, "00000000010100", RUNLEVEL(0, 27));
	huffmanAddPath(pRoot, "00000000010011", RUNLEVEL(0, 28));
	huffmanAddPath(pRoot, "00000000010010", RUNLEVEL(0, 29));
	huffmanAddPath(pRoot, "00000000010001", RUNLEVEL(0, 30));
	huffmanAddPath(pRoot, "00000000010000", RUNLEVEL(0, 31));
	huffmanAddPath(pRoot, "000000000011000", RUNLEVEL(0, 32));
	huffmanAddPath(pRoot, "000000000010111", RUNLEVEL(0, 33));
	huffmanAddPath(pRoot, "000000000010110", RUNLEVEL(0, 34));
	huffmanAddPath(pRoot, "000000000010101", RUNLEVEL(0, 35));
	huffmanAddPath(pRoot, "000000000010100", RUNLEVEL(0, 36));
	huffmanAddPath(pRoot, "000000000010011", RUNLEVEL(0, 37));
	huffmanAddPath(pRoot, "000000000010010", RUNLEVEL(0, 38));
	huffmanAddPath(pRoot, "000000000010001", RUNLEVEL(0, 39));
	huffmanAddPath(pRoot, "000000000010000", RUNLEVEL(0, 40));
	huffmanAddPath(pRoot, "000000000011111", RUNLEVEL(1, 8));
	huffmanAddPath(pRoot, "000000000011110", RUNLEVEL(1, 9));
	huffmanAddPath(pRoot, "000000000011101", RUNLEVEL(1, 10));
	huffmanAddPath(pRoot, "000000000011100", RUNLEVEL(1, 11));
	huffmanAddPath(pRoot, "000000000011011", RUNLEVEL(1, 12));
	huffmanAddPath(pRoot, "000000000011010", RUNLEVEL(1, 13));
	huffmanAddPath(pRoot, "000000000011001", RUNLEVEL(1, 14));
	huffmanAddPath(pRoot, "0000000000010011", RUNLEVEL(1, 15));
	huffmanAddPath(pRoot, "0000000000010010", RUNLEVEL(1, 16));
	huffmanAddPath(pRoot, "0000000000010001", RUNLEVEL(1, 17));
	huffmanAddPath(pRoot, "0000000000010000", RUNLEVEL(1, 18));
	huffmanAddPath(pRoot, "0000000000010100", RUNLEVEL(6, 3));
	huffmanAddPath(pRoot, "0000000000011010", RUNLEVEL(11, 2));
	huffmanAddPath(pRoot, "0000000000011001", RUNLEVEL(12, 2));
	huffmanAddPath(pRoot, "0000000000011000", RUNLEVEL(13, 2));
	huffmanAddPath(pRoot, "0000000000010111", RUNLEVEL(14, 2));
	huffmanAddPath(pRoot, "0000000000010110", RUNLEVEL(15, 2));
	huffmanAddPath(pRoot, "0000000000010101", RUNLEVEL(16, 2));
	huffmanAddPath(pRoot, "0000000000011111", RUNLEVEL(27, 1));
	huffmanAddPath(pRoot, "0000000000011110", RUNLEVEL(28, 1));
	huffmanAddPath(pRoot, "0000000000011101", RUNLEVEL(29, 1));
	huffmanAddPath(pRoot, "0000000000011100", RUNLEVEL(30, 1));
	huffmanAddPath(pRoot, "0000000000011011", RUNLEVEL(31, 1));
}

static void uninitDecoder(DECODERPARAM *pParam)
{
	delete[] pParam->pBuffer;
	pParam->pBuffer = NULL;
	huffmanDestroy(pParam->pMbAddrIncTree);
	huffmanDestroy(pParam->pMbTypeTree[0]);
	huffmanDestroy(pParam->pMbTypeTree[1]);
	huffmanDestroy(pParam->pMbTypeTree[2]);
	huffmanDestroy(pParam->pMbTypeTree[3]);
	huffmanDestroy(pParam->pDCSizeLuminanceTree);
	huffmanDestroy(pParam->pDCSizeChrominanceTree);
	huffmanDestroy(pParam->pDCTCoeffTree);
}

static int nextStartCode(BITSTREAM *pStream, int nBits)
{
	long nCode;
	int n;

	bsalign(pStream);
	nCode = bsread(pStream, nBits - 8);
	while (nCode != 0x000001)
	{
		n = bsread(pStream, 8);
		if (n == -1)
			return -1;
		nCode = ((nCode & 0xFFFF) << 8) | n;
	}
	return bsread(pStream, 8);
}

static int lookupHuffman(BITSTREAM *pStream, HUFFMANNODE *pRoot)
{
	int nBadValue;
	HUFFMANNODE *pNode;
	int nRead;
#ifdef _DEBUG
	int nValue = 0, nCount = 0;
#endif

	nBadValue = huffmanGetValue(pRoot);
	pNode = pRoot;
	do
	{
		nRead = bsread(pStream, 1);
		if (nRead == -1)  //end of stream
			return nBadValue;

#ifdef _DEBUG
		++nCount;
		nValue = (nValue << 1) | nRead;
#endif
		pNode = huffmanTraverse(pNode, nRead);
		if (pNode == NULL)  //should not happen
			return nBadValue;
	} while (huffmanGetValue(pNode) == nBadValue);
	return huffmanGetValue(pNode);
}

static int sign(int n)
{
	if (n > 0)
		return 1;
	if (n < 0)
		return -1;
	return 0;
}

static void idct(DECODERPARAM *pParam, int *matrix, int nWidth, int nHeight)
{
	int i, j, m, n;
	int *pm;
	double d[64], *pd, d1, d2, dd;

	dd = sqrt(4.0 / (nWidth * nHeight));
	pd = d;
	for (i = 0; i < nHeight; i++)
		for (j = 0; j < nWidth; j++)
		{
			*pd = 0.0;
			pm = matrix;
			for (m = 0; m < nHeight; m++)
			{
				d1 = m == 0? 0.707106781 : 1.0;  //1/sqrt(2)
				for (n = 0; n < nWidth; n++)
				{
					d2 = n == 0? 0.707106781 : 1.0;  //1/sqrt(2)
					*pd += d1 * d2 * (*pm++) * pParam->pCos[i][m] * pParam->pCos[j][n];
				}
			}
			*pd++ *= dd;
		}
	for (i = nWidth * nHeight - 1; i >= 0; i--)
		matrix[i] = (int)floor(d[i] + 0.5);
}

static bool decode1(BITSTREAM *pStream, DECODERPARAM *pParam, unsigned char *pFrame)
{
	int i, j;
	bool bResult;
	long nCode;
	int nPictureCodingType;
	int nQuantizerScale;
	int nMbAddrInc, nMbType;
	MACROBLOCK *pMacroblock;
	int nDCSize, nDCDifferential, nDCTCoeffNext;
	int nRun, nLevel;
	int dct_zz[64], dct_recon[6][64], *pRecon;
	unsigned char *pBlock, *pPixel;
	int nPastY, nPastCb, nPastCr;
	int *pY, *pCb, *pCr;
	int nBlockOfSlice;

	bResult = false;
	nCode = nextStartCode(pStream, 32);
	while (nCode != -1)
	{
		if (nCode == SEQUENCE_HEADER_CODE)
		{
			bsseek(pStream, 62);
			if (bsread(pStream, 1))  //load intra quantizer matrix
				for (i = 0; i < 64; i++)
					pParam->pIntraQuantizerMatrix[i] = bsread(pStream, 8);
			else
				memcpy(pParam->pIntraQuantizerMatrix, g_nIntraQuantizerMatrix, sizeof(pParam->pIntraQuantizerMatrix));
			if (bsread(pStream, 1))  //load non intra quantizer matrix
				for (i = 0; i < 64; i++)
					pParam->pNonIntraQuantizerMatrix[i] = bsread(pStream, 8);
			else
				memset(pParam->pNonIntraQuantizerMatrix, 16, sizeof(pParam->pNonIntraQuantizerMatrix));
			nCode = nextStartCode(pStream, 32);
		}
		else if (nCode == PICTURE_START_CODE)
		{
			bsseek(pStream, 10);  //temporal reference
			nPictureCodingType = bsread(pStream, 3);  //picture coding type
			do
			{
				nCode = nextStartCode(pStream, 32);
				if (nCode == -1)
					return bResult;
			} while (nCode == USER_DATA_START_CODE || nCode == EXTENSION_START_CODE);
			if (nPictureCodingType == 1)  //I-picture
			{
				while (nCode >= SLICE_START_CODE_BEGIN && nCode <= SLICE_START_CODE_END)
				{
					nQuantizerScale = bsread(pStream, 5);  //quantizer scale
					while (bsread(pStream, 1) == 1)  //extra bit slice
						bsseek(pStream, 8);  //extra information slice

					pBlock = pFrame + (nCode - 1) * pParam->nWidth * 16 * 6 / 4;
					nPastY = nPastCb = nPastCr = 128 * 8;
					nBlockOfSlice = 0;
					//macroblock
					for (;;)
					{
						do
						{
							nMbAddrInc = lookupHuffman(pStream, pParam->pMbAddrIncTree);  //macroblock address increment
							if (nMbAddrInc == TREE_BAD)
								return bResult;
						} while (nMbAddrInc == MACROBLOCK_STUFFING);
						if (nMbAddrInc == MACROBLOCK_EXIT)
							break;
						nMbType = lookupHuffman(pStream, pParam->pMbTypeTree[nPictureCodingType - 1]);  //macroblock type
						if (nMbType == TREE_BAD)
							return bResult;

						pMacroblock = g_Macroblocks + nMbType;
						if (pMacroblock->nQuant)
							nQuantizerScale = bsread(pStream, 5);  //quantizer scale
						if (pMacroblock->nMotionForward)
						{
							//motion horizontal forward code
							//motion vertical forward code
						}
						if (pMacroblock->nMotionBackward)
						{
							//motion horizontal backward code
							//motion vertical backward code
						}
						if (pMacroblock->nPattern)
						{
							//coded block pattern
						}
						for (i = 0; i < 6; i++)
						{
							memset(dct_zz, 0, sizeof(dct_zz));
							if (pMacroblock->nIntra)
							{
								nDCSize = lookupHuffman(pStream, i < 4? pParam->pDCSizeLuminanceTree : pParam->pDCSizeChrominanceTree);  //dct dc size luminance
								if (nDCSize == TREE_BAD)
									return bResult;
								if (nDCSize != 0)
								{
									nDCDifferential = bsread(pStream, nDCSize);  //dct dc differential
									if (nDCDifferential & (1 << (nDCSize - 1)))
										dct_zz[0] = nDCDifferential;
									else
										dct_zz[0] = (-1 << nDCSize) | (nDCDifferential + 1);
								}
								if (nPictureCodingType != 4)
								{
									j = 0;
									for (;;)
									{
										nDCTCoeffNext = lookupHuffman(pStream, pParam->pDCTCoeffTree);
										if (nDCTCoeffNext == TREE_BAD)
											return bResult;
										if (nDCTCoeffNext == END_OF_BLOCK)  //end of block
											break;
										if (nDCTCoeffNext == ESCAPE)  //escape
										{
											nRun = bsread(pStream, 6);
											nLevel = bsread(pStream, 8);
											if (nLevel == 0x80)
												nLevel = bsread(pStream, 8) - 256;  //-256~-128
											else if (nLevel == 0x00)
												nLevel = bsread(pStream, 8);  //128~255
											else if (nLevel > 0x80)
												nLevel -= 256;  //-127~127
										}
										else
										{
											nRun = nDCTCoeffNext >> 8;
											nLevel = nDCTCoeffNext & 0xFF;
											if (bsread(pStream, 1) == 1)
												nLevel = -nLevel;
										}
										j += nRun + 1;
										dct_zz[j] = nLevel;
									}  //for
								}  //if

								pRecon = dct_recon[i];
								for (j = 1; j < 64; j++)
								{
									pRecon[j] = (dct_zz[g_nScan[j]] * nQuantizerScale * pParam->pIntraQuantizerMatrix[j]) >> 3;
									if ((pRecon[j] & 1) == 0)
										pRecon[j] -= sign(pRecon[j]);
									if (pRecon[j] > 2047)
										pRecon[j] = 2047;
									else if (pRecon[j] < -2048)
										pRecon[j] = -2048;
								}
								pRecon[0] = dct_zz[0] * 8;
								if (i < 4)  //Y
								{
									pRecon[0] += nPastY;
									nPastY = pRecon[0];
								}
								else if (i == 4)  //Cb
								{
									pRecon[0] += nPastCb;
									nPastCb = pRecon[0];
								}
								else  //Cr
								{
									pRecon[0] += nPastCr;
									nPastCr = pRecon[0];
								}

								idct(pParam, pRecon, 8, 8);
								for (j = 0; j < 64; j++)
								{
									if (pRecon[j] > 255)
										pRecon[j] = 255;
									else if (pRecon[j] < 0)
										pRecon[j] = 0;
								}
							}  //if
							else
							{
								//dct coeff first
							}
						}  //for
						if (nPictureCodingType == 4)
							bsseek(pStream, 1);  //end of macroblock

						//merge 6 blocks into a YV12 macroblock
						for (i = 0; i < 4; i++)
						{
							pPixel = pBlock + i * pParam->nWidth * 3;
							pY = dct_recon[0] + i * 16;
							pCb = dct_recon[4] + i * 8;
							pCr = dct_recon[5] + i * 8;
							for (j = 0; j < 4; j++)
							{
								pPixel[0] = pY[j * 2];  //Y
								pPixel[1] = pY[j * 2 + 1];  //Y
								pPixel[2] = pY[j * 2 + 8];  //Y
								pPixel[3] = pY[j * 2 + 9];  //Y
								pPixel[4] = pCb[j];  //U
								pPixel[5] = pCr[j];  //V
								pPixel += 6;
							}
							pY = dct_recon[1] + i * 16;
							for (j = 4; j < 8; j++)
							{
								pPixel[0] = pY[(j - 4) * 2];  //Y
								pPixel[1] = pY[(j - 4) * 2 + 1];  //Y
								pPixel[2] = pY[(j - 4) * 2 + 8];  //Y
								pPixel[3] = pY[(j - 4) * 2 + 9];  //Y
								pPixel[4] = pCb[j];  //U
								pPixel[5] = pCr[j];  //V
								pPixel += 6;
							}
						}
						for (i = 4; i < 8; i++)
						{
							pPixel = pBlock + i * pParam->nWidth * 3;
							pY = dct_recon[2] + (i - 4) * 16;
							pCb = dct_recon[4] + i * 8;
							pCr = dct_recon[5] + i * 8;
							for (j = 0; j < 4; j++)
							{
								pPixel[0] = pY[j * 2];  //Y
								pPixel[1] = pY[j * 2 + 1];  //Y
								pPixel[2] = pY[j * 2 + 8];  //Y
								pPixel[3] = pY[j * 2 + 9];  //Y
								pPixel[4] = pCb[j];  //U
								pPixel[5] = pCr[j];  //V
								pPixel += 6;
							}
							pY = dct_recon[3] + (i - 4) * 16;
							for (j = 4; j < 8; j++)
							{
								pPixel[0] = pY[(j - 4) * 2];  //Y
								pPixel[1] = pY[(j - 4) * 2 + 1];  //Y
								pPixel[2] = pY[(j - 4) * 2 + 8];  //Y
								pPixel[3] = pY[(j - 4) * 2 + 9];  //Y
								pPixel[4] = pCb[j];  //U
								pPixel[5] = pCr[j];  //V
								pPixel += 6;
							}
						}

						pBlock += 48;
						if (++nBlockOfSlice * 16 >= pParam->nWidth)
						{
							nBlockOfSlice = 0;
							pBlock += pParam->nWidth * 3 * 7;
						}

						bResult = true;
					}  //for

					nCode = nextStartCode(pStream, 24);
				}  //while
			}  //if
		}
		else
			nCode = nextStartCode(pStream, 32);
	}  //for

	return bResult;
}

CMPEGVideoDecoder::CMPEGVideoDecoder()
{
	m_pQueue = queueCreate(32 * 1024);
	initDecoder(&m_Param);
}

CMPEGVideoDecoder::~CMPEGVideoDecoder()
{
	queueDestroy(m_pQueue);
	uninitDecoder(&m_Param);
}

bool CMPEGVideoDecoder::SetInputInfo(MediaType type, MediaInfo *pInfo)
{
	MPEGVideoInfo *pMPEGVideoInfo;

	if (pInfo->nSize < sizeof(MPEGVideoInfo))
		return false;

	pMPEGVideoInfo = (MPEGVideoInfo *)pInfo;
	//if (pMPEGVideoInfo->nVersion != 1)  //not support
	//	return false;

	m_Param.nWidth = pMPEGVideoInfo->nWidth;
	m_Param.nHeight = pMPEGVideoInfo->nHeight;
	m_fPelAspectRatio = pMPEGVideoInfo->fPelAspectRatio;
	delete[] m_Param.pBuffer;
	m_Param.pBuffer = NULL;
	return true;
}

bool CMPEGVideoDecoder::GetOutputInfo(MediaInfo *pInfo)
{
	VideoRenderInfo *pVideoInfo;

	if (pInfo->nSize < sizeof(VideoRenderInfo))
		return false;

	pVideoInfo = (VideoRenderInfo *)pInfo;
	pVideoInfo->nSize = sizeof(VideoRenderInfo);
	pVideoInfo->nWidth = m_Param.nWidth;
	pVideoInfo->nHeight = m_Param.nHeight;
	pVideoInfo->fPixelAspectRatio = m_fPelAspectRatio;
	pVideoInfo->nPixelFormat = PIXEL_RGB24;
	return true;
}

int CMPEGVideoDecoder::GetInputSize(void)
{
	return m_Param.nWidth * m_Param.nHeight;  //is it large enough for a picture?
}

int CMPEGVideoDecoder::GetOutputSize(void)
{
	return m_Param.nWidth * m_Param.nHeight * 3;
}

int CMPEGVideoDecoder::Decode(unsigned char *pInData, int nInSize, void *pOutData)
{
	BITSTREAM *pStream;
	bool bResult;

	queuePush(m_pQueue, pInData, nInSize);
	pStream = bsopen(ReadByte, m_pQueue);
	if (m_Param.pBuffer == NULL)
		m_Param.pBuffer = new unsigned char[m_Param.nWidth * ((m_Param.nHeight + 15) & ~0x0F) * 6 / 4];  //height may be not multiple of 16
	bResult = decode1(pStream, &m_Param, m_Param.pBuffer);
	bsclose(pStream);
	if (bResult)
	{
		convertPixel(m_Param.nWidth, m_Param.nHeight, m_Param.pBuffer, (unsigned char *)pOutData);
		return m_Param.nWidth * m_Param.nHeight * 3;
	}
	else
		return 0;
}

void CMPEGVideoDecoder::Flush(void)
{
}

int CMPEGVideoDecoder::ReadByte(void *pData)
{
	DATAQUEUE *pQueue;
	int n;

	pQueue = (DATAQUEUE *)pData;
	return queuePop(pQueue, &n, 1) == 0? -1 : n;
}
