//--------------------------------------------------------------------------------------
// File: q27.cpp
//--------------------------------------------------------------------------------------
#include "stdafx.h"
#include <shellapi.h>

//--------------------------------------------------------------------------------------
// Constant
const TCHAR* kConfigFile = _T("./q27.ini");

const TCHAR* kBaseUrl = _T("https://member.gungho.jp/front/member/webgs/rocenter.aspx?SIID=");
const TCHAR* kIdFormLoginName   = _T("ctl00_ctl00_MainContent_TopContent_loginNameControl_txtLoginName");
const TCHAR* kIdFormPassword    = _T("ctl00_ctl00_MainContent_TopContent_passwordControl_txtPassword");
const TCHAR* kIdFormSubmit      = _T("ctl00_ctl00_MainContent_TopContent_ibtNext1");

//--------------------------------------------------------------------------------------
// Global variables
_bstr_t SIID;
_bstr_t SectionName;

//--------------------------------------------------------------------------------------
void FormatCOMError(_com_error& e)
{
    _tprintf(_T("*** _com_error 0x%08X\n"), e.Error());
    _tprintf(_T("%s\n"), e.ErrorMessage());
}

//--------------------------------------------------------------------------------------
void RunProcess(_bstr_t& AttractionID, _bstr_t& password)
{
    _bstr_t pathWork;
    _bstr_t pathExec;
    _bstr_t parameters;
    TCHAR buffer[1024];

    if (::GetPrivateProfileString(_T("Application"), _T("StartIn"), NULL, buffer, 1024, kConfigFile) > 0)
    {
        pathWork = buffer;
        
        _stprintf(buffer, _T("%s\\Ragexe.exe"), static_cast<TCHAR*>(pathWork));
        pathExec = buffer;
    }

    _stprintf(buffer, _T("1rag1 -w -u:%s -p:%s"), static_cast<TCHAR*>(AttractionID), static_cast<TCHAR*>(password));
    parameters = buffer;

    ::ShellExecute(NULL, _T("open"), pathExec, parameters, pathWork, SW_SHOWNORMAL);
}

//--------------------------------------------------------------------------------------
HRESULT WaitComplete(SHDocVw::IWebBrowser2Ptr& web2)
{
    while (1)
    {
        if ((web2->Busy!=VARIANT_TRUE) &&
            (web2->ReadyState==READYSTATE_COMPLETE))
            break;
        ::Sleep(1000);
    }

    return S_OK;
}

HRESULT ProcessWebLogin(SHDocVw::IWebBrowser2Ptr& web2)
{
    _bstr_t GunghoID;
    _bstr_t password;
    TCHAR buffer[1024];

    if (::GetPrivateProfileString(static_cast<TCHAR*>(SectionName), _T("GunghoID"), NULL, buffer, 1024, kConfigFile) > 0)
        GunghoID = buffer;
    if (::GetPrivateProfileString(static_cast<TCHAR*>(SectionName), _T("Password"), NULL, buffer, 1024, kConfigFile) > 0)
        password = buffer;

    HRESULT hr;

    try
    {
        MSHTML::IHTMLDocument3Ptr html3 = web2->Document;
        MSHTML::IHTMLInputTextElementPtr elmInput;
        
        // - LoginName
        elmInput = html3->getElementById(kIdFormLoginName);
        _tprintf(_T("LoginName: %s\n"), static_cast<TCHAR*>(GunghoID));
        elmInput->put_value(GunghoID);
        _tprintf(_T("-> %s\n"), static_cast<TCHAR*>(elmInput->Getvalue()));

        // - Password
        elmInput = html3->getElementById(kIdFormPassword);
        _tprintf(_T("Password: %s\n"), static_cast<TCHAR*>(password));
        elmInput->put_value(password);
        _tprintf(_T("-> %s\n"), static_cast<TCHAR*>(elmInput->Getvalue()));

        // Click submit.
        MSHTML::IHTMLElementPtr element = html3->getElementById(kIdFormSubmit);
        hr = element->click();
        if (FAILED(hr))
        {
            _tprintf(_T("*** Failure click 0x%08X\n"), hr);
            return hr;
        }

        WaitComplete(web2);
        _tprintf(_T("%s\n"), static_cast<TCHAR*>(web2->LocationURL));
    }
    catch (...)
    {
        throw;
    }

    return S_OK;
}

