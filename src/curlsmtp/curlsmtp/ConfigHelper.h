
#include <map>
#include <string>
#include <windows.h>

#define CONF_FILE_NAME_A "config.ini"
#define CONF_FILE_NAME_W L"" CONF_FILE_NAME_A

typedef std::map<std::string, std::string> STRINGSTRINGMAPA;
typedef std::map<std::string, STRINGSTRINGMAPA> STRINGSTRINGSTRINGMAPMAPA;
typedef std::map<std::wstring, std::wstring> STRINGSTRINGMAPW;
typedef std::map<std::wstring, STRINGSTRINGMAPW> STRINGSTRINGSTRINGMAPMAPW;

class CConfigHelperA {
public:
	//获取程序文件路径
	__inline static std::string GetAppPath()
	{
		std::string tsFilePath = ("");
		CHAR * pFoundPosition = 0;
		CHAR tFilePath[MAX_PATH] = { 0 };
		::GetModuleFileNameA(NULL, tFilePath, MAX_PATH);
		if (*tFilePath)
		{
			pFoundPosition = strrchr(tFilePath, ('\\'));
			if (*(++pFoundPosition))
			{
				*pFoundPosition = ('\0');
			}
			tsFilePath = tFilePath;
		}
		return tsFilePath;
	}

	CConfigHelperA(LPCSTR lpPathFileName = (CONF_FILE_NAME_A)) {
		m_tsPathFileName = CConfigHelperA::GetAppPath() + lpPathFileName;
	};
	virtual ~CConfigHelperA() {};

	void InitRead(STRINGSTRINGSTRINGMAPMAPA & tttmapmap)
	{
		LPSTR lpTSD = NULL;
		DWORD dwSectionNames = (0L);
		LPSTR lpSectionNames = (0L);
		LPSTR lpTK = NULL;
		DWORD dwKeyNames = (0L);
		LPSTR lpKeyNames = (0L);
		LPSTR lpVL = NULL;

		lpTSD = NULL;
		dwSectionNames = MAXBYTE;
		lpSectionNames = (LPSTR)::GlobalAlloc(GPTR, dwSectionNames * sizeof(CHAR));

		while ((::GetPrivateProfileSectionNamesA(lpSectionNames, dwSectionNames, m_tsPathFileName.c_str())) &&
			((::GetLastError() == ERROR_MORE_DATA) || (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)))
		{
			lpTSD = (LPSTR)::GlobalReAlloc(lpSectionNames, (dwSectionNames + dwSectionNames) * sizeof(CHAR), GHND);
			if (!lpTSD)
			{
				if (lpSectionNames)
				{
					::GlobalFree(lpSectionNames);
					lpSectionNames = (0L);
				}
			}
			else
			{
				lpSectionNames = lpTSD;
				dwSectionNames += dwSectionNames;
			}
		}
		lpTSD = lpSectionNames;
		while (lpTSD && (*lpTSD))
		{
			tttmapmap.insert(STRINGSTRINGSTRINGMAPMAPA::value_type(lpTSD, STRINGSTRINGMAPA{}));

			lpTK = NULL;
			dwKeyNames = MAXBYTE;
			lpKeyNames = (LPSTR)::GlobalAlloc(GPTR, dwKeyNames * sizeof(CHAR));

			while ((::GetPrivateProfileSectionA(lpTSD, lpKeyNames, dwKeyNames, m_tsPathFileName.c_str())) &&
				((::GetLastError() == ERROR_MORE_DATA) || (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)))
			{
				lpTK = (LPSTR)::GlobalReAlloc(lpKeyNames, (dwKeyNames + dwKeyNames) * sizeof(CHAR), GHND);
				if (!lpTK)
				{
					if (lpKeyNames)
					{
						::GlobalFree(lpKeyNames);
						lpKeyNames = (0L);
					}
				}
				else
				{
					lpKeyNames = lpTK;
					dwKeyNames += dwKeyNames;
				}
			}
			lpTK = lpKeyNames;
			while (lpTK && (*lpTK))
			{
				if ((lpVL = strchr(lpTK, ('\x3D'))) && (lpTK != lpVL))
				{
					*(lpVL) = ('\x00');//''
					tttmapmap.at(lpTSD).insert(STRINGSTRINGMAPA::value_type(lpTK, lpVL + 1));
					*(lpVL) = ('\x3D');//'='
				}
				lpTK += strlen(lpTK) + 1;
			}
			if (lpKeyNames)
			{
				::GlobalFree(lpKeyNames);
				lpKeyNames = (0L);
			}

			lpTSD += strlen(lpTSD) + 1;
		}
		if (lpSectionNames)
		{
			::GlobalFree(lpSectionNames);
			lpSectionNames = (0L);
		}
	}

	//! ConfigHelper Singleton
	static CConfigHelperA * getInstance(LPCSTR lpPathFileName = (CONF_FILE_NAME_A)) {
		static CConfigHelperA instance(lpPathFileName);
		return &instance;
	}

public:
	std::string ReadString(LPCSTR lpSectionName, LPCSTR lpKeyName, LPCSTR lpDefaultValue)
	{
		std::string t(USHRT_MAX, ('\0'));
		::GetPrivateProfileStringA(lpSectionName, lpKeyName, lpDefaultValue, (LPSTR)t.c_str(), t.size(), m_tsPathFileName.c_str());
		return t.c_str();
	}

	BOOL WriteString(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpValue)
	{
		return ::WritePrivateProfileStringA(lpAppName, lpKeyName, lpValue, m_tsPathFileName.c_str());
	}

private:

	std::string m_tsPathFileName;
};

