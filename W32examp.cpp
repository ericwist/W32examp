/*****************
* W32examp.cpp
* Standard windows screen in C++
* for example code
* Author: Eric Wistrand
* Jul 26, 2023
*****************/

#include "framework.h"
#include "W32examp.h"
#include "util.h"
#include <stdarg.h>  // Add at top with other includes

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HWND hEditWin;
HWND hButtonWin;
HWND hEditWin2;
HWND hButtonWin2;
HWND hButtonWinFC;
HWND hButtonWinSC;
HWND hButtonWinCBOX;
HWND hButtonWinStop;
HWND hListBox;
HWND ghListBox = 0;
HBRUSH hEditBrush = NULL;
std::wstring szDirectory1;
std::wstring szDirectory2;
WCHAR szExt[100] = L"*";
WCHAR szType[100] = L"";
BOOL isCheckShowFiles = FALSE;
volatile BOOL gbCancelOperation = FALSE;
int windowX, windowY, windowWidth, windowHeight, editBoxWidth, editBoxHeight, listBoxHeight;

// Add this line in your implementation file
//ThreadSafePrinter g_printer;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_W32EXAMP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
    
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_W32EXAMP));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_W32EXAMP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_W32EXAMP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: BOOL isThereAPreviousInstanceRunning()
//
//   PURPOSE: Run only one app instance
//
//   COMMENTS:
//
//        In this function, check for another app instance running
//

BOOL isThereAPreviousInstanceRunning() {
    BOOL ret = TRUE;
    int tries = 5;
    HANDLE hMutex = NULL;
    Idle(50);
#if _DEBUG
    Idle(200);
#endif
    while (tries > 0) {
        // We don't want to own the mutex.
        hMutex = CreateMutex(NULL, FALSE, szWindowClass);
        if (NULL == hMutex) {
            // Something bad happened, fail.
            // We don't know anything here so
            // just exit and the user will have to
            // double-click icon again
            break;
        }

        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            CloseHandle(hMutex);
            hMutex = NULL;
            // It's possible that the instance we found isn't coming up,
            // but rather is going down.  Try again.
            tries--;
            // Give the other instance time to finish if it's going down.
            Idle(100);
        }
        else {
            // We were the first one to create the mutex
            // so that makes us the main instance.  Leak
            // the mutex in this function so it gets cleaned
            // up by the OS when this instance exits.
            // Return so we continue to open program
            ret = FALSE;
            break;
        }
    }
    return ret;
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    if (TRUE == isThereAPreviousInstanceRunning()) {
        // Do not initialize another one is already running
        return FALSE;
    }

    RECT rc;
    hInst = hInstance; // Store instance handle in our global variable
    //moveWindowUpperLeft((HWND)NULL);
    
    HWND hDTWnd = GetDesktopWindow();
    GetWindowRect(hDTWnd, &rc);
    windowX = 0;
    windowY = 0;
    windowWidth = (rc.right - rc.left) / 2;
    windowHeight = ((rc.bottom - rc.top) - 100);

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_CLIPCHILDREN,
        windowX, windowY, windowWidth, windowHeight, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   
   SetWindowPos(hWnd, NULL, windowX, windowY, windowWidth, windowHeight, SWP_SHOWWINDOW);
   GetWindowRect(hWnd, &rc);
   editBoxWidth = rc.right - rc.left - SM_CXDLGFRAME - SM_CXDLGFRAME - 100;
   editBoxHeight = 40;

