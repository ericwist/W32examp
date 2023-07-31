/*****************
* util.h
* Some Common untility functions
* for example code
* Author: Eric Wistrand
* Jul 25, 2023
*****************/

#include "util.h"
#include <locale>
#include <codecvt>
#include <msi.h>
#include <sys/types.h>
#include <sys/stat.h>
//remark this out to turn off threads
#define THREADED_CALLS

//For string conversions
using convert_type = std::codecvt_utf8<wchar_t>;
std::wstring_convert<convert_type, wchar_t> converter;

//Declare my list of file objects globally in the file only
std::list<CFileListItem> FilesUnique;
std::list<CFileListItem> FilesUniqueDirectory2;
//
//   FUNCTION: GetDirRequestorLoad( WCHAR *Path, size_t size )
//
//   PURPOSE: To bring up the directory browser
//
int GetDirRequestorLoad( WCHAR *Path, size_t size ) {

    BROWSEINFO bi;
    /* Set Open File Name Structure. */
    memset( &bi, 0, sizeof( bi ) );
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

//
//   FUNCTION: FastCompare(WCHAR *directory1, WCHAR *directory2)
//
//   PURPOSE: To compare filenames and some properties
//
BOOL FastCompare(WCHAR *directory1, WCHAR *directory2) {

    WCHAR out[260];
    if (!GetDirExist(directory1)) {
        swprintf_s(out, 260, L"Directory does NOT exist: %s", directory1);
        printToScreen(out);
        return FALSE;
    }
    if (!GetDirExist(directory2)) {
        swprintf_s(out, 260, L"Directory does NOT exist: %s", directory2);
        printToScreen(out);
        return FALSE;
    }
    FilesUnique.clear();
    //START THREADS
    //start time
    DWORD totaltime = 0;
    DWORD timestart = GetTickCount(); 
#ifdef THREADED_CALLS
    DWORD res;
    WCHAR* param = directory1;
    WCHAR* pparam = param;
    HANDLE hTraverseOne = (HANDLE)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TraverseDirectory1, (void*)pparam, CREATE_SUSPENDED, NULL);

    param = directory2;
    pparam = param;
    HANDLE hTraverseTwo = (HANDLE)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TraverseDirectory2, (void*)pparam, CREATE_SUSPENDED, NULL);

    HANDLE hThreads[2];
    hThreads[0] = hTraverseOne;
    hThreads[1] = hTraverseTwo;
    ResumeThread(hTraverseOne);
    ResumeThread(hTraverseTwo);

    WCHAR outth1a[260] = L"THREADS ONE ABANDONED========================================";
    WCHAR outth2a[260] = L"THREADS TWO ABANDONED========================================";
    WCHAR outth1[260] =  L"THREADS FINISHED JOINED to 0=================================";
    WCHAR outth2[260] =  L"THREADS FINISHED JOINED to 1=================================";
    WCHAR outerr[260] =  L"FATAL THREAD ERROR===========================================";
    WCHAR outwait[260] = L"*THREADS RUNNING=============================================";
    WCHAR outfin[260] =  L"*THREADS FINISHED============================================";
    //join threads - bWaitAll=TRUE then it will return WAIT_OBJECT_0(0) to indicate both threads where joined and completed
    while (WAIT_TIMEOUT == (res = WaitForMultipleObjects(2, hThreads, TRUE, 1)))
    {
        //Idle(1);
    }
    switch (res) {
    case WAIT_OBJECT_0:
        printToScreen(outth1);
        break;
    case (WAIT_OBJECT_0 + 1):
        printToScreen(outth2);
        break;
    case WAIT_FAILED:
        printToScreen(outerr);
        return FALSE;
    case WAIT_ABANDONED_0:
        printToScreen(outth1a);
        return FALSE;
    case (WAIT_ABANDONED_0 + 1):
        printToScreen(outth2a);
        return FALSE;
    }
    //END THREADS
#else
    FindFiles(directory1, FilesUnique);
    FindFiles(directory2, FilesUniqueDirectory2);
