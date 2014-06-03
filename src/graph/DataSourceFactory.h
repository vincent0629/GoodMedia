#ifndef _DATASOURCEFACTORY_H_
#define _DATASOURCEFACTORY_H_

#include "DataSource.h"

typedef CDataSource *(*DATASOURCE_CREATE_FUNC)(const char *prefix);

class CDataSourceFactory
{
public:
	static void Register(const char *prefix, DATASOURCE_CREATE_FUNC func);
	static CDataSource *Create(const char *path);
private:
	static int m_nNum;
	static char m_pPrefix[10][10];
	static DATASOURCE_CREATE_FUNC m_pFuncs[10];
};

#endif
