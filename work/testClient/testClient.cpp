// testClient.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.

#include <conio.h>
#include <atlconv.h>

#include "inc/global.hpp"
#include "inc/client.hpp"

HANDLE hStdin;
DWORD fdwSaveOldMode;

void ErrorExit(LPCSTR);
std::string getThisPath();

int main(int nargc, char* argv[])
{
    try
    {   
        HWND console = GetConsoleWindow();
        RECT ConsoleRect;
        GetWindowRect(console, &ConsoleRect);

        MoveWindow(console, ConsoleRect.left, ConsoleRect.top, 500, 400, TRUE);

        SetConsoleTitle(TEXT("testClient"));


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

        boost::property_tree::ptree pt;
        std::string iniFile = getThisPath() + "/../conf/test.ini";
        boost::property_tree::ini_parser::read_ini(iniFile, pt);
        std::string svrIp = pt.get<std::string>("client.server_ip");
        std::string svrPort = pt.get<std::string>("client.server_port");
        std::string filename = pt.get<std::string>("client.filename");
        std::string outpath = pt.get<std::string>("client.outpath");

        std::cout << "[" << iniFile << "] read!!\n";
        std::cout << "server_ip [" << svrIp << "]\n";
        std::cout << "server_port [" << svrPort << "]\n";
        std::cout << "filename [" << filename << "]\n";
        std::cout << "outpath [" << outpath << "]\n";

        while (1) {
            int key_value;
            std::cout << filename << "파일을 다운로드 하시겠습니까?[y/n] : ";
            key_value = _getch();
            printf("%c\n", key_value);
            Sleep(1000);
            if (key_value == 'Y' || key_value == 'y') {
                std::cout << "server start (ip:" << svrIp << ", port:" << svrPort << ")\n";

                boost::asio::io_context io_context;
                //boost::asio:io_context io_context;
                client c(io_context, svrIp, svrPort, filename);
                c.setOutPath(outpath);
                io_context.run();

                double result;
                result = (double)(c.end - c.start);
                std::cout << "result : " << result << "ms\n";
            }
            system("PAUSE");
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << "\n";
    }
}

void ErrorExit(LPCSTR lpszMessage)
{
    fprintf(stderr, "%s\n", lpszMessage);

    // Restore input mode on exit.

    SetConsoleMode(hStdin, fdwSaveOldMode);

    ExitProcess(0);
}
std::string getThisPath() {
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, path, MAX_PATH);

    USES_CONVERSION;
    std::string str = W2A(path);
    return str.substr(0, str.find_last_of("\\/"));
}
// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
