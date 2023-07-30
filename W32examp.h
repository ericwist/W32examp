/*****************
* W32examp.h
* Standard windows screen in C++
* for example code
* Author: Eric Wistrand
* Jul 26, 2023
*****************/
#pragma once

#include "resource.h"
#include <string>

// Declare list box handle globally so i can access it from utility functions
extern HWND ghListBox;
extern BOOL isCheckShowFiles;
// This is a class to store the file items, meaning file properties and/or hashs
class CFileListItem
{
public:

    CFileListItem()
    {
        m_Filename = L"";
        m_Size = 0;
        m_lastError = 0;
        m_dwLowDateTime = 0;
        m_dwHighDateTime = 0;
        //slow compare
        m_dwHash[0] = 0;
        m_dwHash[1] = 0;
        m_dwHash[2] = 0;
        m_dwHash[3] = 0;
    }

    CFileListItem(const std::wstring& fn, const ULONG& sz, const DWORD& lt, const DWORD& ht) : m_Filename(fn), m_Size(sz), m_dwLowDateTime(lt), m_dwHighDateTime(ht) {}
    CFileListItem(const std::wstring& fn, const ULONG& h1, const ULONG& h2, const ULONG& h3, const ULONG& h4) : m_Filename(fn) {
        m_dwHash[0] = h1;
        m_dwHash[1] = h2;
        m_dwHash[2] = h3;
        m_dwHash[3] = h4;
    }
    void SetFilename(const std::wstring& s) { m_Filename = s; }
    void SetSize(const ULONG& s) { m_Size = s; }
    void SetLowDateTime(const DWORD& t) { m_dwLowDateTime = t; }
    void SetHighDateTime(const DWORD& t) { m_dwHighDateTime = t; }

    void SetFileHash(const ULONG& h1, const ULONG& h2, const ULONG& h3, const ULONG& h4) {
        m_dwHash[0] = h1;
        m_dwHash[1] = h2;
        m_dwHash[2] = h3;
        m_dwHash[3] = h4;
    }

    std::wstring m_Filename;
    ULONG    m_Size;
    DWORD    m_dwLowDateTime;
    DWORD    m_dwHighDateTime;
    DWORD    m_lastError;
    int      m_lockState;
    //slow compare
    ULONG m_dwHash[4];

    bool operator==(std::wstring filename)
    {
        return(this->m_Filename == filename);
    }

private:

};


