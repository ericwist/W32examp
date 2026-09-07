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
#include <filesystem>
#include <algorithm>
#include <cctype>

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
        : m_Filename(fn), m_Size(sz), m_dwLowDateTime(lt), m_dwHighDateTime(ht) {
    }

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




bool isAbsolutePath(const std::wstring& path);
bool isRootPath(const std::wstring& path);
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

inline std::wstring RemoveSpacesAndNonPrintable(const std::wstring& str) {
    size_t start = str.find_first_not_of(L" \t\n\r\f\v");
    if (start == std::wstring::npos) {
        return L"";  // String is all whitespace
    }
    size_t end = str.find_last_not_of(L" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}