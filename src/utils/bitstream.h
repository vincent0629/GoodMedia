#ifndef _BITSTREAM_H_
#define _BITSTREAM_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*BSFUNC)(void *pData);
struct _BITSTREAM;
typedef struct _BITSTREAM BITSTREAM;

BITSTREAM *bsopen(BSFUNC pFunc, void *pData);
int bsread(BITSTREAM *pStream, int nBits);
void bsseek(BITSTREAM *pStream, int nBits);
void bsalign(BITSTREAM *pStream);
void bsclose(BITSTREAM *pStream);

#ifdef __cplusplus
}
#endif

#endif