#endif
    //compare two unique lists and crate file list in FilesUnique
    CompareFiles(FilesUnique, FilesUniqueDirectory2);
    FilesUnique.splice(FilesUnique.end(), FilesUniqueDirectory2);
    DumpUniqueFiles(FilesUnique);
    FilesUnique.clear();
    //get end time
    totaltime = GetTickCount() - timestart;
    WCHAR outtime[260] = L"";
    swprintf(outtime, 260, L"TOTAL MILLISECONDS TIME FOR FAST OPERATION IS: %lu", totaltime);
    printToScreen(outtime);
    return TRUE;
}

//
//   FUNCTION: SlowCompare(WCHAR *directory1, WCHAR *directory2)
//
//   PURPOSE: To compare filenames and some properties
//
BOOL SlowCompare(WCHAR* directory1, WCHAR* directory2) {

    WCHAR out[260];
    if (!GetDirExist(directory1)) {
        swprintf_s(out, 260, L"Directory does NOT exist: %s", directory1);
        printToScreen(out);
        return FALSE;
    }
    if (!GetDirExist(directory2)) {
        swprintf_s(out, 260, L"Directory does NOT exist: %s", directory2);
        printToScreen(out);
        return FALSE;
    }
    FilesUnique.clear();
    DWORD totaltime = 0;
    DWORD timestart = GetTickCount();
#ifdef THREADED_CALLS
    //threads
    DWORD res;
    WCHAR* param = directory1;
    WCHAR* pparam = param;
    HANDLE hTraverseOne = (HANDLE)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TraverseDirectory1Slow, (void*)pparam, CREATE_SUSPENDED, NULL);

    param = directory2;
    pparam = param;
    HANDLE hTraverseTwo = (HANDLE)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)TraverseDirectory2Slow, (void*)pparam, CREATE_SUSPENDED, NULL);

    HANDLE hThreads[2];
    hThreads[0] = hTraverseOne;
    hThreads[1] = hTraverseTwo;
    ResumeThread(hTraverseOne);
    ResumeThread(hTraverseTwo);

    WCHAR outth1a[260] = L"THREADS ONE ABANDONED========================================";
    WCHAR outth2a[260] = L"THREADS TWO ABANDONED========================================";
    WCHAR outth1[260] =  L"THREADS FINISHED JOINED to 0=================================";
    WCHAR outth2[260] =  L"THREADS FINISHED JOINED to 1=================================";
    WCHAR outerr[260] =  L"FATAL THREAD ERROR===========================================";
    WCHAR outwait[260] = L"*THREADS RUNNING=============================================";
    WCHAR outfin[260] =  L"*THREADS FINISHED============================================";
    //join threads - bWaitAll=TRUE then it will return WAIT_OBJECT_0(0) to indicate both threads where joined and completed
    while (WAIT_TIMEOUT == (res = WaitForMultipleObjects(2, hThreads, TRUE, 1)))
    {
        //Idle(1);
    }
    switch (res) {
    case WAIT_OBJECT_0:
        printToScreen(outth1);
        break;
    case (WAIT_OBJECT_0 + 1):
        printToScreen(outth2);
        break;
    case WAIT_FAILED:
        printToScreen(outerr);
        return FALSE;
    case WAIT_ABANDONED_0:
        printToScreen(outth1a);
        return FALSE;
    case (WAIT_ABANDONED_0 + 1):
        printToScreen(outth2a);
        return FALSE;
    }
#else
    FindFilesSlow(directory1, FilesUnique);
    FindFilesSlow(directory2, FilesUniqueDirectory2); 
#endif
    //compare two unique lists and crate file list in FilesUnique
    CompareFilesSlow(FilesUnique, FilesUniqueDirectory2);
    FilesUnique.splice(FilesUnique.end(), FilesUniqueDirectory2);
    DumpUniqueFilesSlow(FilesUnique);
    FilesUnique.clear();
    //get end time
    totaltime = GetTickCount() - timestart;
    WCHAR outtime[260] = L"";
    swprintf(outtime, 260, L"TOTAL MILLISECONDS TIME FOR SLOW OPERATION IS: %lu", totaltime);
    printToScreen(outtime);
    return TRUE;
}

