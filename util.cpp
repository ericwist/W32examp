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
#include <psapi.h>
#include <process.h>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <Windows.h>

//remark this out to turn off threads
#define THREADED_CALLS

//For string conversions
//using convert_type = std::codecvt_utf8<wchar_t>;
//std::wstring_convert<convert_type, wchar_t> converter;

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

    // Reset cancellation flag at start
    gbCancelOperation = FALSE;

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
    if (wcscmp(directory1, directory2) == 0) {
        WCHAR msg1[260] = L"Directory 1 and Directory 2 are the same, no need to compare";
        printToScreen(msg1);
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
    if (hTraverseOne == NULL || hTraverseTwo == NULL) {
        return FALSE;
    }

    // Attempt to place the two threads on different logical processors for better parallelism.
    // If the machine has fewer than 2 processors, affinity will not be changed.
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    DWORD_PTR affinity1 = 1ULL << 0; // logical processor 0
    DWORD_PTR affinity2 = 1ULL << 1; // logical processor 1
    if (si.dwNumberOfProcessors < 2) {
        affinity2 = affinity1; // fallback: same processor if only one available
    }

    if (hTraverseOne != NULL) {
        SetThreadAffinityMask(hTraverseOne, affinity1);
    }
    if (hTraverseTwo != NULL) {
        SetThreadAffinityMask(hTraverseTwo, affinity2);
    }

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
        Idle(1);  // Process messages so Stop button clicks are received
    }
    switch (res) {
    case WAIT_OBJECT_0:
#if _DEBUG
        printToScreen(outth1);
#endif
        break;
    case (WAIT_OBJECT_0 + 1):
#if _DEBUG
        printToScreen(outth2);
#endif
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

    // Check if operation was cancelled
    if (gbCancelOperation) {
        WCHAR cancelMsg[260] = L"Operation cancelled by user";
        printToScreen(cancelMsg);
        FilesUnique.clear();
        FilesUniqueDirectory2.clear();
        gbCancelOperation = FALSE;
        return FALSE;
    }

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
    gbCancelOperation = FALSE;
    return TRUE;
}

//
//   FUNCTION: SlowCompare(WCHAR *directory1, WCHAR *directory2)
//
//   PURPOSE: To compare filenames and some properties
//
BOOL SlowCompare(WCHAR* directory1, WCHAR* directory2) {

    // Reset cancellation flag at start
    gbCancelOperation = FALSE;

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
    if (wcscmp(directory1, directory2) == 0) {
        WCHAR msg1[260] = L"Directory 1 and Directory 2 are the same, no need to compare";
        printToScreen(msg1);
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
    if (hTraverseOne == NULL || hTraverseTwo == NULL) {
        return FALSE;
    }

    // Attempt to place the two threads on different logical processors for better parallelism.
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    DWORD_PTR affinity1 = 1ULL << 0; // logical processor 0
    DWORD_PTR affinity2 = 1ULL << 1; // logical processor 1
    if (si.dwNumberOfProcessors < 2) {
        affinity2 = affinity1;
    }

    if (hTraverseOne != NULL) {
        SetThreadAffinityMask(hTraverseOne, affinity1);
    }
    if (hTraverseTwo != NULL) {
        SetThreadAffinityMask(hTraverseTwo, affinity2);
    }

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
        Idle(1);  // Process messages so Stop button clicks are received
    }

    // Close thread handles to prevent resource leak
    CloseHandle(hTraverseOne);
    CloseHandle(hTraverseTwo);

    switch (res) {
    case WAIT_OBJECT_0:
#if _DEBUG
        printToScreen(outth1);
#endif
        break;
    case (WAIT_OBJECT_0 + 1):
#if _DEBUG
        printToScreen(outth2);
#endif
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

    // Check if operation was cancelled
    if (gbCancelOperation) {
        WCHAR cancelMsg[260] = L"Operation cancelled by user";
        printToScreen(cancelMsg);
        FilesUnique.clear();
        FilesUniqueDirectory2.clear();
        gbCancelOperation = FALSE;
        return FALSE;
    }

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
    gbCancelOperation = FALSE;
    return TRUE;
    return TRUE;
}

//
//   FUNCTION: GetCurrentMemoryUsage()
//
//   PURPOSE: Get current process memory usage in bytes
//
SIZE_T GetCurrentMemoryUsage()
{
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        return pmc.WorkingSetSize;
    }
    return 0;
}

