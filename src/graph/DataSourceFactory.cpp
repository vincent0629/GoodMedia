#include "DataSourceFactory.h"
#include "CacheSource.h"
#include <string.h>

int CDataSourceFactory::m_nNum = 0;
char CDataSourceFactory::m_pPrefix[10][10] = {0};
DATASOURCE_CREATE_FUNC CDataSourceFactory::m_pFuncs[10] = {NULL};

void CDataSourceFactory::Register(const char *prefix, DATASOURCE_CREATE_FUNC func)
{
	strcpy(m_pPrefix[m_nNum], prefix);
	m_pFuncs[m_nNum] = func;
	++m_nNum;
}

CDataSource *CDataSourceFactory::Create(const char *path)
{
	int i;

	for (i = 0; i < m_nNum; i++)
		if (strncmp(path, m_pPrefix[i], strlen(m_pPrefix[i])) == 0)
			return (*m_pFuncs[i])(m_pPrefix[i]);

	return new CCacheSource();
}
