// curlsmtp.cpp : main source file for curlsmtp.exe
//

#include "stdafx.h"

#include "resource.h"

// Note: Proxy/Stub Information
//		To build a separate proxy/stub DLL, 
//		run nmake -f curlsmtpps.mk in the project directory.
#include "initguid.h"
#include "curlsmtp.h"
#include "curlsmtp_i.c"

#include "aboutdlg.h"
#include "MainDlg.h"

CServerAppModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;

	if(dlgMain.Create(NULL) == NULL)
	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	_Module.Lock();
	dlgMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

#include "curl_smtp.h"
#include "ConfigHelper.h"
__inline static void string_split_to_vector(std::vector<std::string> & sv, std::string strData, std::string strSplitter, std::string::size_type stPos = 0)
{
	std::string strTmp = ("");
	std::string::size_type stIdx = 0;
	std::string::size_type stSize = strData.length();

	while ((stPos = strData.find(strSplitter, stIdx)) != std::string::npos)
	{
		strTmp = strData.substr(stIdx, stPos - stIdx);
		if (!strTmp.length())
		{
			strTmp = ("");
		}
		sv.push_back(strTmp);

		stIdx = stPos + strSplitter.length();
	}

	if (stIdx < stSize)
	{
		strTmp = strData.substr(stIdx, stSize - stIdx);
		if (!strTmp.length())
		{
			strTmp = ("");
		}
		sv.push_back(strTmp);
	}
}
__inline static void wstring_split_to_vector(std::vector<std::wstring> & wsv, std::wstring wstrData, std::wstring wstrSplitter, std::wstring::size_type stPos = 0)
{
	std::wstring wstrTemp = (L"");
	std::wstring::size_type stIdx = 0;
	std::wstring::size_type stSize = wstrData.length();

	while ((stPos = wstrData.find(wstrSplitter, stIdx)) != std::wstring::npos)
	{
		wstrTemp = wstrData.substr(stIdx, stPos - stIdx);
		if (!wstrTemp.length())
		{
			wstrTemp = (L"");
		}
		wsv.push_back(wstrTemp);

		stIdx = stPos + wstrSplitter.length();
	}

	if (stIdx < stSize)
	{
		wstrTemp = wstrData.substr(stIdx, stSize - stIdx);
		if (!wstrTemp.length())
		{
			wstrTemp = (L"");
		}
		wsv.push_back(wstrTemp);
	}
}
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	AllocConsole();
	freopen("CONIN$", "rb", stdin);
	freopen("CONOUT$", "wb", stdout);
	freopen("CONOUT$", "wb", stderr);
	STRINGSTRINGSTRINGMAPMAP sssmm;
	CConfigHelper::getInstance()->InitRead(sssmm);

	USES_CONVERSION_EX;
	std::string url = CT2A(sssmm.at(L"PUBLIC").at(L"URL").c_str());
	std::string username = CT2A(sssmm.at(L"PUBLIC").at(L"USERNAME").c_str());
	std::string password = CT2A(sssmm.at(L"PUBLIC").at(L"PASSWORD").c_str());
	std::string from = CT2A(sssmm.at(L"PUBLIC").at(L"FROM").c_str());
	std::string fromname = CT2A(sssmm.at(L"PUBLIC").at(L"FROMNAME").c_str());
	std::string to = CT2A(sssmm.at(L"PUBLIC").at(L"TO").c_str());
	std::string toname = CT2A(sssmm.at(L"PUBLIC").at(L"TONAME").c_str());
	std::string cc = CT2A(sssmm.at(L"PUBLIC").at(L"CC").c_str());
	std::string ccname = CT2A(sssmm.at(L"PUBLIC").at(L"CCNAME").c_str());
	std::string bcc = CT2A(sssmm.at(L"PUBLIC").at(L"BCC").c_str());
	std::string bccname = CT2A(sssmm.at(L"PUBLIC").at(L"BCCNAME").c_str());
	std::string file = CT2A(sssmm.at(L"PUBLIC").at(L"FILES").c_str());
	std::string subject = CT2A(sssmm.at(L"PUBLIC").at(L"SUBJECT").c_str());
	std::string body = CT2A(sssmm.at(L"PUBLIC").at(L"BODY").c_str());
	std::vector<std::string> tos;
	std::vector<std::string> ccs;
	std::vector<std::string> bccs;
	std::vector<std::string> tonames;
	std::vector<std::string> ccnames;
	std::vector<std::string> bccnames;
	std::vector<std::string> headers;
	std::vector<std::string> files;

	string_split_to_vector(tos, to, ",");
	string_split_to_vector(tonames, toname, ",");
	string_split_to_vector(ccs, cc, ",");
	string_split_to_vector(ccnames, ccname, ",");
	string_split_to_vector(bccs, bcc, ",");
	string_split_to_vector(bccnames, bccname, ",");
	string_split_to_vector(files, file, ",");

	curl_smtp(url, username, password, from, fromname, tos, ccs, bccs, tonames, ccnames, bccnames, headers, files, subject, body);
	//curl_smtp();
	getchar();
	FreeConsole();
	return 0;

	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(ObjectMap, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	AtlAxWinInit();

	// Parse command line, register or unregister or run the server
	int nRet = 0;
	TCHAR szTokens[] = _T("-/");
	bool bRun = true;
	bool bAutomation = false;

	LPCTSTR lpszToken = _Module.FindOneOf(::GetCommandLine(), szTokens);
	while(lpszToken != NULL)
	{
		if(lstrcmpi(lpszToken, _T("UnregServer")) == 0)
		{
			_Module.UpdateRegistryFromResource(IDR_CURLSMTP, FALSE);
			nRet = _Module.UnregisterServer(TRUE);
			bRun = false;
			break;
		}
		else if(lstrcmpi(lpszToken, _T("RegServer")) == 0)
		{
			_Module.UpdateRegistryFromResource(IDR_CURLSMTP, TRUE);
			nRet = _Module.RegisterServer(TRUE);
			bRun = false;
			break;
		}
		else if((lstrcmpi(lpszToken, _T("Automation")) == 0) ||
			(lstrcmpi(lpszToken, _T("Embedding")) == 0))
		{
			bAutomation = true;
			break;
		}
		lpszToken = _Module.FindOneOf(lpszToken, szTokens);
	}

	if(bRun)
	{
		_Module.StartMonitor();
		hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED);
		ATLASSERT(SUCCEEDED(hRes));
		hRes = ::CoResumeClassObjects();
		ATLASSERT(SUCCEEDED(hRes));

		if(bAutomation)
		{
			CMessageLoop theLoop;
			nRet = theLoop.Run();
		}
		else
		{
			nRet = Run(lpstrCmdLine, nCmdShow);
		}

		_Module.RevokeClassObjects();
		::Sleep(_Module.m_dwPause);
	}

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