//
//   FUNCTION: FindFiles(const std::wstring& directory)
//
//   PURPOSE: Get unique files from given directory & its sub dirs. Store to the List.
//
void FindFiles(const std::wstring& directory, std::list<CFileListItem>& filesList)
{
    std::wstring tmp = directory + L"\\*";
    WIN32_FIND_DATAW file;
 
    HANDLE search_handle = FindFirstFileW(tmp.c_str(), &file);
    if (search_handle != INVALID_HANDLE_VALUE)
    {
        std::vector<std::wstring> directories;
        do
        {
            if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if ((!lstrcmpW(file.cFileName, L".")) || (!lstrcmpW(file.cFileName, L"..")))
                    continue;
            }
            tmp = directory + L"\\" + std::wstring(file.cFileName);
            if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                directories.push_back(tmp);
            }
            else
            {
                ULONG sz = 0;
                if ((sz=GetFileSize(tmp)) > 0) {
                    if (FileInList(file.cFileName, sz, file.ftLastWriteTime.dwLowDateTime, file.ftLastWriteTime.dwHighDateTime, filesList) == ERROR_NOT_FOUND) {
                        Add(file.cFileName, sz, file.ftLastWriteTime.dwLowDateTime, file.ftLastWriteTime.dwHighDateTime, filesList);
                    }
                    
                }
            }
        } while (FindNextFileW(search_handle, &file));

        FindClose(search_handle);
        for (std::vector<std::wstring>::iterator iter = directories.begin(), end = directories.end(); iter != end; ++iter)
            FindFiles(*iter, filesList);
    }
}

//
//   FUNCTION: FindFilesSlow(const std::wstring& directory)
//
//   PURPOSE: Get unique files from given directory & its sub dirs. Use file Hash this time.
//
void FindFilesSlow(const std::wstring& directory, std::list<CFileListItem>& filesList)
{
    std::wstring tmp = directory + L"\\*";
    WIN32_FIND_DATAW file;
    PMSIFILEHASHINFO pFileHash = (PMSIFILEHASHINFO)malloc(sizeof(MSIFILEHASHINFO));
    
    HANDLE search_handle = FindFirstFileW(tmp.c_str(), &file);
    if (search_handle != INVALID_HANDLE_VALUE)
    {
        std::vector<std::wstring> directories;
        do
        {
            if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if ((!lstrcmpW(file.cFileName, L".")) || (!lstrcmpW(file.cFileName, L"..")))
                    continue;
            }
            tmp = directory + L"\\" + std::wstring(file.cFileName);
            if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                directories.push_back(tmp);
            }
            else
            {
                memset(pFileHash,0, sizeof(MSIFILEHASHINFO));
                pFileHash->dwFileHashInfoSize = sizeof(MSIFILEHASHINFO);
                if (MsiGetFileHashW(tmp.c_str(), 0, pFileHash) == ERROR_SUCCESS) {
                    if (FileInListSlow(file.cFileName, pFileHash->dwData[0], pFileHash->dwData[2], pFileHash->dwData[3], pFileHash->dwData[3], filesList) == ERROR_NOT_FOUND) {
                        AddSlow(file.cFileName, pFileHash->dwData[0], pFileHash->dwData[2], pFileHash->dwData[3], pFileHash->dwData[3], filesList);
                    }
                }
            }
        } while (FindNextFileW(search_handle, &file));

        FindClose(search_handle);
        free(pFileHash);
        for (std::vector<std::wstring>::iterator iter = directories.begin(), end = directories.end(); iter != end; ++iter)
            FindFilesSlow(*iter, filesList);
    }
}

//
//   FUNCTION: CompareFiles(const std::wstring& directory)
//
//   PURPOSE: Compare list of unique files from given directorys, remove matches to leave uniue in filesList.
//
void CompareFiles(std::list<CFileListItem>& filesList, std::list<CFileListItem>& filesListDirectory2) {
    std::list<CFileListItem>::iterator i;
    std::list<CFileListItem>::iterator j;
    bool found;
    //make filesList the unique list
    for (i = filesList.begin(); i != filesList.end();) {
        found = FALSE;
        for (j = filesListDirectory2.begin(); j != filesListDirectory2.end(); ++j) {
            if (i->m_Filename == j->m_Filename && i->m_Size == j->m_Size) {
                i  = filesList.erase(i);
                found = TRUE;
                break;
            }
        }
        if (!found) {
            ++i;
        }
    }
}

