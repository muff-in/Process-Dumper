#include <Windows.h>
#include <CommCtrl.h>
#include <DbgHelp.h>
#include <string>
#include <iostream>

#pragma comment(lib, "Dbghelp.lib")

#define IDM_ABOUT 1001
#define IDC_PROCESSDUMPERGUI 101
#define IDC_PID_EDIT 100
#define IDC_DUMP_BUTTON 102

using namespace std;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); 

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = { 0 };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"ProcessDumper";

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND hWnd = CreateWindow(
        L"ProcessDumper",
        L"Process Dumper",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        400, 200,
        NULL, NULL, hInstance, NULL
    );

    if (hWnd == NULL) {
        MessageBox(NULL, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}





LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND  hPidText, hPidEdit, hFileEdit, hDumpButton;
    static HMENU hMenu, hMenuChild;
    switch (msg) {
    case WM_CREATE:


        hMenu = CreateMenu();
        hMenuChild = CreatePopupMenu();
        AppendMenu(hMenuChild, MF_STRING, IDM_ABOUT, L"About");
        AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hMenuChild, L"Help");
        SetMenu(hWnd, hMenu);

        hPidText = CreateWindow(
            L"STATIC",
            L"Enter process ID:",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 30, 150, 20,
            hWnd, NULL, NULL, NULL
        );


        hPidEdit = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            NULL,
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            10, 50, 250, 20,
            hWnd, (HMENU)IDC_PID_EDIT, NULL, NULL
        );
        hDumpButton = CreateWindow(
            L"BUTTON",
            L"Dump",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            270, 50, 100, 20,
            hWnd, (HMENU)IDC_DUMP_BUTTON, NULL, NULL
        );
        break;

    case WM_COMMAND:

        if (LOWORD(wParam) == IDM_ABOUT)
        {
            MessageBox(hWnd, L"Process Dumper, version 0.1 \n Copyright (c) RixedLabs 2023", L"About Process Dumper", MB_OK);
            return 0;

                break;
        }



        if (LOWORD(wParam) == IDC_DUMP_BUTTON) {
            WCHAR pidBuffer[32];
            GetWindowText(hPidEdit, pidBuffer, 32);
            DWORD pid = _wtoi(pidBuffer);


            HANDLE hOpenProcess{
            OpenProcess(
                PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
                NULL,
                pid
                )
            };

            if (hOpenProcess == NULL) {
                MessageBox(hWnd, L"Opening process failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
                break;
            }

            HANDLE hFileCreation{
                CreateFileW(
                    L"C:\\Users\\Public\\Documents\\DumpFile.dmp",
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    CREATE_NEW,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                )
            };

            if (hFileCreation == INVALID_HANDLE_VALUE) {
                DWORD dwError = GetLastError();
                if (dwError == ERROR_FILE_EXISTS) {
                    MessageBox(hWnd, L"File already exists!", L"Error", MB_ICONEXCLAMATION | MB_OK);
                }
                else {
                    MessageBox(hWnd, L"File creation failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
                }
                break;
            }



            MINIDUMP_EXCEPTION_INFORMATION meInfo;
            ZeroMemory(&meInfo, sizeof(meInfo));
            MINIDUMP_CALLBACK_INFORMATION mcInfo;
            ZeroMemory(&mcInfo, sizeof(mcInfo));

            BOOL Dumping{
                MiniDumpWriteDump(
                    hOpenProcess,
                    pid,
                    hFileCreation,
                    (MINIDUMP_TYPE)0x00000002,
                    &meInfo,
                    NULL,
                    &mcInfo
                )
            };

            if (Dumping == FALSE) {
                MessageBox(hWnd, L"Dumping failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
                break;
            }

            CloseHandle(hOpenProcess);
            CloseHandle(hFileCreation);

            MessageBox(hWnd, L"Dumping successful! Checkout Public Documents Directory ", L"Success", MB_OK);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}