void NoticeLogout(SHDocVw::IWebBrowser2Ptr& web2)
{
    _tprintf(_T("Please logout currently account.\n"));

    HRESULT hr = web2->put_Visible(VARIANT_TRUE);
    if (FAILED(hr))
        _tprintf(_T("*** Failure put_visible 0x%08X\n"), hr);
}

//--------------------------------------------------------------------------------------
int _tmain(int argc, wchar_t** argv)
{
    //----------------------------------------------------------------------------------
#ifndef _DEBUG
    if (argc != 2)
    {
        _tprintf(_T("Unknown arguments.\n"));
        return -1;
    }
    else
#endif
    {
#ifndef _DEBUG
        SectionName = argv[1];
#else
        SectionName = _T("DBGTEST");
#endif

        TCHAR buffer[1024];

        if (::GetPrivateProfileString(static_cast<TCHAR*>(SectionName), _T("SIID"), NULL, buffer, 1024, kConfigFile) > 0)
            SIID = buffer;
    }

    //----------------------------------------------------------------------------------
    HRESULT hr;

    hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        _tprintf(_T("*** Failure CoInitializeEx. (%08X)\n"), hr);
        return -1;
    }

    try
    {
        //------------------------------------------------------------------------------
        _bstr_t url = kBaseUrl;
        url += SIID;
        _tprintf(_T("%s\n"), static_cast<TCHAR*>(url));

        //------------------------------------------------------------------------------
        SHDocVw::IWebBrowser2Ptr web2;

        // Create InternetExplorer instance.
        hr = web2.CreateInstance(__uuidof(SHDocVw::InternetExplorer));
        if (FAILED(hr))
        {
            _tprintf(_T("*** Failure create InternetExplorer instance. 0x%08X\n"), hr);
            throw;
        }

        // Connect
        hr = web2->Navigate(url);
        if (FAILED(hr))
        {
            _tprintf(_T("*** Failure Navigate 0x%08X\n"), hr);
            throw;
        }

        WaitComplete(web2);
        _tprintf(_T("%s\n"), static_cast<TCHAR*>(web2->LocationURL));

        if (url == web2->LocationURL)
        {
        }
        else
        {
            hr = ProcessWebLogin(web2);
            if (FAILED(hr))
                throw;
        }

        //------------------------------------------------------------------------------
        MSHTML::IHTMLDocument3Ptr html3 = web2->Document;

        MSHTML::IHTMLObjectElementPtr elmActiveX = html3->getElementById(_T("LoadPrgAx"));
        if (elmActiveX == NULL)
        {
            NoticeLogout(web2);
            throw;
        }

        MSHTML::IHTMLElementCollectionPtr collection = html3->getElementsByTagName(_T("param"));
        _bstr_t nameAttractionID = _T("AttractionID");
        _bstr_t namePassword = _T("Password");
        _bstr_t AttractionID;
        _bstr_t password;

        for (long i=0; i<collection->length; i++)
        {
            MSHTML::IHTMLParamElementPtr elmParam = collection->item(i, 0);
            if (elmParam->name == nameAttractionID)
                AttractionID = elmParam->value;
            if (elmParam->name == namePassword)
                password = elmParam->value;
        }

        web2->Quit();

        //------------------------------------------------------------------------------
        _tprintf(_T("AttractionID: %s\n"), static_cast<TCHAR*>(AttractionID));
        _tprintf(_T("Password: %s\n"), static_cast<TCHAR*>(password));

        RunProcess(AttractionID, password);
    }
    catch (_com_error& e)
    {
        FormatCOMError(e);
        throw;
    }
    catch (...)
    {
        _tprintf(_T("*** Force exit.\n"));
        ::CoUninitialize();
        return -1;
    }

    ::CoUninitialize();
    return 0;
}
