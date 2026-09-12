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
#include <cstdint>  // Add this for std::uint64_t

// Declare list box handle globally so i can access it from utility functions
extern HWND ghListBox;
extern BOOL isCheckShowFiles;
extern volatile BOOL gbCancelOperation;  // Flag to cancel current operation

// Memory limit constants
#define MAX_MEMORY_MB 500  // Maximum memory to allow for file lists (500 MB)
#define WARNING_MEMORY_MB 400  // Warning threshold (80% of max)
void printToScreen(WCHAR* FormattedStr);

// Memory monitoring functions (changed from SIZE_T to std::uint64_t for cross-platform)
std::uint64_t GetCurrentMemoryUsage();
BOOL CheckMemoryLimit(std::uint64_t currentUsage);

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

/*
 * Output Callback Interface for Cross-Platform GUI Integration
 * 
 * This interface allows platform-agnostic code to send output messages
 * to the GUI without direct Win32 dependencies.
 */

// Callback function type for output messages
typedef void (*OutputCallback)(const wchar_t* message, void* context);

// Output Manager class - handles all output callbacks
class OutputManager {
private:
    OutputCallback callback;
    void* context;
    std::mutex callbackMutex;

public:
    OutputManager() : callback(nullptr), context(nullptr) {}

    ~OutputManager() {
        close();
    }

    // Register an output callback
    void RegisterCallback(OutputCallback cb, void* ctx = nullptr) {
        std::lock_guard<std::mutex> lock(callbackMutex);
        callback = cb;
        context = ctx;
    }

    // Overload for std::wstring
    void SendOutput(const std::wstring& message) {
        std::lock_guard<std::mutex> lock(callbackMutex);
        if (callback != nullptr) {
            callback(message.c_str(), context);
        }
    }

    // Overload for raw wchar_t*
    void SendOutput(const wchar_t* message) {
        std::lock_guard<std::mutex> lock(callbackMutex);
        if (callback != nullptr) {
            callback(message, context);
        }
    }

    // Send formatted output
    void SendFormattedOutput(const wchar_t* format, ...) {
        wchar_t buffer[4096];
        va_list args;
        va_start(args, format);
        vswprintf_s(buffer, sizeof(buffer) / sizeof(wchar_t), format, args);
        va_end(args);
        SendOutput(buffer);
    }

    // Clear/close the callback
    void close() {
        std::lock_guard<std::mutex> lock(callbackMutex);
        callback = nullptr;
        context = nullptr;
    }
};

// Global output manager instance
extern OutputManager g_outputManager;

// Default Win32 GUI callback implementation
void Win32GuiOutputCallback(const wchar_t* message, void* context);












