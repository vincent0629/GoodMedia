#ifndef _HUFFMAN_H_
#define _HUFFMAN_H_

#ifdef __cplusplus
extern "C" {
#endif

struct _HUFFMANNODE;
typedef struct _HUFFMANNODE HUFFMANNODE;

HUFFMANNODE *huffmanCreate(int nBadValue);
void huffmanAddPath(HUFFMANNODE *pRoot, const char *path, int nValue);
HUFFMANNODE *huffmanTraverse(HUFFMANNODE *pNode, int nBit);
int huffmanGetValue(HUFFMANNODE *pNode);
void huffmanDestroy(HUFFMANNODE *pRoot);

#ifdef __cplusplus
}
#endif

#endif
