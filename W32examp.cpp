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
HWND hListBox;
HWND ghListBox = 0;
WCHAR szDirectory1[MAX_PATH];
WCHAR szDirectory2[MAX_PATH];
WCHAR szExt[100] = L"*";
WCHAR szType[100] = L"";
BOOL isCheckShowFiles = FALSE;
int windowX, windowY, windowWidth, windowHeight, editBoxWidth, editBoxHeight, listBoxHeight;

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

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_CAPTION | WS_THICKFRAME | WS_SYSMENU,
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


   hEditWin = CreateWindowEx(WS_EX_CLIENTEDGE,L"EDIT", szDirectory1,
//       ES_READONLY | /*ES_WANTRETURN |*/ WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE | ES_MULTILINE,
       WS_CHILD | WS_VISIBLE | ES_MULTILINE,
       CW_USEDEFAULT, CW_USEDEFAULT,
       CW_USEDEFAULT, CW_USEDEFAULT,
       hWnd, nullptr, hInstance, nullptr);


   hButtonWin = CreateWindowEx(WS_EX_CLIENTEDGE, L"BUTTON", L"...",
       WS_CHILD | WS_VISIBLE ,
       CW_USEDEFAULT, CW_USEDEFAULT,
       CW_USEDEFAULT, CW_USEDEFAULT,
       hWnd, (HMENU)ID_FILE_FINDDIRECTORYONE, hInstance, nullptr);

   hEditWin2 = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", szDirectory2,
       WS_CHILD | WS_VISIBLE | ES_MULTILINE,
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
   SetWindowPos(hButtonWin, NULL, editBoxWidth+10, 20, 60, editBoxHeight, SWP_SHOWWINDOW);
   UpdateWindow(hButtonWin);

   ShowWindow(hEditWin2, nCmdShow);
   SetWindowPos(hEditWin2, NULL, 20, 80, editBoxWidth, editBoxHeight, SWP_SHOWWINDOW);
   UpdateWindow(hEditWin2);

   ShowWindow(hButtonWin2, nCmdShow);
   SetWindowPos(hButtonWin2, NULL, editBoxWidth+10, 80, 60, editBoxHeight, SWP_SHOWWINDOW);
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

   EnableWindow(hListBox, true);
   ShowWindow(hListBox, SW_SHOW);
   SetWindowPos(hListBox, NULL, 20, (editBoxHeight * 4) + 55, editBoxWidth, listBoxHeight, SWP_SHOWWINDOW);
   UpdateWindow(hListBox);

   ShowWindow(hWnd, SW_SHOW);
   UpdateWindow(hWnd);
   WCHAR ready[] = L"Ready...";
   printToScreen(ready);
   return TRUE;
}

void DisableMenusAndButtons(HWND hWnd) {
    EnableWindow(hButtonWin, false);
    EnableWindow(hButtonWin2, false);
    EnableWindow(hButtonWinFC, false);
    EnableWindow(hButtonWinSC, false);
    EnableWindow(hButtonWinCBOX, false);
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
                GetDirRequestorLoad(szDirectory1, MAX_PATH);
                SendMessage(hEditWin, WM_SETTEXT, 0, (LPARAM)szDirectory1);
                EnableMenusAndButtons(hWnd);
            }
                break;
            case ID_FILE_FINDDIRECTORYTWO:
            {
                DisableMenusAndButtons(hWnd);
                GetDirRequestorLoad(szDirectory2, MAX_PATH);
                SendMessage(hEditWin2, WM_SETTEXT, 0, (LPARAM)szDirectory2);
                EnableMenusAndButtons(hWnd);
            }
                break;
            case ID_FILE_RUNFASTCOMPARE:
            {
                DisableMenusAndButtons(hWnd);
                SendMessage(hEditWin, WM_GETTEXT, 260, (LPARAM)szDirectory1);
                SendMessage(hEditWin2, WM_GETTEXT, 260, (LPARAM)szDirectory2);
                if (BST_CHECKED == SendMessage(hButtonWinCBOX, BM_GETCHECK, 0, 0)) {
                    isCheckShowFiles = TRUE;
                }
                else {
                    isCheckShowFiles = FALSE;
                }
                FastCompare(szDirectory1, szDirectory2);
                EnableMenusAndButtons(hWnd);
            }
                break;
            case ID_FILE_RUNSLOWCOMPARE:
            {
                DisableMenusAndButtons(hWnd);
                SendMessage(hEditWin, WM_GETTEXT, 260, (LPARAM)szDirectory1);
                SendMessage(hEditWin2, WM_GETTEXT, 260, (LPARAM)szDirectory2);
                if (BST_CHECKED == SendMessage(hButtonWinCBOX, BM_GETCHECK, 0, 0)) {
                    isCheckShowFiles = TRUE;
                }
                else {
                    isCheckShowFiles = FALSE;
                }
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

