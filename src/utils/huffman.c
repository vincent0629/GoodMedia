#include "huffman.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct _HUFFMANNODE
{
	struct _HUFFMANNODE *pLeft, *pRight;
	int nValue;
};

HUFFMANNODE *newNode(int nValue)
{
	HUFFMANNODE *pNode;

	pNode = (HUFFMANNODE *)malloc(sizeof(HUFFMANNODE));
	pNode->pLeft = NULL;
	pNode->pRight = NULL;
	pNode->nValue = nValue;
	return pNode;
}

HUFFMANNODE *huffmanCreate(int nBadValue)
{
	return newNode(nBadValue);
}

void huffmanAddPath(HUFFMANNODE *pRoot, const char *path, int nValue)
{
	HUFFMANNODE *pNode;

	pNode = pRoot;
	while (*path != '\0')
	{
		if (*path == '0')
		{
			if (pNode->pLeft == NULL)
				pNode->pLeft = newNode(pNode->nValue);
			pNode = pNode->pLeft;
		}
		else
		{
			if (pNode->pRight == NULL)
				pNode->pRight = newNode(pNode->nValue);
			pNode = pNode->pRight;
		}
		++path;
	}
	assert(pNode->nValue == pRoot->nValue);
	pNode->nValue = nValue;
}

HUFFMANNODE *huffmanTraverse(HUFFMANNODE *pNode, int nBit)
{
	return nBit == 0? pNode->pLeft : pNode->pRight;
}

int huffmanGetValue(HUFFMANNODE *pNode)
{
	return pNode->nValue;
}

void huffmanDestroy(HUFFMANNODE *pRoot)
{
	if (pRoot->pLeft != NULL)
		huffmanDestroy(pRoot->pLeft);
	if (pRoot->pRight != NULL)
		huffmanDestroy(pRoot->pRight);
	free(pRoot);
}