//
//   FUNCTION: CheckMemoryLimit(SIZE_T currentUsage)
//
//   PURPOSE: Check if memory usage exceeds limits
//   RETURNS: TRUE if within limits, FALSE if exceeded
//
BOOL CheckMemoryLimit(SIZE_T currentUsage)
{
    SIZE_T maxBytes = (SIZE_T)MAX_MEMORY_MB * 1024 * 1024;
    SIZE_T warningBytes = (SIZE_T)WARNING_MEMORY_MB * 1024 * 1024;
    
    if (currentUsage > maxBytes)
    {
        return FALSE;  // Exceeded limit
    }
    
    if (currentUsage > warningBytes)
    {
        WCHAR warning[260];
        swprintf_s(warning, 260, 
            L"WARNING: Memory usage high (%llu MB). Consider smaller directories.",
            currentUsage / (1024 * 1024));
        printToScreen(warning);
    }
    
    return TRUE;
}

//
//   FUNCTION: EstimateItemMemory()
//
//   PURPOSE: Estimate memory used by one CFileListItem
//   RETURNS: Approximate bytes per item
//
SIZE_T EstimateItemMemory(const std::wstring& filename)
{
    // Approximate: wstring overhead (48 bytes) + filename chars (2 bytes each) + CFileListItem structure
    return 48 + (filename.length() * 2) + 100;  // 100 bytes for object overhead
}

//
//   FUNCTION: FindFiles(const std::wstring& directory)
//
//   PURPOSE: Get unique files from given directory & its sub dirs. Store to the List.
//   NOTE: Now includes memory limit checking
//
void FindFiles(const std::wstring& directory, std::list<CFileListItem>& filesList)
{
    std::wstring tmp = directory + L"\\*";
    WIN32_FIND_DATAW file;

    HANDLE search_handle = FindFirstFileW(tmp.c_str(), &file);
    if (search_handle != INVALID_HANDLE_VALUE)
    {
        std::vector<std::wstring> directories;
        int fileCount = 0;
        do
        {
            // Check for cancellation
            if (gbCancelOperation)
            {
                FindClose(search_handle);
                return;
            }

            // Check memory periodically (every 1000 files) instead of every file
            if (++fileCount % 1000 == 0)
            {
                SIZE_T currentMemory = GetCurrentMemoryUsage();
                if (!CheckMemoryLimit(currentMemory))
                {
                    WCHAR errMsg[260];
                    swprintf_s(errMsg, 260, 
                        L"ERROR: Memory limit exceeded (%llu MB). Stopping directory scan.",
                        currentMemory / (1024 * 1024));
                    printToScreen(errMsg);
                    FindClose(search_handle);
                    return;
                }
            }

            if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if ((!lstrcmpW(file.cFileName, L".")) || (!lstrcmpW(file.cFileName, L"..")))
                    continue;
            }
            
            tmp = directory + L"\\" + std::wstring(file.cFileName);
            
            if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                directories.push_back(tmp);
            }
            else
            {
                ULONG sz = 0;
                if ((sz = GetFileSize(tmp)) > 0)
                {
                    // Skip duplicate check - just add all files, duplicates will be filtered during comparison
                    Add(file.cFileName, sz, file.ftLastWriteTime.dwLowDateTime, 
                        file.ftLastWriteTime.dwHighDateTime, filesList);
                }
            }
        } while (FindNextFileW(search_handle, &file));

        FindClose(search_handle);
        
        // Recursively search subdirectories
        for (std::vector<std::wstring>::iterator iter = directories.begin(), end = directories.end(); 
             iter != end; ++iter)
        {
            // Check memory periodically (every 10 directories)
            static int recursiveCallCount = 0;
            if (++recursiveCallCount % 10 == 0)
            {
                SIZE_T currentMemory = GetCurrentMemoryUsage();
                if (!CheckMemoryLimit(currentMemory))
                {
                    WCHAR errMsg[260];
                    swprintf_s(errMsg, 260, L"Memory limit reached. Stopped at directory: %s", iter->c_str());
                    printToScreen(errMsg);
                    return;
                }
            }

            FindFiles(*iter, filesList);
        }
    }
}

