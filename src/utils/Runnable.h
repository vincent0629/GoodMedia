#ifndef _RUNNABLE_H_
#define _RUNNABLE_H_

class IRunnable
{
public:
	virtual void Run(void *pData) = 0;
};

#endif
