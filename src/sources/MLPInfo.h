#ifndef _MLPINFO_H_
#define _MLPINFO_H_

#include "MediaInfo.h"

enum MLPStream {MLP_STREAM_FBA, MLP_STREAM_FBB};

typedef struct _MLPInfo : AudioInfo
{
	MLPStream nStream;
} MLPInfo;

#endif
