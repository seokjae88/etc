// testCurl.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <conio.h>
#include <atlconv.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>

#include "httpClient.h"

HANDLE hStdin;
DWORD fdwSaveOldMode;

void ErrorExit(LPCSTR);
void logPrintFunc(const std::string& log);

int main()
{
    HWND console = GetConsoleWindow();
    RECT ConsoleRect;
    GetWindowRect(console, &ConsoleRect);

    //MoveWindow(console, ConsoleRect.left, ConsoleRect.top, 600, 500, TRUE);
    SetConsoleTitle(TEXT("mp4Parser"));

    DWORD fdwMode;
    INPUT_RECORD irInBuf[128];
    int counter = 0;

    // Get the standard input handle. 
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");

    // Save the current input mode, to be restored on exit. 
    if (!GetConsoleMode(hStdin, &fdwSaveOldMode))
        ErrorExit("GetConsoleMode");

    // Enable the window and mouse input events. 
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS;
    if (!SetConsoleMode(hStdin, fdwMode))
        ErrorExit("SetConsoleMode");


    CHTTPClient* hClient = new CHTTPClient(logPrintFunc);
    hClient->InitSession();

    //hClient->SetProgressFnCallback();
    long status;
    if (hClient->DownloadFile("D:/jsj/2. 개발업무/src/test/down.mp4", "https://www.youtube.com/watch?v=nbc6jm_v1KM", status)) {
        std::cout << "Hello World!\n";
    }    
}

void logPrintFunc(const std::string& log) {
    std::cout << log << "\n";
}

void ErrorExit(LPCSTR lpszMessage)
{
    fprintf(stderr, "%s\n", lpszMessage);

    // Restore input mode on exit.

    SetConsoleMode(hStdin, fdwSaveOldMode);

    ExitProcess(0);
}
