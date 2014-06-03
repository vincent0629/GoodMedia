#include "FileSource.h"
#include "SourceGraph.h"
#include "DefaultMediaSourceFactory.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#ifdef _WIN32
#include <conio.h>
#else
//#include <curses.h>
#endif

extern void printMediaInfo(MediaType type, MediaInfo *pInfo, int nLevel);  //defined in MediaInfo.cpp

typedef struct
{
	MediaType type;
	const char *pExt;
} TYPEITEM;

TYPEITEM pTypeTable[] = {
	{MEDIA_TYPE_3GPP, ".3GP"},
	{MEDIA_TYPE_AAC, ".AAC/.M4A"},
	{MEDIA_TYPE_AC3, ".AC3"},
	{MEDIA_TYPE_AIFF, ".AIF/.AIFF/.AIFC"},
	{MEDIA_TYPE_AMR, ".AMR"},
	{MEDIA_TYPE_ASF, ".ASF/.WMA/.WMV/.DVR-MS"},
	{MEDIA_TYPE_AU, ".AU/.SND"},
	{MEDIA_TYPE_AVI, ".AVI/.DIVX"},
	{MEDIA_TYPE_BITMAP, ".BMP/.DIB"},
	{MEDIA_TYPE_DAT, ".DAT"},
	{MEDIA_TYPE_DDPLUS, ".EC3"},
	{MEDIA_TYPE_DTS, ".DTS/.DTSHD/.CPT/.CPTL"},
	{MEDIA_TYPE_FLAC, ".FLAC"},
	{MEDIA_TYPE_FLASHVIDEO, ".FLV"},
	{MEDIA_TYPE_H264, ".264/.MPG/.MPEG"},
	{MEDIA_TYPE_MATROSKA, ".MKV/.MKA/.MKS"},
	{MEDIA_TYPE_MIDI, ".MID/.MIDI/.KAR/.SMF"},
	{MEDIA_TYPE_MLP, ".VR/.MLP"},
	{MEDIA_TYPE_MONKEYSAUDIO, ".APE/.MAC"},
	{MEDIA_TYPE_MPEG4, ".MP4/.M4A/.M4P/.M4V/.3GP/.3G2"},
	{MEDIA_TYPE_MPEGAUDIO, ".MP3/.MP2/.MP1/.MPA"},
	{MEDIA_TYPE_MPEGPROGRAM, ".MPG/.MPEG/.VOB/.EVO/.VRO/.M2P"},
	{MEDIA_TYPE_MPEGTRANSPORT, ".MPG/.MPEG/.TRP/.TS/.M2TS/.MTS"},
	{MEDIA_TYPE_MPEGVIDEO, ".M1V/.M2V/.MPG/.MPEG/.MPV"},
	{MEDIA_TYPE_OGG, ".OGG/.OGV/.OGA/.OGX/.OGM"},
	{MEDIA_TYPE_PNG, ".PNG"},
	{MEDIA_TYPE_QUICKTIME, ".MOV/.QT"},
	{MEDIA_TYPE_REALAUDIO, ".RA"},
	{MEDIA_TYPE_REALMEDIA, ".RM/.RMVB"},
	{MEDIA_TYPE_RMID, ".RMI"},
	{MEDIA_TYPE_WAV, ".WAV"}
};

static bool checkFile(const char *path)
{
	const char *pExt;
	char str[256], *ptr;
	int i;

	pExt = strrchr(path, '.');
	if (pExt != NULL)
	{
		strcpy(str, pExt);
		for (ptr = str; *ptr != '\0'; ++ptr)
			*ptr = toupper(*ptr);
		for (i = sizeof(pTypeTable) / sizeof(TYPEITEM) - 1; i >= 0; i--)
			if (strstr(pTypeTable[i].pExt, str) != NULL)
				return true;
	}
	return false;
}

