#ifndef _PROGRAMASSOCIATIONINFO_H_
#define _PROGRAMASSOCIATIONINFO_H_

#include "MediaInfo.h"

typedef struct _ProgramInfo
{
	int nProgramNumber;
	int nPID;
} ProgramInfo;

typedef struct _ProgramAssociationInfo : MediaInfo
{
	int nVersion;
	int nProgramNum;
	ProgramInfo *pInfo;
} ProgramAssociationInfo;

#endif
