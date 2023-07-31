/*****************
* util.h
* Some Common untility functions
* for example code
* Author: Eric Wistrand
* Jul 25, 2023
*****************/
#pragma once
#include "framework.h"
#include "W32examp.h"
#include <shlobj_core.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <processthreadsapi.h>

int GetDirRequestorLoad( WCHAR *FileName, size_t size );
BOOL FastCompare(WCHAR* directory1, WCHAR* directory2);
BOOL SlowCompare(WCHAR* directory1, WCHAR* directory2);
void printToScreen(WCHAR FormattedStr[261]);
void FindFiles(const std::wstring& directory, std::list<CFileListItem>& filesList);
void FindFilesSlow(const std::wstring& directory, std::list<CFileListItem>& filesList);
void CompareFiles(std::list<CFileListItem>& filesList, std::list<CFileListItem>& filesListDirectory2);
void CompareFilesSlow(std::list<CFileListItem>& filesList, std::list<CFileListItem>& filesListDirectory2);
// ULONG GetSize(WCHAR *filepath);
void DumpUniqueFiles(std::list<CFileListItem>& filesList);
void DumpUniqueFilesSlow(std::list<CFileListItem>& filesList);
DWORD Add(const std::wstring& filename, const ULONG& size, const DWORD& lt, const DWORD& ht, std::list<CFileListItem>& filesList);
DWORD FileInList(const std::wstring& filename, const ULONG& size, const DWORD& lt, const DWORD& ht, std::list<CFileListItem>& filesList);
DWORD AddSlow(const std::wstring& filename, const ULONG& h1, const ULONG& h2, const ULONG& h3, const ULONG& h4, std::list<CFileListItem>& filesList);
DWORD FileInListSlow(const std::wstring& filename, const ULONG& h1, const ULONG& h2, const ULONG& h3, const ULONG& h4, std::list<CFileListItem>& filesList);
long GetFileSize(std::wstring filename);
bool GetDirExist(std::wstring dirname);
unsigned int WINAPI TraverseDirectory1(void* parg);
unsigned int WINAPI TraverseDirectory2(void* parg);
unsigned int WINAPI TraverseDirectory1Slow(void* parg);
unsigned int WINAPI TraverseDirectory2Slow(void* parg);
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