#ifndef _GOODPLAYER_H_
#define _GOODPLAYER_H_

#include "PlayerListener.h"

class CGoodPlayer : public IPlayerListener
{
	void VideoSizeChanged(void *pObj, int nWidth, int nHeight);
	void PlayCompleted(void *pObj);
};

#endif