//
//   FUNCTION: FindFilesSlow(const std::wstring& directory)
//
//   PURPOSE: Get unique files from given directory & its sub dirs. Use file Hash. Memory checked.
//
void FindFilesSlow(const std::wstring& directory, std::list<CFileListItem>& filesList)
{
	std::wstring tmp = directory + L"\\*";
	WIN32_FIND_DATAW file;
	PMSIFILEHASHINFO pFileHash = (PMSIFILEHASHINFO)malloc(sizeof(MSIFILEHASHINFO));

	if (!pFileHash)
	{
		WCHAR err[260]= L"ERROR: Failed to allocate memory for file hash.";
		printToScreen(err);
		return;
	}

	WCHAR err[260];
	HANDLE search_handle = FindFirstFileW(tmp.c_str(), &file);

	if (search_handle != INVALID_HANDLE_VALUE)
	{
		std::vector<std::wstring> directories;
		int fileCount = 0;
		do
		{
			// Check for cancellation
			if (gbCancelOperation)
			{
				FindClose(search_handle);
				free(pFileHash);
				return;
			}

			// Check memory periodically (every 1000 files) instead of every file
			if (++fileCount % 1000 == 0)
			{
				SIZE_T currentMemory = GetCurrentMemoryUsage();
				if (!CheckMemoryLimit(currentMemory))
				{
					swprintf_s(err, 260, 
						L"ERROR: Memory limit exceeded (%llu MB). Stopping scan.",
						currentMemory / (1024 * 1024));
					printToScreen(err);
					FindClose(search_handle);
					free(pFileHash);
					return;
				}
			}

            if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if ((!lstrcmpW(file.cFileName, L".")) || (!lstrcmpW(file.cFileName, L"..")))
                    continue;
            }
            
            tmp = directory + L"\\" + std::wstring(file.cFileName);
            
            if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                directories.push_back(tmp);
            }
            else
            {
                memset(pFileHash, 0, sizeof(MSIFILEHASHINFO));
                pFileHash->dwFileHashInfoSize = sizeof(MSIFILEHASHINFO);
                UINT res = MsiGetFileHashW(tmp.c_str(), 0, pFileHash);

                if (res == ERROR_SUCCESS)
                {
                    // Skip duplicate check - just add all files, duplicates will be filtered during comparison
                    AddSlow(file.cFileName, pFileHash->dwData[0], pFileHash->dwData[1], 
                           pFileHash->dwData[2], pFileHash->dwData[3], filesList);
                }
                else if (res == ERROR_FILE_NOT_FOUND)
                {
                    swprintf_s(err, 260, L"FILE[%s] NOT FOUND...", file.cFileName);
                    printToScreen(err);
                }
                else if (res == ERROR_ACCESS_DENIED)
                {
                    swprintf_s(err, 260, L"FILE[%s] ACCESS DENIED...", file.cFileName);
                    printToScreen(err);
                }
            }
        } while (FindNextFileW(search_handle, &file));

        FindClose(search_handle);
        
        // Recursively search subdirectories
        for (std::vector<std::wstring>::iterator iter = directories.begin(), end = directories.end(); 
             iter != end; ++iter)
        {
            // Check memory periodically
            static int recursiveCallCount = 0;
            if (++recursiveCallCount % 10 == 0)
            {
                SIZE_T currentMemory = GetCurrentMemoryUsage();
                if (!CheckMemoryLimit(currentMemory))
                {
                    swprintf_s(err, 260, L"Memory limit reached. Stopped at: %s", iter->c_str());
                    printToScreen(err);
                    break;
                }
            }

            FindFilesSlow(*iter, filesList);
        }
    }

    free(pFileHash);
}