//   editBoxWidth = rc.right - rc.left - SM_CXDLGFRAME - SM_CXDLGFRAME - 60;//left border + right border;
   listBoxHeight = rc.bottom - rc.top - SM_CYDLGFRAME - SM_CYSIZE - SM_CYSIZE - (editBoxHeight*4) - 50;//Frame + Titlebar


   hEditWin = CreateWindowEx(WS_EX_CLIENTEDGE,L"EDIT", szDirectory1.c_str(),
       WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY,
       CW_USEDEFAULT, CW_USEDEFAULT,
       CW_USEDEFAULT, CW_USEDEFAULT,
       hWnd, nullptr, hInstance, nullptr);

   hEditBrush = CreateSolidBrush(RGB(255, 255, 255));

   hButtonWin = CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"...",
       WS_CHILD | WS_VISIBLE ,
       CW_USEDEFAULT, CW_USEDEFAULT,
       CW_USEDEFAULT, CW_USEDEFAULT,
       hWnd, (HMENU)ID_FILE_FINDDIRECTORYONE, hInstance, nullptr);

   hEditWin2 = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", szDirectory2.c_str(),
       WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY,
       CW_USEDEFAULT, CW_USEDEFAULT,
       CW_USEDEFAULT, CW_USEDEFAULT,
       hWnd, nullptr, hInstance, nullptr);

   hButtonWin2 = CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"...",
       WS_CHILD | WS_VISIBLE ,
       CW_USEDEFAULT, CW_USEDEFAULT,
       CW_USEDEFAULT, CW_USEDEFAULT,
       hWnd, (HMENU)ID_FILE_FINDDIRECTORYTWO, hInstance, nullptr);

   hButtonWinFC = CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"Run fast compare",
       WS_CHILD | WS_VISIBLE,
       CW_USEDEFAULT, CW_USEDEFAULT,
       CW_USEDEFAULT, CW_USEDEFAULT,
       hWnd, (HMENU)ID_FILE_RUNFASTCOMPARE, hInstance, nullptr);

   hButtonWinSC = CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"Run slow compare",
       WS_CHILD | WS_VISIBLE,
       CW_USEDEFAULT, CW_USEDEFAULT,
       CW_USEDEFAULT, CW_USEDEFAULT,
       hWnd, (HMENU)ID_FILE_RUNSLOWCOMPARE, hInstance, nullptr);

   hButtonWinCBOX = CreateWindowEx(0, L"BUTTON", L"Show unique files",
       WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
       CW_USEDEFAULT, CW_USEDEFAULT,
       CW_USEDEFAULT, CW_USEDEFAULT,
       hWnd, (HMENU)ID_FILE_SHOWUNIQUEFILES, hInstance, nullptr);

   hButtonWinStop = CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"Stop",
       WS_CHILD | WS_VISIBLE,
       CW_USEDEFAULT, CW_USEDEFAULT,
       CW_USEDEFAULT, CW_USEDEFAULT,
       hWnd, (HMENU)ID_FILE_STOPCANCELOP, hInstance, nullptr);

   hListBox = CreateWindowEx(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
       WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_VSCROLL,
       CW_USEDEFAULT, CW_USEDEFAULT, 
       CW_USEDEFAULT, CW_USEDEFAULT,
       hWnd, NULL, hInstance, NULL);

   if (!hEditWin || !hEditWin2 || !hButtonWin || !hButtonWin2 || !hListBox)
       return FALSE;
   

   ghListBox = hListBox;
   ShowWindow(hEditWin, nCmdShow);
   SetWindowPos(hEditWin, NULL, 20, 20, editBoxWidth, editBoxHeight, SWP_SHOWWINDOW);
   UpdateWindow(hEditWin);

   ShowWindow(hButtonWin, nCmdShow);
   SetWindowPos(hButtonWin, NULL, editBoxWidth+20, 20, 60, editBoxHeight, SWP_SHOWWINDOW);
   UpdateWindow(hButtonWin);

   ShowWindow(hEditWin2, nCmdShow);
   SetWindowPos(hEditWin2, NULL, 20, 80, editBoxWidth, editBoxHeight, SWP_SHOWWINDOW);
   UpdateWindow(hEditWin2);

   ShowWindow(hButtonWin2, nCmdShow);
   SetWindowPos(hButtonWin2, NULL, editBoxWidth+20, 80, 60, editBoxHeight, SWP_SHOWWINDOW);
   UpdateWindow(hButtonWin2);

   ShowWindow(hButtonWinFC, nCmdShow);
   SetWindowPos(hButtonWinFC, NULL, 20, (editBoxHeight * 2) + 55, 160, 25, SWP_SHOWWINDOW);
   UpdateWindow(hButtonWinFC);

   ShowWindow(hButtonWinSC, nCmdShow);
   SetWindowPos(hButtonWinSC, NULL, 200, (editBoxHeight * 2) + 55, 160, 25, SWP_SHOWWINDOW);
   UpdateWindow(hButtonWinSC);

   ShowWindow(hButtonWinCBOX, nCmdShow);
   SetWindowPos(hButtonWinCBOX, NULL, 20, (editBoxHeight * 3) + 55, 160, 25, SWP_SHOWWINDOW);
   UpdateWindow(hButtonWinCBOX);
   //turn off files to start
   SendMessage(hButtonWinCBOX, BM_SETCHECK, (WPARAM)0, (LPARAM)ID_FILE_SHOWUNIQUEFILES);
   isCheckShowFiles = FALSE;

   ShowWindow(hButtonWinStop, nCmdShow);
   SetWindowPos(hButtonWinStop, NULL, 380, (editBoxHeight * 2) + 55, 160, 25, SWP_SHOWWINDOW);
   UpdateWindow(hButtonWinStop);

   EnableWindow(hListBox, true);
   ShowWindow(hListBox, SW_SHOW);
   SetWindowPos(hListBox, NULL, 20, (editBoxHeight * 4) + 55, editBoxWidth, listBoxHeight, SWP_SHOWWINDOW);
   UpdateWindow(hListBox);

   ShowWindow(hWnd, SW_SHOW);
   UpdateWindow(hWnd);
   OutputDebugString(L"Ready 1...\n");
   g_outputManager.RegisterCallback(Win32GuiOutputCallback, nullptr);
   g_outputManager.SendOutput(L"Ready...");
   return TRUE;
}

