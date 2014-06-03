#ifndef _PIXELCONVERT_H_
#define _PIXELCONVERT_H_

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

EXTERN void convertPixel(int nWidth, int nHeight, unsigned char *pSrcFrame, unsigned char *pDstFrame);

#undef EXTERN

#endif
