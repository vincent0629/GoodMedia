#include "FileSource.h"
#include <string.h>

CFileSource::CFileSource()
{
	m_pFile = NULL;
	m_nSize = 0;
}

CFileSource::~CFileSource()
{
	Close();
}

bool CFileSource::Open(const char *path)
{
	fpos_t pos;

	Close();

	m_pFile = fopen(path, "rb");
	if (m_pFile != NULL)
	{
		if (fseek(m_pFile, 0, SEEK_END) == 0)
		{
			fgetpos(m_pFile, &pos);
#ifdef _WIN32
			m_nSize = pos;
#else
			m_nSize = pos.__pos;
#endif
			fseek(m_pFile, 0, SEEK_SET);
		}
		else  //the file is too large?
			m_nSize = 0;
	}

	return m_pFile != NULL;
}

void CFileSource::Close(void)
{
	if (m_pFile != NULL)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
	m_nSize = 0;
}

size_t CFileSource::GetSize(void)
{
	return m_nSize;
}

size_t CFileSource::GetPosition(void)
{
	fpos_t pos;

	if (m_pFile == NULL)
		return 0;

	fgetpos(m_pFile, &pos);
#ifdef _WIN32
	return pos;
#else
	return pos.__pos;
#endif
}

int CFileSource::ReadData(void)
{
	return m_pFile == NULL? -1 : fgetc(m_pFile);
}

int CFileSource::ReadData(void *pData, int nSize)
{
	return m_pFile == NULL? 0 : fread(pData, 1, nSize, m_pFile);
}

bool CFileSource::SeekData(size_t nOffset, int nFrom)
{
	fpos_t pos;

	if (m_pFile == NULL)
		return false;

	if (nFrom == SEEK_CUR)
	{
		fgetpos(m_pFile, &pos);
#ifdef _WIN32
		nOffset += pos;
#else
		nOffset += pos.__pos;
#endif
	}
	else if (nFrom != SEEK_SET)
		return false;
	if (nOffset > m_nSize && m_nSize >= 0)
	{
		fseek(m_pFile, 0, SEEK_END);
		return false;
	}

#ifdef _WIN32
	pos = nOffset;
#else
	pos.__pos = nOffset;
#endif
	fsetpos(m_pFile, &pos);
	return true;
}
