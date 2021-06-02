// fileDiff.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <conio.h>
#include <atlconv.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>

#define READSIZE 16

HANDLE hStdin;
DWORD fdwSaveOldMode;

void ErrorExit(LPCSTR lpszMessage)
{
    fprintf(stderr, "%s\n", lpszMessage);

    // Restore input mode on exit.

    SetConsoleMode(hStdin, fdwSaveOldMode);

    ExitProcess(0);
}

int main()
{
    HWND console = GetConsoleWindow();
    RECT ConsoleRect;
    GetWindowRect(console, &ConsoleRect);

    //MoveWindow(console, ConsoleRect.left, ConsoleRect.top, 600, 500, TRUE);

    SetConsoleTitle(TEXT("fileDiff"));


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

    std::ifstream iStreamA;
    std::ifstream iStreamB;
    iStreamA.open("D:/jsj/2. 개발업무/src/work/mp4Parser/Release/bb.aac", std::ios::in | std::ios::binary);
    iStreamB.open("D:/jsj/2. 개발업무/test/file3_track2.aac", std::ios::in | std::ios::binary);

    char dataA[READSIZE];
    char dataB[READSIZE];
    int readCount = 0;
    if (iStreamA.is_open() && iStreamB.is_open()) {
        while (!iStreamA.eof() && !iStreamB.eof()) {
            memset(dataA, 0x0, READSIZE);
            memset(dataB, 0x0, READSIZE);

            iStreamA.read(dataA, READSIZE);
            iStreamB.read(dataB, READSIZE);

            if (memcmp(dataA, dataB, READSIZE) > 0) {
                printf("%d\n\n", readCount);
                for (int i = 0; i < READSIZE; i++) {
                    printf("%d: 0x%02x ", i, dataA[i]);
                }
                printf("\n\n ");
                for (int i = 0; i < READSIZE; i++) {
                    printf("%d: 0x%02x ", i, dataB[i]);
                }
                system("pause");
            }
            readCount++;
        }

        printf("SSSSSSSSSSSSSSSSSS!!!\n");
    }

}

