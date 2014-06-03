static void YCbCrToRGB(unsigned char Y, unsigned char Cb, unsigned char Cr, unsigned char *pRGB)
{
	int i, nRGB[3];

	nRGB[2] = Y + 1.402f * (Cr - 128);
	nRGB[1] = Y - 0.34414f * (Cb - 128) - 0.71414f * (Cr - 128);
	nRGB[0] = Y + 1.772f * (Cb - 128);
	for (i = 0; i < 3; i++)
		if (nRGB[i] < 0)
			pRGB[i] = 0;
		else if (nRGB[i] > 255)
			pRGB[i] = 255;
		else
			pRGB[i] = nRGB[i];
}

void convertPixel(int nWidth, int nHeight, unsigned char *pSrcFrame, unsigned char *pDstFrame)
{
	int i, j;
	unsigned char *pSrcPixel, *pDstPixel;

	for (i = 0; i < nHeight / 2; i++)
		for (j = 0; j < nWidth / 2; j++)
		{
			pSrcPixel = pSrcFrame + i * nWidth * 3 + j * 6;
			pDstPixel = pDstFrame + (i * nWidth + j) * 2 * 3;
			YCbCrToRGB(pSrcPixel[0], pSrcPixel[4], pSrcPixel[5], pDstPixel);
			YCbCrToRGB(pSrcPixel[1], pSrcPixel[4], pSrcPixel[5], pDstPixel + 3);
			YCbCrToRGB(pSrcPixel[2], pSrcPixel[4], pSrcPixel[5], pDstPixel + nWidth * 3);
			YCbCrToRGB(pSrcPixel[3], pSrcPixel[4], pSrcPixel[5], pDstPixel + nWidth * 3 + 3);
		}
}
