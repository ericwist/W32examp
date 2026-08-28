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
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <processthreadsapi.h>

std::string WstringToUtf8(const std::wstring& wstr);
std::wstring Utf8ToWstring(const std::string& str);
BOOL FastCompare(const std::wstring& directory1, const std::wstring& directory2);
BOOL SlowCompare(const std::wstring& directory1, const std::wstring& directory2);
void FindFiles(const std::wstring& directory, std::list<CFileListItem>& filesList);
void FindFilesSlow(const std::wstring& directory, std::list<CFileListItem>& filesList);
void CompareFiles(std::list<CFileListItem>& filesList, std::list<CFileListItem>& filesListDirectory2);
void CompareFilesSlow(std::list<CFileListItem>& filesList, std::list<CFileListItem>& filesListDirectory2);
void DumpUniqueFiles(std::list<CFileListItem>& filesList);
void DumpUniqueFilesSlow(std::list<CFileListItem>& filesList);
DWORD Add(const std::wstring& filename, const ULONG& size, const DWORD& lt, const DWORD& ht, std::list<CFileListItem>& filesList);
DWORD FileInList(const std::wstring& filename, const ULONG& size, const DWORD& lt, const DWORD& ht, std::list<CFileListItem>& filesList);
DWORD AddSlow(const std::wstring& filename, const ULONG& h1, const ULONG& h2, const ULONG& h3, const ULONG& h4, std::list<CFileListItem>& filesList);
DWORD FileInListSlow(const std::wstring& filename, const ULONG& h1, const ULONG& h2, const ULONG& h3, const ULONG& h4, std::list<CFileListItem>& filesList);
long GetFileSize(std::wstring filename);
bool GetDirExist(std::wstring dirname);
extern volatile BOOL gbCancelOperation;  // Flag to cancel current operation