void CompareFiles(std::list<CFileListItem>& filesList, std::list<CFileListItem>& filesListDirectory2) 
{
    WCHAR statusMsg[260];
    swprintf_s(statusMsg, 260, L"Building index of %zu files from directory 2...", filesListDirectory2.size());
    printToScreen(statusMsg);
    
    auto startBuild = std::chrono::high_resolution_clock::now();
            
    std::unordered_set<std::wstring> directory2Keys;
    
    for (const auto& item : filesListDirectory2) 
    {
        // Create a composite key: filename|size|lowDateTime|highDateTime
        std::wstring key = item.m_Filename + L"|" + 
                          std::to_wstring(item.m_Size) + L"|" +
                          std::to_wstring(item.m_dwLowDateTime) + L"|" +
                          std::to_wstring(item.m_dwHighDateTime);
        directory2Keys.insert(key);
    }
    
    // Now remove matches from filesList in O(n) time
    for (auto i = filesList.begin(); i != filesList.end();) 
    {
        std::wstring key = i->m_Filename + L"|" + 
                          std::to_wstring(i->m_Size) + L"|" +
                          std::to_wstring(i->m_dwLowDateTime) + L"|" +
                          std::to_wstring(i->m_dwHighDateTime);
        
        if (directory2Keys.find(key) != directory2Keys.end()) 
        {
            // Found in directory2, remove from filesList
            i = filesList.erase(i);
        } 
        else 
        {
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
    // Build a set of unique keys from directory2 for O(1) lookup
    std::unordered_set<std::wstring> directory2Keys;
    
    for (const auto& item : filesListDirectory2) 
    {
        // Create a composite key: filename|hash values
        std::wstring key = item.m_Filename + L"|" + 
                          std::to_wstring(item.m_dwHash[0]) + L"|" +
                          std::to_wstring(item.m_dwHash[1]) + L"|" +
                          std::to_wstring(item.m_dwHash[2]) + L"|" +
                          std::to_wstring(item.m_dwHash[3]);
        directory2Keys.insert(key);
    }
    
    // Now remove matches from filesList in O(n) time
    for (auto i = filesList.begin(); i != filesList.end();) 
    {
        std::wstring key = i->m_Filename + L"|" + 
                          std::to_wstring(i->m_dwHash[0]) + L"|" +
                          std::to_wstring(i->m_dwHash[1]) + L"|" +
                          std::to_wstring(i->m_dwHash[2]) + L"|" +
                          std::to_wstring(i->m_dwHash[3]);
        
        if (directory2Keys.find(key) != directory2Keys.end()) 
        {
            // Found in directory2, remove from filesList
            i = filesList.erase(i);
        } 
        else 
        {
            ++i;
        }
    }
}

void DumpUniqueFiles(std::list<CFileListItem>& filesList) {
    WCHAR out[260] = L"===================================== FAST COMPARE DONE============================================";
    WCHAR outend[260] = L"=================================================================================================";
    WCHAR endres[260];
#if _DEBUG
    printToScreen(out);
#endif
    if (isCheckShowFiles == TRUE) {
        int itemCount = 0;
        for (auto i = filesList.begin(); i != filesList.end(); ++i)
        {
            // Check for cancellation and pump messages every 100 items
            if (++itemCount % 100 == 0)
            {
                Idle(1);  // Process messages to allow Stop button to work
                if (gbCancelOperation)
                {
                    WCHAR cancelMsg[260] = L"Cancelled while displaying files";
                    printToScreen(cancelMsg);
                    return;
                }
            }

            swprintf_s(out, 260, L"FILE[%s]::SIZE[%lu]::LASTWRITE[%lu.%lu]", i->m_Filename.c_str(), i->m_Size, i->m_dwLowDateTime, i->m_dwHighDateTime);
            printToScreen(out);
        }
    }
    swprintf(endres, 260, L"FAST COMPARE END RESULT [%zu] UNIQUE FILES**", filesList.size());
    printToScreen(endres);
#if _DEBUG
    printToScreen(outend);
#endif
}

void DumpUniqueFilesSlow(std::list<CFileListItem>& filesList) {
    WCHAR out[260] = L"=====================================SLOW COMPARE DONE============================================";
    WCHAR outend[260] = L"=================================================================================================";
    WCHAR endres[260];
    printToScreen(out);
    if (isCheckShowFiles == TRUE) {
        int itemCount = 0;
        for (auto i = filesList.begin(); i != filesList.end(); ++i)
        {
            // Check for cancellation and pump messages every 100 items
            if (++itemCount % 100 == 0)
            {
                Idle(1);  // Process messages to allow Stop button to work
                if (gbCancelOperation)
                {
                    WCHAR cancelMsg[260] = L"Cancelled while displaying files";
                    printToScreen(cancelMsg);
                    return;
                }
            }

            swprintf_s(out, 260, L"FILE[%s]::H1[%lu]::H2[%lu]::H3[%lu]::H4[%lu]", i->m_Filename.c_str(), i->m_dwHash[0], i->m_dwHash[1], i->m_dwHash[2], i->m_dwHash[3]);
            printToScreen(out);
        }
    }
    swprintf(endres, 260, L"**SLOW COMPARE END RESULT [%zu] UNIQUE FILES**", filesList.size());
    printToScreen(endres);
#if _DEBUG
    printToScreen(outend);
#endif
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
    std::string converted_str = WstringToUtf8(filename);

    struct stat stat_buf;
    int rc = stat(converted_str.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

bool GetDirExist(std::wstring dirname) {

    std::wstring dw = std::wstring(dirname);
    std::string converted_dir = WstringToUtf8(dw);

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

// Utility function to convert UTF-8 std::string to std::wstring (UTF-16)
std::wstring Utf8ToWstring(const std::string& str)
{
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// Utility function to convert std::wstring (UTF-16) to UTF-8 std::string
std::string WstringToUtf8(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}
