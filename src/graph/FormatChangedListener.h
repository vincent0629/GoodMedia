#ifndef _FORMATCHANGEDLISTENER_H_
#define _FORMATCHANGEDLISTENER_H_

#include "MediaSource.h"

class IFormatChangedListener
{
public:
	virtual void FormatChanged(void *pObj, CMediaSource *pSource, int nIndex) = 0;
};

#endif