void DisableMenusAndButtons(HWND hWnd) {
    EnableWindow(hButtonWin, false);
    EnableWindow(hButtonWin2, false);
    EnableWindow(hButtonWinFC, false);
    EnableWindow(hButtonWinSC, false);
    EnableWindow(hButtonWinCBOX, false);
    EnableWindow(hButtonWinStop, true);  // Keep Stop button ENABLED during operations
    EnableMenuItem(GetMenu(hWnd), ID_FILE_FINDDIRECTORYONE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    EnableMenuItem(GetMenu(hWnd), ID_FILE_FINDDIRECTORYTWO, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    EnableMenuItem(GetMenu(hWnd), ID_FILE_RUNFASTCOMPARE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    EnableMenuItem(GetMenu(hWnd), ID_FILE_RUNSLOWCOMPARE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
    EnableMenuItem(GetMenu(hWnd), ID_FILE_SHOWUNIQUEFILES, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
}

void EnableMenusAndButtons(HWND hWnd) {
    EnableWindow(hButtonWin, true);
    EnableWindow(hButtonWin2, true);
    EnableWindow(hButtonWinFC, true);
    EnableWindow(hButtonWinSC, true);
    EnableWindow(hButtonWinCBOX, true);
    EnableWindow(hButtonWinStop, false);  // Disable Stop button when not running
    EnableMenuItem(GetMenu(hWnd), ID_FILE_FINDDIRECTORYONE, MF_BYCOMMAND | MF_ENABLED);
    EnableMenuItem(GetMenu(hWnd), ID_FILE_FINDDIRECTORYTWO, MF_BYCOMMAND | MF_ENABLED);
    EnableMenuItem(GetMenu(hWnd), ID_FILE_RUNFASTCOMPARE, MF_BYCOMMAND | MF_ENABLED);
    EnableMenuItem(GetMenu(hWnd), ID_FILE_RUNSLOWCOMPARE, MF_BYCOMMAND | MF_ENABLED);
    EnableMenuItem(GetMenu(hWnd), ID_FILE_SHOWUNIQUEFILES, MF_BYCOMMAND | MF_ENABLED);
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case ID_FILE_FINDDIRECTORYONE:
            {
                DisableMenusAndButtons(hWnd);
                // Ensure szDirectory1 has enough space for MAX_PATH characters
                szDirectory1.resize(MAX_PATH);
                // Pass writable buffer to the function
                GetDirRequestorLoad(&szDirectory1[0], MAX_PATH);
                SendMessage(hEditWin, WM_SETTEXT, 0, (LPARAM)szDirectory1.c_str());
                EnableMenusAndButtons(hWnd);
            }
                break;
            case ID_FILE_FINDDIRECTORYTWO:
            {
                DisableMenusAndButtons(hWnd);
                // Ensure szDirectory2 has enough space for MAX_PATH characters
                szDirectory2.resize(MAX_PATH);
                // Pass writable buffer to the function
                GetDirRequestorLoad(&szDirectory2[0], MAX_PATH);
                SendMessage(hEditWin2, WM_SETTEXT, 0, (LPARAM)szDirectory2.c_str());
                EnableMenusAndButtons(hWnd);
            }
                break;
            case ID_FILE_RUNFASTCOMPARE:
            {
                DisableMenusAndButtons(hWnd);
                WCHAR buffer[MAX_PATH + 1] = { 0 };
                SendMessage(hEditWin, WM_GETTEXT, MAX_PATH, (LPARAM)buffer);
                szDirectory1 = buffer;
                szDirectory1 = RemoveSpacesAndNonPrintable(szDirectory1);
                if (szDirectory1.empty()) {
                    g_outputManager.SendOutput(L"Error: Directory1 is empty. Please select valid directory.");
                    EnableMenusAndButtons(hWnd);
                    return 0;
                }
                if (!isRootPath(szDirectory1)) {
                    g_outputManager.SendOutput(L"Error: Directory 1 is not a root path. Please select a valid directory.");
                    EnableMenusAndButtons(hWnd);
                    return 0;
                }
                ZeroMemory(buffer, sizeof(buffer));
                SendMessage(hEditWin2, WM_GETTEXT, MAX_PATH, (LPARAM)buffer);
                szDirectory2 = buffer;
				szDirectory2 = RemoveSpacesAndNonPrintable(szDirectory2);
                if (szDirectory2.empty()) {
                    g_outputManager.SendOutput(L"Error: Directory2 is empty. Please select valid directory.");
                    EnableMenusAndButtons(hWnd);
                    return 0;
                }
                if (!isRootPath(szDirectory2)) {
                    g_outputManager.SendOutput(L"Error: Directory2  is not a root path. Please select a valid directory.");
                    EnableMenusAndButtons(hWnd);
                    return 0;
                }
                if (BST_CHECKED == SendMessage(hButtonWinCBOX, BM_GETCHECK, 0, 0)) {
                    isCheckShowFiles = TRUE;
                }
                else {
                    isCheckShowFiles = FALSE;
                }
                g_outputManager.SendOutput(L"Starting fast comparison... please wait...");
                FastCompare(szDirectory1, szDirectory2);
                EnableMenusAndButtons(hWnd);
            }
                break;
            case ID_FILE_RUNSLOWCOMPARE:
            {
                DisableMenusAndButtons(hWnd);
                WCHAR buffer[MAX_PATH + 1] = { 0 };
                SendMessage(hEditWin, WM_GETTEXT, MAX_PATH, (LPARAM)buffer);
                szDirectory1 = buffer;
                szDirectory1 = RemoveSpacesAndNonPrintable(szDirectory1);
                if (szDirectory1.empty()) {
                    g_outputManager.SendOutput(L"Error: Directory1 is empty. Please select valid directory.");
                    EnableMenusAndButtons(hWnd);
                    return 0;
                }
                if (!isRootPath(szDirectory1)) {
                    g_outputManager.SendOutput(L"Error: Directory 1 is not a root path. Please select a valid directory.");
                    EnableMenusAndButtons(hWnd);
                    return 0;
                }
                ZeroMemory(buffer, sizeof(buffer));
                SendMessage(hEditWin2, WM_GETTEXT, MAX_PATH, (LPARAM)buffer);
                szDirectory2 = buffer;
                szDirectory2 = RemoveSpacesAndNonPrintable(szDirectory2);
                if (szDirectory2.empty()) {
                    g_outputManager.SendOutput(L"Error: Directory2 is empty. Please select valid directory.");
                    EnableMenusAndButtons(hWnd);
                    return 0;
                }
                if (!isRootPath(szDirectory2)) {
                    g_outputManager.SendOutput(L"Error: Directory2  is not a root path. Please select a valid directory.");
                    EnableMenusAndButtons(hWnd);
                    return 0;
                }
                if (BST_CHECKED == SendMessage(hButtonWinCBOX, BM_GETCHECK, 0, 0)) {
                    isCheckShowFiles = TRUE;
                }
                else {
                    isCheckShowFiles = FALSE;
                }
                g_outputManager.SendOutput(L"Starting slow comparison... please wait...");
                SlowCompare(szDirectory1, szDirectory2);
                EnableMenusAndButtons(hWnd);
            }
                break;
            case ID_FILE_SHOWUNIQUEFILES:
            {
                //DisableMenusAndButtons(hWnd);
                if (wmEvent == BN_CLICKED) {
                    if (BST_CHECKED == SendMessage(hButtonWinCBOX, BM_GETCHECK, 0, 0)) {
                        SendMessage(hButtonWinCBOX, BM_SETCHECK, (WPARAM)0, (LPARAM)ID_FILE_SHOWUNIQUEFILES);
                        isCheckShowFiles = FALSE;
                    }
                    else {
                        SendMessage(hButtonWinCBOX, BM_SETCHECK, (WPARAM)1, (LPARAM)ID_FILE_SHOWUNIQUEFILES);
                        isCheckShowFiles = TRUE;
                    }
                }
                //EnableMenusAndButtons(hWnd);
            }
            break;
            case ID_FILE_STOPCANCELOP:
            {
                gbCancelOperation = TRUE;
                g_outputManager.SendOutput(L"Stop requested - canceling current operation...");
            }
            break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_CTLCOLORSTATIC: 
        {
            HDC hdcEdit = reinterpret_cast<HDC>(wParam);
            HWND hwndEdit = reinterpret_cast<HWND>(lParam);
            OutputDebugString(L"WM_CTLCOLOREDIT received\n");
            if (hwndEdit == hEditWin || hwndEdit == hEditWin2) {
                OutputDebugString(L"Setting hEditWin background to white\n");
                SetBkColor(hdcEdit, RGB(255, 255, 255));  // White background
                SetTextColor(hdcEdit, RGB(0, 0, 0));      // Black text
                SelectObject(hdcEdit, hEditBrush);
                return reinterpret_cast<INT_PTR>(hEditBrush);
            }
        }
        break;
        //keep user from being able to move/size the window
    case WM_SYSCOMMAND:
        switch (
            wParam &
            0xfff0) // (filter out reserved lower 4 bits:  see msdn remarks
            // http://msdn.microsoft.com/en-us/library/ms646360(VS.85).aspx)
        {
        case SC_MOVE:
        case SC_RESTORE:
        case SC_KEYMENU:
        case SC_DEFAULT:
        case SC_SIZE:
        case SC_MAXIMIZE:
        case SC_MINIMIZE:
            return 0;
        default:// All other commands will act in a "default" manner
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_DESTROY:
        if (hEditBrush) {
            DeleteObject(hEditBrush);
            hEditBrush = NULL;
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void printToScreen(WCHAR* FormattedStr)
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

// Global output manager instance
OutputManager g_outputManager;

// Default Win32 GUI callback implementation
void Win32GuiOutputCallback(const wchar_t* message, void* context)
{
    try
    {
        if (ghListBox != (HWND)NULL)
        {
            // Force List Box to bottom
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
                // Clear list box
                SendMessage(ghListBox, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
            }

            int pos = (int)SendMessage(ghListBox, LB_ADDSTRING, 0, (LPARAM)message);
            SendMessage(ghListBox, LB_SETITEMDATA, pos, (LPARAM)0);
            SendMessage(ghListBox, LB_SETCURSEL, pos, (LPARAM)0);
        }
    }
    catch (...)
    {
    }
}
