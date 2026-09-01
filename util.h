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
#include <cstdint>

std::string WstringToUtf8(const std::wstring& wstr);
std::wstring Utf8ToWstring(const std::string& str);
int FastCompare(const std::wstring& directory1, const std::wstring& directory2);
int SlowCompare(const std::wstring& directory1, const std::wstring& directory2);
void FindFiles(const std::wstring& directory, std::list<CFileListItem>& filesList);
void FindFilesSlow(const std::wstring& directory, std::list<CFileListItem>& filesList);
void CompareFiles(std::list<CFileListItem>& filesList, std::list<CFileListItem>& filesListDirectory2);
void CompareFilesSlow(std::list<CFileListItem>& filesList, std::list<CFileListItem>& filesListDirectory2);
void DumpUniqueFiles(std::list<CFileListItem>& filesList);
void DumpUniqueFilesSlow(std::list<CFileListItem>& filesList);
uint32_t Add(const std::wstring& filename, const uint32_t& size, const uint32_t& lt, const uint32_t& ht, std::list<CFileListItem>& filesList);
uint32_t FileInList(const std::wstring& filename, const uint32_t& size, const uint32_t& lt, const uint32_t& ht, std::list<CFileListItem>& filesList);
uint32_t AddSlow(const std::wstring& filename, const uint32_t& h1, const uint32_t& h2, const uint32_t& h3, const uint32_t& h4, std::list<CFileListItem>& filesList);
uint32_t FileInListSlow(const std::wstring& filename, const uint32_t& h1, const uint32_t& h2, const uint32_t& h3, const uint32_t& h4, std::list<CFileListItem>& filesList);
long GetFileSize(std::wstring filename);
bool GetDirExist(std::wstring dirname);
extern volatile int gbCancelOperation;  // Flag to cancel current operation