class CConfigHelperW {
public:
	//获取程序文件路径
	__inline static std::wstring GetAppPath()
	{
		std::wstring tsFilePath = (L"");
		WCHAR * pFoundPosition = 0;
		WCHAR tFilePath[MAX_PATH] = { 0 };
		::GetModuleFileNameW(NULL, tFilePath, MAX_PATH);
		if (*tFilePath)
		{
			pFoundPosition = wcsrchr(tFilePath, (L'\\'));
			if (*(++pFoundPosition))
			{
				*pFoundPosition = (L'\0');
			}
			tsFilePath = tFilePath;
		}
		return tsFilePath;
	}

	CConfigHelperW(LPCWSTR lpPathFileName = (CONF_FILE_NAME_W)) {
		m_tsPathFileName = CConfigHelperW::GetAppPath() + lpPathFileName;
	};
	virtual ~CConfigHelperW() {};

	void InitRead(STRINGSTRINGSTRINGMAPMAPW & tttmapmap)
	{
		LPWSTR lpTSD = NULL;
		DWORD dwSectionNames = (0L);
		LPWSTR lpSectionNames = (0L);
		LPWSTR lpTK = NULL;
		DWORD dwKeyNames = (0L);
		LPWSTR lpKeyNames = (0L);
		LPWSTR lpVL = NULL;

		lpTSD = NULL;
		dwSectionNames = MAXBYTE;
		lpSectionNames = (LPWSTR)::GlobalAlloc(GPTR, dwSectionNames * sizeof(WCHAR));

		while ((::GetPrivateProfileSectionNamesW(lpSectionNames, dwSectionNames, m_tsPathFileName.c_str()) <= (0)) ||
			((::GetLastError() == ERROR_MORE_DATA) || (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)))
		{
			lpTSD = (LPWSTR)::GlobalReAlloc(lpSectionNames, (dwSectionNames + dwSectionNames) * sizeof(WCHAR), GHND);
			if (!lpTSD)
			{
				if (lpSectionNames)
				{
					::GlobalFree(lpSectionNames);
					lpSectionNames = (0L);
				}
			}
			else
			{
				lpSectionNames = lpTSD;
				dwSectionNames += dwSectionNames;
			}
		}
		lpTSD = lpSectionNames;
		while (lpTSD && (*lpTSD))
		{
			tttmapmap.insert(STRINGSTRINGSTRINGMAPMAPW::value_type(lpTSD, STRINGSTRINGMAPW{}));

			lpTK = NULL;
			dwKeyNames = MAXBYTE;
			lpKeyNames = (LPWSTR)::GlobalAlloc(GPTR, dwKeyNames * sizeof(WCHAR));
			::GetPrivateProfileSectionW(lpTSD, lpKeyNames, dwKeyNames, m_tsPathFileName.c_str());
			DWORD dwErr = GetLastError();
			while ((::GetPrivateProfileSectionW(lpTSD, lpKeyNames, dwKeyNames, m_tsPathFileName.c_str()) == (dwSectionNames - 2)) ||
				((::GetLastError() == ERROR_MORE_DATA) || (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)))
			{
				lpTK = (LPWSTR)::GlobalReAlloc(lpKeyNames, (dwKeyNames + dwKeyNames) * sizeof(WCHAR), GHND);
				if (!lpTK)
				{
					if (lpKeyNames)
					{
						::GlobalFree(lpKeyNames);
						lpKeyNames = (0L);
					}
				}
				else
				{
					lpKeyNames = lpTK;
					dwKeyNames += dwKeyNames;
				}
			}
			lpTK = lpKeyNames;
			while (lpTK && (*lpTK))
			{
				if ((lpVL = wcschr(lpTK, (L'\x3D'))) && (lpTK != lpVL))
				{
					*(lpVL) = (L'\x00');//''
					tttmapmap.at(lpTSD).insert(STRINGSTRINGMAPW::value_type(lpTK, lpVL + 1));
					*(lpVL) = (L'\x3D');//'='
				}
				lpTK += wcslen(lpTK) + 1;
			}
			if (lpKeyNames)
			{
				::GlobalFree(lpKeyNames);
				lpKeyNames = (0L);
			}

			lpTSD += wcslen(lpTSD) + 1;
		}
		if (lpSectionNames)
		{
			::GlobalFree(lpSectionNames);
			lpSectionNames = (0L);
		}
	}

	//! ConfigHelper Singleton
	static CConfigHelperW * getInstance(LPCWSTR lpPathFileName = (CONF_FILE_NAME_W)) {
		static CConfigHelperW instance(lpPathFileName);
		return &instance;
	}

public:
	std::wstring ReadString(LPCWSTR lpSectionName, LPCWSTR lpKeyName, LPCWSTR lpDefaultValue)
	{
		std::wstring t(USHRT_MAX, (L'\0'));
		::GetPrivateProfileStringW(lpSectionName, lpKeyName, lpDefaultValue, (LPWSTR)t.c_str(), t.size(), m_tsPathFileName.c_str());
		return t.c_str();
	}

	BOOL WriteString(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpValue)
	{
		return ::WritePrivateProfileStringW(lpAppName, lpKeyName, lpValue, m_tsPathFileName.c_str());
	}

private:

	std::wstring m_tsPathFileName;
};

#if !defined(_UNICODE) && !defined(UNICODE)
#define STRINGSTRINGMAP				STRINGSTRINGMAPA
#define STRINGSTRINGSTRINGMAPMAP	STRINGSTRINGSTRINGMAPMAPA
#define CConfigHelper				CConfigHelperA
#else
#define STRINGSTRINGMAP				STRINGSTRINGMAPW
#define STRINGSTRINGSTRINGMAPMAP	STRINGSTRINGSTRINGMAPMAPW
#define CConfigHelper				CConfigHelperW
#endif