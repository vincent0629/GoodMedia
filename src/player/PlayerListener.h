#ifndef _PLAYERLISTENER_H_
#define _PLAYERLISTENER_H_

class IPlayerListener
{
public:
	virtual void VideoSizeChanged(void *pObj, int nWidth, int nHeight) = 0;
	virtual void PlayCompleted(void *pObj) = 0;
};

#endif
