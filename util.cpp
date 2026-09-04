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
#include <thread>
#include <fstream>
#include <mutex>
#include <queue>
#include <cstdint> 
#ifdef __linux__
#include <dirent.h>
#endif
#ifdef __APPLE__
#include <dirent.h>
#endif

//remark this out to turn off threads
#define THREADED_CALLS

//Declare my list of file objects globally in the file only
std::list<CFileListItem> FilesUnique;
std::list<CFileListItem> FilesUniqueDirectory2;

class ThreadSafeLogger {
private:
    std::wofstream logFile;
    std::mutex logMutex;
    std::queue<std::wstring> messageQueue;
    std::string filename;
    bool isOpened;

public:
    ThreadSafeLogger(const std::string& fname) : filename(fname), isOpened(false) {
        std::lock_guard<std::mutex> lock(logMutex);
        logFile.open(filename, std::ios::out | std::ios::trunc);
        isOpened = logFile.is_open();
    }

    ~ThreadSafeLogger() {
        close();
    }

    void log(const std::wstring& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        if (isOpened && logFile.is_open()) {
            logFile << message << std::endl;
            logFile.flush();
        }
    }

    void close() {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile << L"end close!" << std::endl;
            logFile.close();
        }
        isOpened = false;
    }

    // Reopen the logger for a new compare run
    void reopen() {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile.close();
        }
        logFile.open(filename, std::ios::out | std::ios::trunc);
        isOpened = logFile.is_open();
    }

    bool isOpen() const {
        return isOpened && logFile.is_open();
    }
};

// Global thread-safe logger instance
ThreadSafeLogger g_logger("compare.log");

//
// FUNCTION: SetThreadAffinity(std::thread& thread, unsigned int coreId)
//
// PURPOSE: Set thread affinity to run on a specific core (platform-agnostic)
//

void SetThreadAffinity(std::thread& thread, unsigned int coreId)
{
#ifdef _WIN32
    // Windows implementation
    DWORD_PTR affinityMask = 1ULL << coreId;
    if (!SetThreadAffinityMask(thread.native_handle(), affinityMask))
    {
        g_logger.log(L"WARNING: Failed to set thread affinity for core " + std::to_wstring(coreId));
    }
#elif defined(__linux__)
    // Linux implementation using pthread
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(coreId, &cpuset);
    int result = pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset);
    if (result != 0)
    {
        g_logger.log(L"WARNING: Failed to set thread affinity for core " + std::to_wstring(coreId));
    }
#elif defined(__APPLE__)
    // macOS implementation using thread_policy
    // macOS doesn't provide direct cpu affinity API like Linux/Windows
    // This is a best-effort approach using thread_policy_set
    g_logger.log(L"NOTE: macOS does not support direct thread affinity");
#else
    // Other platforms
    g_logger.log(L"NOTE: Thread affinity not supported on this platform");
#endif
}

//
// FUNCTION: GetCoreCount()
//
// PURPOSE: Get the number of available CPU cores
//
unsigned int GetCoreCount()
{
    return std::thread::hardware_concurrency();
}

