#ifndef _STREAMPOS_H_
#define _STREAMPOS_H_

typedef struct
{
	size_t nHeadPos;
	size_t nAbsPos;
	size_t nRelPos;
	size_t nSize;
	int nTime;
} StreamPos;

#endif