//
//   FUNCTION: CompareFiles(const std::wstring& directory)
//
//   PURPOSE: Compare list of unique files from given directorys, remove matches to leave uniue in filesList.
//
void CompareFilesSlow(std::list<CFileListItem>& filesList, std::list<CFileListItem>& filesListDirectory2) {
    std::list<CFileListItem>::iterator i;
    std::list<CFileListItem>::iterator j;
    bool found;
    //make filesList the unique list
    for (i = filesList.begin(); i != filesList.end();) {
        found = FALSE;
        for (j = filesListDirectory2.begin(); j != filesListDirectory2.end(); ++j) {
            if (i->m_Filename == j->m_Filename && i->m_dwHash[0] == j->m_dwHash[0] && i->m_dwHash[1] == j->m_dwHash[1] && i->m_dwHash[2] == j->m_dwHash[2] && i->m_dwHash[3] == j->m_dwHash[3]) {
                i = filesList.erase(i);
                found = TRUE;
                break;
            }
        }
        if (!found) {
            ++i;
        }
    }
}

void DumpUniqueFiles(std::list<CFileListItem>& filesList) {
    WCHAR out[260] = L"===================================== FAST COMPARE DONE============================================";
    WCHAR outend[260] = L"=================================================================================================";
    WCHAR endres[260];
    printToScreen(out);
    if (isCheckShowFiles == TRUE) {
        for (auto i = filesList.begin(); i != filesList.end(); ++i)
        {
            swprintf_s(out, 260, L"FILE[%s]::SIZE[%lu]::LASTWRITE[%lu.%lu]", i->m_Filename.c_str(), i->m_Size, i->m_dwLowDateTime, i->m_dwHighDateTime);
            printToScreen(out);
        }
    }
    swprintf(endres, 260, L"**FAST COMPARE END RESULT [%zu] UNIQUE FILES**", filesList.size());
    printToScreen(endres);
    printToScreen(outend);
}

void DumpUniqueFilesSlow(std::list<CFileListItem>& filesList) {
    WCHAR out[260] = L"=====================================SLOW COMPARE DONE============================================";
    WCHAR outend[260] = L"=================================================================================================";
    WCHAR endres[260];
    printToScreen(out);
    if (isCheckShowFiles == TRUE) {
        for (auto i = filesList.begin(); i != filesList.end(); ++i)
        {
            swprintf_s(out, 260, L"FILE[%s]::H1[%lu]::H2[%lu]::H3[%lu]::H4[%lu]", i->m_Filename.c_str(), i->m_dwHash[0], i->m_dwHash[1], i->m_dwHash[2], i->m_dwHash[3]);
            printToScreen(out);
        }
    }
    swprintf(endres, 260, L"**SLOW COMPARE END RESULT [%zu] UNIQUE FILES**", filesList.size());
    printToScreen(endres);
    printToScreen(outend);
}


