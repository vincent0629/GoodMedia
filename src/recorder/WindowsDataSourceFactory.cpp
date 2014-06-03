#include "WindowsDataSourceFactory.h"
#include "DataSourceFactory.h"
#include "WaveInSource.h"
#include "platform.h"

static CDataSource *createFunc(const TCHAR *prefix)
{
	return new CWaveInSource();
}

void CWindowsDataSourceFactory::Register(void)
{
	CDataSourceFactory::Register(_T("acap://"), createFunc);
}
