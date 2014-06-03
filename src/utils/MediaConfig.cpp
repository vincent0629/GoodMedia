#include "MediaConfig.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

CMediaConfig::CMediaConfig()
{
	m_bModified = false;
}

CMediaConfig::~CMediaConfig()
{
	Save();
	Reset();
}

bool CMediaConfig::Load(const char *path)
{
	FILE *file;
	char str[256], *p;
	Property *pProperty;

	Save();
	Reset();

	strcpy(m_pPath, path);
	file = fopen(path, "rt");
	if (file == NULL)
		return false;

	while (fgets(str, 256, file))
	{
		if (str[strlen(str) - 1] == '\n')
			str[strlen(str) - 1] = '\0';
		if (str[0] != '#')  //not comment
		{
			p = strchr(str, '=');
			if (p != NULL)
			{
				*(p++) = '\0';
				pProperty = new Property;				
				pProperty->key = new char[p - str];
				strcpy(pProperty->key, str);
				pProperty->value = new char[strlen(p) + 1];
				strcpy(pProperty->value, p);
				m_Properties.push_back(pProperty);
			}
		}
	}
	fclose(file);
	return true;
}

bool CMediaConfig::Save(void)
{
	FILE *file;
	list<Property *>::iterator it;
	Property *pProperty;

	if (!m_bModified)
		return true;

	file = fopen(m_pPath, "wt");
	if (file == NULL)
		return false;

	for (it = m_Properties.begin(); it != m_Properties.end(); it++)
	{
		pProperty = *it;
		fprintf(file, "%s=%s\n", pProperty->key, pProperty->value);
	}
	fclose(file);
	m_bModified = false;
	return true;
}

CMediaConfig::Property *CMediaConfig::Search(const char *key, bool bCreate)
{
	list<Property *>::iterator it;
	Property *pProperty;

	for (it = m_Properties.begin(); it != m_Properties.end(); it++)
	{
		pProperty = *it;
		if (strcmp(pProperty->key, key) == 0)
			return pProperty;
	}

	if (bCreate)
	{
		pProperty->key = new char[strlen(key) + 1];
		strcpy(pProperty->key, key);
		m_Properties.push_back(pProperty);
		return pProperty;
	}
	return NULL;
}

int CMediaConfig::GetInt(const char *key, int nDefault)
{
	Property *pProp;

	pProp = Search(key, false);
	return pProp == NULL? nDefault : atoi(pProp->value);
}

const char *CMediaConfig::GetString(const char *key, const char *pDefault)
{
	Property *pProp;

	pProp = Search(key, false);
	return pProp == NULL? pDefault : pProp->value;
}

void CMediaConfig::SetInt(const char *key, int nValue)
{
	Property *pProp;
	char str[10];

	pProp = Search(key, true);
	delete[] pProp->value;
	sprintf(str, "%d", nValue);
	pProp->value = new char[strlen(str) + 1];
	strcpy(pProp->value, str);
	m_bModified = true;
}

void CMediaConfig::SetString(const char *key, const char *pValue)
{
	Property *pProp;

	pProp = Search(key, true);
	delete[] pProp->value;
	pProp->value = new char[strlen(pValue) + 1];
	strcpy(pProp->value, pValue);
	m_bModified = true;
}

void CMediaConfig::Reset(void)
{
	list<Property *>::iterator it;
	Property *pProperty;

	for (it = m_Properties.begin(); it != m_Properties.end(); it++)
	{
		pProperty = *it;
		delete[] pProperty->key;
		delete[] pProperty->value;
		delete pProperty;
	}
	m_Properties.clear();
}