//
//   FUNCTION: FastCompare(WCHAR *directory1, WCHAR *directory2)
//
//   PURPOSE: To compare filenames and some properties
//
int FastCompare(const std::wstring& directory1, const std::wstring& directory2) {

    // Reset cancellation flag at start
    gbCancelOperation = 0;

    if (!g_logger.isOpen()) {
		g_logger.reopen();
    }

    if (!g_logger.isOpen()) {
        g_printer.print(L"Failed to open log file for writing.");
        return 0;
	}
    if(!isRootPath(directory1) || !isRootPath(directory2)) {
        g_logger.log(L"Error: One or both directories are not root paths. Please select valid directories.");
        g_logger.close();
        return 0;
	}
    if (!GetDirExist(directory1)) {
        g_logger.log(std::wstring(L"Directory does NOT exist: ") + directory1);
        g_logger.close();
        return 0;
    }
    if (!GetDirExist(directory2)) {
        g_logger.log(std::wstring(L"Directory does NOT exist: ") + directory2);
        g_logger.close();
        return 0;
    }
    if (directory1 == directory2) {
        g_logger.log(L"Directory 1 and Directory 2 are the same, no need to compare");
        g_logger.close();
        return 0;
    }
    FilesUnique.clear();
    //START THREADS
    //start time
    uint64_t totaltime = 0;
    uint64_t timestart = GetTickCount64();
#ifdef THREADED_CALLS
    try {
        unsigned int coreCount = GetCoreCount();
        g_logger.log(L"Available CPU cores: " + std::to_wstring(coreCount));

        // Create threads using std::thread
        std::thread thread1(FindFiles, std::wstring(directory1), std::ref(FilesUnique));
        std::thread thread2(FindFiles, std::wstring(directory2), std::ref(FilesUniqueDirectory2));

        // Set thread affinity to different cores if available
        if (coreCount >= 2) {
            SetThreadAffinity(thread1, 0);  // Core 0
            SetThreadAffinity(thread2, 1);  // Core 1
            g_logger.log(L"Threads pinned to cores 0 and 1");
        }
        else {
            g_logger.log(L"Only 1 core available; threads will share the same core");
        }

        // Wait for both threads to complete
        thread1.join();
        thread2.join();

        g_logger.log(L"THREADS FINISHED JOINED=================================");
    }
    catch (const std::exception&) {  // Remove variable 'e' since it's unused
        g_logger.log(L"FATAL THREAD ERROR===========================================");
        g_logger.close();
        return 0;
    }
    //END THREADS
#else
    FindFiles(directory1, FilesUnique);
    FindFiles(directory2, FilesUniqueDirectory2);
#endif

    // Check if operation was cancelled
    if (gbCancelOperation) {
        g_logger.log(L"Operation cancelled by user");
        FilesUnique.clear();
        FilesUniqueDirectory2.clear();
        gbCancelOperation = 0;
        g_logger.close();
        return 0;
    }

    //compare two unique lists and crate file list in FilesUnique
    CompareFiles(FilesUnique, FilesUniqueDirectory2);
    FilesUnique.splice(FilesUnique.end(), FilesUniqueDirectory2);
    DumpUniqueFiles(FilesUnique);
    FilesUnique.clear();
    //get end time
    totaltime = GetTickCount64() - timestart;
    g_logger.log(L"TOTAL MILLISECONDS TIME FOR FAST OPERATION IS: " + std::to_wstring(totaltime));
    g_logger.close();
    gbCancelOperation = 0;
    return TRUE;
}

