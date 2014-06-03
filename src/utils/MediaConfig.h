#ifndef _MEDIACONFIG_H_
#define _MEDIACONFIG_H_

#include <list>

class CMediaConfig
{
public:
	typedef struct
	{
		char *key;
		char *value;
	} Property;
public:
	CMediaConfig();
	~CMediaConfig();
	bool Load(const char *path);
	int GetInt(const char *key, int nDefault);
	const char *GetString(const char *key, const char *pDefault);
	void SetInt(const char *key, int nValue);
	void SetString(const char *key, const char *pValue);
protected:
	Property *Search(const char *key, bool bCreate);
	bool Save(void);
	void Reset(void);
private:
	char m_pPath[256];
	bool m_bModified;
	std::list<Property *> m_Properties;
};

#endif
