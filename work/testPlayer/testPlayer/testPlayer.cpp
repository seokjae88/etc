#include <conio.h>
#include <atlconv.h>
#include <iostream>

#include "muxer.h"


HANDLE hStdin;
DWORD fdwSaveOldMode;

void ErrorExit(LPCSTR);

int main()
{
    HWND console = GetConsoleWindow();
    RECT ConsoleRect;
    GetWindowRect(console, &ConsoleRect);

    MoveWindow(console, ConsoleRect.left, ConsoleRect.top, 500, 400, TRUE);

    SetConsoleTitle(TEXT("testPlayer"));


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
    
    std::string in_filename_v = "D:/jsj/2. 개발업무/test/file3_track1.h264";
    std::string in_filename_a = "D:/jsj/2. 개발업무/test/file3_track2.aac";
    std::string out_filename = "D:/jsj/2. 개발업무/test/test_out.mp4";


    muxer* muxer_ = new muxer();
    muxer_->run();

    system("pause");
}

void ErrorExit(LPCSTR lpszMessage)
{
    fprintf(stderr, "%s\n", lpszMessage);

    // Restore input mode on exit.

    SetConsoleMode(hStdin, fdwSaveOldMode);

    ExitProcess(0);
}