//
//   FUNCTION: SlowCompare(WCHAR *directory1, WCHAR *directory2)
//
//   PURPOSE: To compare filenames and some properties
//
int SlowCompare(const std::wstring& directory1, const std::wstring& directory2) {

    // Reset cancellation flag at start
    gbCancelOperation = 0;

    if (!g_logger.isOpen()) {
        g_logger.reopen();
    }

    if (!g_logger.isOpen()) {
        g_printer.print(L"Failed to open log file for writing.");
        return 0;
    }
    if (!isRootPath(directory1) || !isRootPath(directory2)) {
        g_logger.log(L"Error: One or both directories are not root paths. Please select valid directories.");
        g_logger.close();
        return 0;
    }

    if (!GetDirExist(directory1)) {
        g_logger.log(std::wstring(L"Directory does NOT exist: ") + directory1);
        g_logger.close();
        return 0;
    }
    if (!GetDirExist(directory2)) {
        g_logger.log(std::wstring(L"Directory does NOT exist: ") + directory2);
		g_logger.close();
        return 0;
    }
    if (directory1 == directory2) {
        g_logger.log(L"Directory 1 and Directory 2 are the same, no need to compare");
        g_logger.close();
        return 0;
    }
    FilesUnique.clear();
    uint64_t totaltime = 0;
    uint64_t timestart = GetTickCount64();
#ifdef THREADED_CALLS
    try {
        unsigned int coreCount = GetCoreCount();
        g_logger.log(L"Available CPU cores: " + std::to_wstring(coreCount));

        // Create threads using std::thread
        std::thread thread1(FindFilesSlow, std::wstring(directory1), std::ref(FilesUnique));
        std::thread thread2(FindFilesSlow, std::wstring(directory2), std::ref(FilesUniqueDirectory2));

        // Set thread affinity to different cores if available
        if (coreCount >= 2) {
            SetThreadAffinity(thread1, 0);  // Core 0
            SetThreadAffinity(thread2, 1);  // Core 1
            g_logger.log(L"Threads pinned to cores 0 and 1");
        }
        else {
            g_logger.log(L"Only 1 core available; threads will share the same core");
        }

        // Wait for both threads to complete
        thread1.join();
        thread2.join();

        g_logger.log(L"THREADS FINISHED JOINED=================================");
    }
    catch (const std::exception&) {  // Remove variable 'e' since it's unused
        g_logger.log(L"FATAL THREAD ERROR===========================================");
        g_logger.close();
        return 0;
    }
#else
    FindFilesSlow(directory1, FilesUnique);
    FindFilesSlow(directory2, FilesUniqueDirectory2); 
#endif

    // Check if operation was cancelled
    if (gbCancelOperation) {
        g_logger.log(L"Operation cancelled by user");
        FilesUnique.clear();
        FilesUniqueDirectory2.clear();
        gbCancelOperation = 0;
        g_logger.close();
        return 0;
    }

    //compare two unique lists and crate file list in FilesUnique
    CompareFilesSlow(FilesUnique, FilesUniqueDirectory2);
    FilesUnique.splice(FilesUnique.end(), FilesUniqueDirectory2);
    DumpUniqueFilesSlow(FilesUnique);
    FilesUnique.clear();
    //get end time
    totaltime = GetTickCount64() - timestart;
    g_logger.log(L"TOTAL MILLISECONDS TIME FOR SLOW OPERATION IS: " + std::to_wstring(totaltime));
    g_logger.close();
    gbCancelOperation = 0;
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
//   FUNCTION: CheckMemoryLimit(SIZE_t currentUsage)
//
//   PURPOSE: Check if memory usage exceeds limits
//   RETURNS: TRUE if within limits, FALSE if exceeded
//
int CheckMemoryLimit(SIZE_T currentUsage)
{
    SIZE_T maxBytes = (SIZE_T)MAX_MEMORY_MB * 1024 * 1024;
    SIZE_T warningBytes = (SIZE_T)WARNING_MEMORY_MB * 1024 * 1024;
    
    if (currentUsage > maxBytes)
    {
        return 0;  // Exceeded limit
    }
    
    if (currentUsage > warningBytes)
    {
        g_logger.log(L"WARNING: Memory usage high (" + std::to_wstring(currentUsage / (1024 * 1024)) + L" MB). Consider smaller directories.");
    }
    
    return 1;
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
//   NOTE: Now includes memory limit checking. Cross-platform implementation.
//
void FindFiles(const std::wstring& directory, std::list<CFileListItem>& filesList)
{
#ifdef _WIN32
    // ==================== WINDOWS IMPLEMENTATION ====================
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
                    g_logger.log(L"ERROR: Memory limit exceeded (" + std::to_wstring(currentMemory / (1024 * 1024)) + L" MB). Stopping directory scan.");
                    FindClose(search_handle);
                    return;
                }
            }

            if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if ((!wcscmp(file.cFileName, L".")) || (!wcscmp(file.cFileName, L"..")))
                    continue;
            }
            
            tmp = directory + L"\\" + std::wstring(file.cFileName);
            
            if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                directories.push_back(tmp);
            }
            else
            {
                uint32_t sz = 0;
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
                    g_logger.log(L"Memory limit reached. Stopped at directory: " + *iter);
                    return;
                }
            }

            FindFiles(*iter, filesList);
        }
    }