static bool checkType(const char *path, MediaType type)
{
	const char *pExt;
	char str[256], *ptr;
	int i;

	pExt = strrchr(path, '.');
	if (pExt != NULL)
	{
		strcpy(str, pExt);
		for (ptr = str; *ptr != '\0'; ++ptr)
			*ptr = toupper(*ptr);
		for (i = sizeof(pTypeTable) / sizeof(TYPEITEM) - 1; i >= 0; i--)
			if (pTypeTable[i].type == type)
				return strstr(pTypeTable[i].pExt, str) != NULL;
	}
	return false;
}

static void printInfo(CMediaSource *pSource)
{
	char pBuf[256];
	MediaInfo *pInfo;
	int i;
	CMediaSource *pOutputSource;
	static int nLevel = 0;

	pInfo = (MediaInfo *)pBuf;
	pInfo->nSize = sizeof(pBuf);
	printMediaInfo(pSource->GetSourceType(), pSource->GetSourceInfo(pInfo)? pInfo : NULL, nLevel);
	++nLevel;
	for (i = 0; i < pSource->GetOutputNum(); i++)
	{
		pOutputSource = pSource->GetOutputSource(i);
		if (pOutputSource != NULL)
			printInfo(pOutputSource);
		else
		{
			pInfo->nSize = sizeof(pBuf);
			printMediaInfo(pSource->GetOutputType(i), pSource->GetOutputInfo(i, pInfo)? pInfo : NULL, nLevel);
		}
	}
	--nLevel;
}

static MediaType run(const char *path)
{
	CFileSource *pDataSource;
	CSourceGraph *pGraph;
	MediaType type;
	int nDuration;

	pDataSource = new CFileSource();
	pDataSource->Open(path);
	pGraph = new CSourceGraph();
	type = pGraph->Open(pDataSource);
	if (type != MEDIA_TYPE_UNKNOWN)
	{
		printInfo(pGraph->GetSource());
		nDuration = pGraph->GetDuration();
		printf("Duration = %d ms", nDuration);
		nDuration /= 1000;
		printf(" (%02d:%02d:%02d)\n", nDuration / 60 / 60, nDuration / 60 % 60, nDuration % 60);
	}
	delete pGraph;
	delete pDataSource;
	return type;
}

int main(int argc, char *argv[])
{
	char *param;
	FILE *file;
	bool bPause, bCheck;
	MediaType type;
	int i;
	int nResult;
	clock_t tBegin;

	if (argc < 2)
	{
#ifdef _WIN32
		param = strrchr(argv[0], '\\');
#else
		param = strrchr(argv[0], '/');
#endif
		if (param == NULL)
			param = argv[0];
		else
			++param;
		printf("%s [-c] [-p] <file>\n", param);
		printf(" -c: Only parse files with known extension.\n");
		printf(" -p: Pause after parsed.\n");
		return 0;
	}

	param = argv[argc - 1];
	if (param[0] == '"')
	{
		//remove double quote
		++param;
		if (param[strlen(param) - 1] == '"')
			param[strlen(param) - 1] = '\0';
	}

	printf("%s\n", param);
	file = fopen(param, "rb");
	if (file == NULL)
	{
		printf("File not found.\n");
		return 2;
	}
	fclose(file);

	bPause = false;
	bCheck = false;
	for (i = 1; i < argc - 1; i++)
		if (strcasecmp(argv[i], "-p") == 0)
			bPause = true;
		else if (strcasecmp(argv[i], "-c") == 0)
			bCheck = true;

	if (bCheck)
		bCheck = checkFile(param);
	else
		bCheck = true;

	nResult = 0;
	if (!bCheck)
	{
		printf("** warning: file skipped **\n");
		nResult = 1;
	}
	else
	{
		tBegin = clock();
		CDefaultMediaSourceFactory::Register();
		type = run(param);
		if (type == MEDIA_TYPE_UNKNOWN)
		{
			printf("** warning: file parsed fail **\n");
			nResult = 1;
		}
		else if (!checkType(param, type))
		{
			printf("** warning: file type doesn't match file extension **\n");
			nResult = 1;
		}
		printf("\nTime elapse = %ld ms\n", (clock() - tBegin) * 1000 / CLOCKS_PER_SEC);
	}

	if (bPause)
		;//getch();

	return nResult;
}