void printToScreen(WCHAR FormattedStr[261])
{
    try
    {
        if (ghListBox != (HWND)NULL)
        {
            //Force List Box to bottom
            int max;
            int min;
            GetScrollRange(ghListBox, SB_VERT, &min, &max);
            SetScrollPos(ghListBox, SB_VERT, max, TRUE);
            SendMessage(ghListBox, WM_VSCROLL, SB_BOTTOM, 0);
            int count = (int)SendMessage(ghListBox, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
            SendMessage(ghListBox, LB_SETCARETINDEX, (WPARAM)(count - 1), (LPARAM)0);
            UpdateWindow(ghListBox);

            int idx = (int)SendMessage(ghListBox, LB_GETCARETINDEX, (WPARAM)0, (LPARAM)0);
            if (idx > 2000)
            {
                //clear list box
                SendMessage(ghListBox, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
            }

            int pos = (int)SendMessage(ghListBox, LB_ADDSTRING, 0, (LPARAM)FormattedStr);
            SendMessage(ghListBox, LB_SETITEMDATA, pos, (LPARAM)0);
            SendMessage(ghListBox, LB_SETCURSEL, pos, (LPARAM)0);
        }
    }
    catch (...)
    {
    }
}

DWORD Add(const std::wstring& filename, const ULONG& size, const DWORD& lt, const DWORD& ht, std::list<CFileListItem>& filesList)
{
    CFileListItem fileData;
    fileData.SetFilename(filename);
    fileData.SetSize(size);
    fileData.SetLowDateTime(lt);
    fileData.SetHighDateTime(ht);
    // push file onto list
    filesList.push_back(fileData);

    return(0);
}

DWORD AddSlow(const std::wstring& filename, const ULONG& h1, const ULONG& h2, const ULONG& h3, const ULONG& h4, std::list<CFileListItem>& filesList)
{
    CFileListItem fileData;
    fileData.SetFilename(filename);
    fileData.SetFileHash(h1, h2, h3, h4);
    // push file onto list
    filesList.push_back(fileData);

    return(0);
}

DWORD FileInList(const std::wstring& filename, const ULONG& size, const DWORD& lt, const DWORD& ht, std::list<CFileListItem> &filesList)
{
    std::list<CFileListItem>::iterator i;
    for (i = filesList.begin(); i != filesList.end(); ++i)
    {
        if (i->m_Filename == filename && i->m_Size == size && i->m_dwLowDateTime == lt && i->m_dwHighDateTime == ht)
        {
            return(ERROR_SUCCESS);
        }
    }
    return(ERROR_NOT_FOUND);
}

DWORD FileInListSlow(const std::wstring& filename, const ULONG& h1, const ULONG& h2, const ULONG& h3, const ULONG& h4, std::list<CFileListItem>& filesList)
{
    std::list<CFileListItem>::iterator i;
    for (i = filesList.begin(); i != filesList.end(); ++i)
    {
        if (i->m_Filename == filename && i->m_dwHash[0] == h1 && i->m_dwHash[1] == h2 && i->m_dwHash[2] == h3 && i->m_dwHash[3] == h4)
        {
            return(ERROR_SUCCESS);
        }
    }
    return(ERROR_NOT_FOUND);
}


long GetFileSize(std::wstring filename)
{
    std::string converted_str = converter.to_bytes(filename);

    struct stat stat_buf;
    int rc = stat(converted_str.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

bool GetDirExist(std::wstring dirname) {

    std::wstring dw = std::wstring(dirname);
    std::string converted_dir = converter.to_bytes(dw);

    struct stat s;
    int err = stat(converted_dir.c_str(), &s);
    if (-1 == err) {
        if (ENOENT == errno) {
            /* does not exist */
            return FALSE;
        }
        else {
            // bad error just exit
            perror("stat");
            exit(1);
        }
    }
    else {
        return TRUE;
#if 0
        if (S_ISDIR(s.st_mode)) {
            /* it's a dir */
            return TRUE;
        }
        else {
            /* exists but is no dir */
            return FALSE;
        }
#endif
    }
}

// Threads
// TraverseDirectories - Thread
//
unsigned int WINAPI TraverseDirectory1(void* parg)
{
    WCHAR* dir = (WCHAR*) parg;
    FindFiles(dir, FilesUnique);
    return 0;
}

unsigned int WINAPI TraverseDirectory2(void* parg)
{
    WCHAR* dir = (WCHAR*)parg;
    FindFiles(dir, FilesUniqueDirectory2);
    return 0;
}

unsigned int WINAPI TraverseDirectory1Slow(void* parg)
{
    WCHAR* dir = (WCHAR*)parg;
    FindFilesSlow(dir, FilesUnique);
    return 0;
}

unsigned int WINAPI TraverseDirectory2Slow(void* parg)
{
    WCHAR* dir = (WCHAR*)parg;
    FindFilesSlow(dir, FilesUniqueDirectory2);
    return 0;
}