#elif defined(__linux__) || defined(__APPLE__)
    // ==================== LINUX/MACOS IMPLEMENTATION ====================
    DIR* dir = opendir(WstringToUtf8(directory).c_str());
    if (dir != nullptr)
    {
        std::vector<std::wstring> directories;
        struct dirent* entry;
        int fileCount = 0;

        while ((entry = readdir(dir)) != nullptr)
        {
            // Check for cancellation
            if (gbCancelOperation)
            {
                closedir(dir);
                return;
            }

            // Check memory periodically (every 1000 files) instead of every file
            if (++fileCount % 1000 == 0)
            {
                SIZE_T currentMemory = GetCurrentMemoryUsage();
                if (!CheckMemoryLimit(currentMemory))
                {
                    g_logger.log(L"ERROR: Memory limit exceeded (" + std::to_wstring(currentMemory / (1024 * 1024)) + L" MB). Stopping directory scan.");
                    closedir(dir);
                    return;
                }
            }

            // Skip . and ..
            if ((wcscmp(Utf8ToWstring(entry->d_name).c_str(), L".") == 0) || 
                (wcscmp(Utf8ToWstring(entry->d_name).c_str(), L"..") == 0))
            {
                continue;
            }

            std::wstring fullPath = directory + L"/" + Utf8ToWstring(entry->d_name);
            struct stat fileStat;

            if (stat(WstringToUtf8(fullPath).c_str(), &fileStat) == 0)
            {
                if (S_ISDIR(fileStat.st_mode))
                {
                    directories.push_back(fullPath);
                }
                else if (S_ISREG(fileStat.st_mode))
                {
                    uint32_t sz = fileStat.st_size;
                    if (sz > 0)
                    {
                        // Get file timestamps - convert to Windows format for compatibility
                        uint32_t lowTime = (uint32_t)(fileStat.st_mtime & 0xFFFFFFFF);
                        uint32_t highTime = (uint32_t)((fileStat.st_mtime >> 32) & 0xFFFFFFFF);

                        Add(Utf8ToWstring(entry->d_name), sz, lowTime, highTime, filesList);
                    }
                }
            }
        }

        closedir(dir);

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
                    g_logger.log(L"Memory limit reached. Stopped at directory: " + *iter);
                    return;
                }
            }

            FindFiles(*iter, filesList);
        }
    }

#else
    // ==================== UNSUPPORTED PLATFORM ====================
    g_logger.log(L"ERROR: FindFiles() is not supported on this platform.");
#endif
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
		g_logger.log(L"ERROR: Failed to allocate memory for file hash.");
		return;
	}

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
					g_logger.log(L"ERROR: Memory limit exceeded (" + std::to_wstring(currentMemory / (1024 * 1024)) + L" MB). Stopping scan.");
					FindClose(search_handle);
					free(pFileHash);
					return;
				}
			}

            if (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if ((!wcscmp(file.cFileName, L".")) || (!wcscmp(file.cFileName, L"..")))

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
                    g_logger.log(L"FILE[" + std::wstring(file.cFileName) + L"] NOT FOUND...");
                }
                else if (res == ERROR_ACCESS_DENIED)
                {
                    g_logger.log(L"FILE[" + std::wstring(file.cFileName) + L"] ACCESS DENIED...");
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
                    g_logger.log(L"Memory limit reached. Stopped at: " + *iter);
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
    g_logger.log(L"Building index of " + std::to_wstring(filesListDirectory2.size()) + L" files from directory 2...");
    
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
#if _DEBUG
    g_logger.log(L"===================================== FAST COMPARE DONE============================================");
	g_printer.print(L"===================================== FAST COMPARE DONE============================================");
#endif
    if (isCheckShowFiles == TRUE) {
        int itemCount = 0;
        for (auto i = filesList.begin(); i != filesList.end(); ++i)
        {
            // Pump messages every 100 items to allow UI updates and Stop button clicks
            if (++itemCount % 100 == 0)
            {
                Idle(1);  // Process messages to allow Stop button to work
                if (gbCancelOperation)
                {
                    g_logger.log(L"Cancelled while displaying files");
                    g_printer.print(L"Cancelled while displaying files");
                    return;
                }
            }
            std::wstring outstr = L"FILE[" + i->m_Filename + L"]::SIZE[" + std::to_wstring(i->m_Size) + L"]::LASTWRITE[" + std::to_wstring(i->m_dwLowDateTime) + L"." + std::to_wstring(i->m_dwHighDateTime) + L"]";
            g_logger.log(outstr);
            g_printer.print(outstr);
        }
    }
    std::wstring endstr = L"**FAST COMPARE END RESULT [" + std::to_wstring(filesList.size()) + L"] UNIQUE FILES**";
    g_logger.log(endstr);
    g_printer.print(endstr);
    
#if _DEBUG
    g_logger.log(L"=================================================================================================");
#endif
}

