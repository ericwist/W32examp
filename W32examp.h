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
#include <psapi.h>  // For memory info
#include <shlobj_core.h>
#include <mutex>
#include <queue>

// Declare list box handle globally so i can access it from utility functions
extern HWND ghListBox;
extern BOOL isCheckShowFiles;
extern volatile BOOL gbCancelOperation;  // Flag to cancel current operation

// Memory limit constants
#define MAX_MEMORY_MB 500  // Maximum memory to allow for file lists (500 MB)
#define WARNING_MEMORY_MB 400  // Warning threshold (80% of max)
void printToScreen(WCHAR* FormattedStr);

// Thread-safe printer class
class ThreadSafePrinter {
private:
    std::mutex printMutex;
    std::queue<std::wstring> messageQueue;

public:
    ThreadSafePrinter() {}

    ~ThreadSafePrinter() {
        close();
    }

    void print(const std::wstring& message) {
        std::lock_guard<std::mutex> lock(printMutex);
        if (ghListBox != NULL) {
            printToScreen(const_cast<WCHAR*>(message.c_str()));
        }
    }

    void close() {
        std::lock_guard<std::mutex> lock(printMutex);
        // No resources to clean up for printer
    }
};

// Global thread-safe printer instance
extern ThreadSafePrinter g_printer;

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
        m_dwHash[0] = 0;
        m_dwHash[1] = 0;
        m_dwHash[2] = 0;
        m_dwHash[3] = 0;
    }

    CFileListItem(const std::wstring& fn, const ULONG& sz, const DWORD& lt, const DWORD& ht) 
        : m_Filename(fn), m_Size(sz), m_dwLowDateTime(lt), m_dwHighDateTime(ht) {}
    
    CFileListItem(const std::wstring& fn, const ULONG& h1, const ULONG& h2, const ULONG& h3, const ULONG& h4) 
        : m_Filename(fn) 
    {
        m_dwHash[0] = h1;
        m_dwHash[1] = h2;
        m_dwHash[2] = h3;
        m_dwHash[3] = h4;
    }

    void SetFilename(const std::wstring& s) { m_Filename = s; }
    void SetSize(const ULONG& s) { m_Size = s; }
    void SetLowDateTime(const DWORD& t) { m_dwLowDateTime = t; }
    void SetHighDateTime(const DWORD& t) { m_dwHighDateTime = t; }

    void SetFileHash(const ULONG& h1, const ULONG& h2, const ULONG& h3, const ULONG& h4) 
    {
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
    ULONG m_dwHash[4];

    bool operator==(std::wstring filename)
    {
        return(this->m_Filename == filename);
    }

private:
};

// Memory monitoring functions
SIZE_T GetCurrentMemoryUsage();
BOOL CheckMemoryLimit(SIZE_T currentUsage);

//force this app to Idle and process messages from other apps and the system
inline bool Idle(DWORD ticks = 0)
{
    MSG   msg;
    DWORD max_time = ticks + GetTickCount();
    BOOL  bret = true;

    while (GetTickCount() < max_time)
    {
        while ((bret = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)))
        {
            if (bret)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        Sleep(5);
    }
    return true;
}

//
//   FUNCTION: GetDirRequestorLoad( WCHAR *Path, size_t size )
//
//   PURPOSE: To bring up the directory browser
//
inline int GetDirRequestorLoad(WCHAR* Path, size_t size) {

    BROWSEINFO bi;
    /* Set Open File Name Structure. */
    memset(&bi, 0, sizeof(bi));
    bi.hwndOwner = (HWND)0;
    bi.pidlRoot = NULL;
    bi.lParam = (LPARAM)Path;
    bi.lpszTitle = L"Select a folder";
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    bi.iImage = 0;

    LPITEMIDLIST pIDList = SHBrowseForFolder(&bi);
    if (pIDList)
    {
        // Create a buffer to store the path, then get the path.
        WCHAR buffer[MAX_PATH] = { '0' };
        if (::SHGetPathFromIDList(pIDList, buffer) != 0)
        {
            wcscpy_s(Path, size, buffer);
        }
        // free the item id list
        CoTaskMemFree(pIDList);
        return TRUE;
    }
    return FALSE;
}




