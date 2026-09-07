/*****************
* W32examp.h
* Standard windows screen in C++
* for example code
* Author: Eric Wistrand
* Jul 26, 2023
*****************/
#pragma once

#include "resource.h"
#include <psapi.h>  // For memory info
#include <shlobj_core.h>
#include <mutex>
#include <queue>
#include <string>

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

// Memory monitoring functions
SIZE_T GetCurrentMemoryUsage();
BOOL CheckMemoryLimit(SIZE_T currentUsage);

//force this app to Idle and process messages from other apps and the system
inline bool Idle(DWORD ticks = 0)
{
    MSG   msg;
    ULONGLONG start_time = GetTickCount64();
    ULONGLONG max_time = start_time + static_cast<ULONGLONG>(ticks);
    BOOL  bret = true;

    while (GetTickCount64() < max_time)
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
        WCHAR buffer[MAX_PATH+1] = { '0' };
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