void DumpUniqueFilesSlow(std::list<CFileListItem>& filesList) {
#if _DEBUG
    g_logger.log(L"===================================== FAST COMPARE DONE============================================");
	g_printer.print(L"===================================== FAST COMPARE DONE============================================");
#endif

    if (isCheckShowFiles == TRUE) {
        int itemCount = 0;
        for (auto i = filesList.begin(); i != filesList.end(); ++i)
        {
            // Pump messages every 100 items to allow UI updates and Stop button clicks
            if (++itemCount % 100 == 0)
            {
                Idle(1);  // Process messages to allow Stop button to work
                if (gbCancelOperation)
                {
                    g_logger.log(L"Cancelled while displaying files");
                    g_printer.print(L"Cancelled while displaying files");
                    return;
                }
            }
			std::wstring outstr = L"FILE[" + i->m_Filename + L"]::H1[" + std::to_wstring(i->m_dwHash[0]) + L"]::H2[" + std::to_wstring(i->m_dwHash[1]) + L"]::H3[" + std::to_wstring(i->m_dwHash[2]) + L"]::H4[" + std::to_wstring(i->m_dwHash[3]) + L"]";
            g_logger.log(outstr);
			g_printer.print(outstr);
        }
    }
    
    std::wstring endstr = L"**SLOW COMPARE END RESULT [" + std::to_wstring(filesList.size()) + L"] UNIQUE FILES**";
    g_logger.log(endstr);
    g_printer.print(endstr);
#if _DEBUG
    g_logger.log(L"=================================================================================================");
#endif
}

uint32_t Add(const std::wstring& filename, const uint32_t& size, const uint32_t& lt, const uint32_t& ht, std::list<CFileListItem>& filesList)
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

uint32_t AddSlow(const std::wstring& filename, const uint32_t& h1, const uint32_t& h2, const uint32_t& h3, const uint32_t& h4, std::list<CFileListItem>& filesList)
{
    CFileListItem fileData;
    fileData.SetFilename(filename);
    fileData.SetFileHash(h1, h2, h3, h4);
    // push file onto list
    filesList.push_back(fileData);

    return(0);
}

uint32_t FileInList(const std::wstring& filename, const uint32_t& size, const uint32_t& lt, const uint32_t& ht, std::list<CFileListItem> &filesList)
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

uint32_t FileInListSlow(const std::wstring& filename, const uint32_t& h1, const uint32_t& h2, const uint32_t& h3, const uint32_t& h4, std::list<CFileListItem>& filesList)
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
            return 0;
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
            return 1;
        }
        else {
            /* exists but is no dir */
            return 0;
        }
#endif
    }
}

// Utility function to convert UTF-8 std::string to std::wstring (UTF-16)
std::wstring Utf8ToWstring(const std::string& str)
{
    if (str.empty()) return std::wstring();
    
#ifdef _WIN32
    // Windows: Use MultiByteToWideChar
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
#else
    // Linux/macOS: Use std::codecvt
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
#endif
}

// Utility function to convert std::wstring (UTF-16) to UTF-8 std::string
std::string WstringToUtf8(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    
#ifdef _WIN32
    // Windows: Use WideCharToMultiByte
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
#else
    // Linux/macOS: Use std::codecvt
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
#endif
}

namespace fs = std::filesystem;
// Utility function to check if a path is absolute, in case 
// the developer provides a way to select file on anonther platform.
// use the to weed out absolute paths
bool isAbsolutePath(const std::wstring& path) {
    return fs::path(path).is_absolute();
}

//make sure it is NOT a relative path
bool isRootPath(const std::wstring& path) {
    return fs::path(path).root_name().empty() == false;
}