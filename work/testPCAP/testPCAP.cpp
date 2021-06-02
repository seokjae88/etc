// testPCAP.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <conio.h>
#include <atlconv.h>

#include "netCap.h"

HANDLE hStdin;
DWORD fdwSaveOldMode;

void ErrorExit(LPCSTR);
void onCapture(const pcap_pkthdr*, const uint8_t*);

int main()
{
    HWND console = GetConsoleWindow();
    RECT ConsoleRect;
    GetWindowRect(console, &ConsoleRect);

    MoveWindow(console, ConsoleRect.left, ConsoleRect.top, 500, 400, TRUE);

    SetConsoleTitle(TEXT("testPCAP"));

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


    PcapReader* pReader = new PcapReader();
    std::cout << "network device num : ";
    int num = _getch();
    num = num - '0';
    std::cout << num << "\n";

    if (pReader->init(num, "udp")) {
        pReader->start(onCapture);
    }

    system("pause");

    Sleep(1000);
    std::cout << "Bye Bye!\n";
}

void ErrorExit(LPCSTR lpszMessage)
{
    fprintf(stderr, "%s\n", lpszMessage);

    // Restore input mode on exit.

    SetConsoleMode(hStdin, fdwSaveOldMode);

    ExitProcess(0);
}

void onCapture(const pcap_pkthdr* pHdr, const uint8_t* pData) {
    PIPHEADER pIpHdr;
    PUDPHEADER pUdpHdr;
    PTCPHEADER pTCPHdr;
	uint8_t* pUdpData;
    NIPPORTINFO stSrcInfo, stDstInfo;
    PNMac pMacSrc, pMacDst;
	uint32_t nIpLen, nUdpLen;

    std::cout << "in data111111111\n";
	if (pHdr->caplen == 0)
	{
		return;
	}
	pMacSrc = (PNMac)pData;
	pMacDst = (PNMac)pData + sizeof(NMac);
	pIpHdr = (PIPHEADER)(pData + DEFLEN_ETHHDR);
	if (pIpHdr->nProto != UDP_PROTOCOL && pIpHdr->nProto != TCP_PROTOCOL) {
		return;
	}
	/*
	sprintf(szSrcIP,"%d.%d.%d.%d",
	*(((uchar*)&stSrcInfo.nIp)) , *(((uchar*)&stSrcInfo.nIp)+1), *(((uchar*)&stSrcInfo.nIp)+2) ,*(((uchar*)&stSrcInfo.nIp)+3));

	sprintf(szDestIP, "%d.%d.%d.%d",
	*(((uchar*)&stDstInfo.nIp)) , *(((uchar*)&stDstInfo.nIp)+1), *(((uchar*)&stDstInfo.nIp)+2) ,*(((uchar*)&stDstInfo.nIp)+3));
	*/

	nIpLen = (pIpHdr->nVerIhl & 0xf) * 4;
	pUdpHdr = (PUDPHEADER)((uint8_t*)pIpHdr + nIpLen);
	pTCPHdr = (PTCPHEADER)((uint8_t*)pIpHdr + nIpLen);
	nUdpLen = ntohs(pUdpHdr->nLength) - sizeof(*pUdpHdr);
	pUdpData = (uint8_t*)((uint8_t*)pUdpHdr + sizeof(*pUdpHdr));

	stSrcInfo.nIp = pIpHdr->stSrcAddr.nIpAddr;
	stDstInfo.nIp = pIpHdr->stDstAddr.nIpAddr;

	if (pIpHdr->nProto == UDP_PROTOCOL) {
		stSrcInfo.nPort = ntohs(pUdpHdr->nSrcPort);
		stDstInfo.nPort = ntohs(pUdpHdr->nDstPort);
	}
	else {
		stSrcInfo.nPort = ntohs(pTCPHdr->nSrcPort);
		stDstInfo.nPort = ntohs(pTCPHdr->nDstPort);
	}

    std::cout << "in data!222222222222\n";
    for (int i = 0; i < nUdpLen; i++) {
        //std::cout << std::hex << pUdpData[i] << " ";
    }
    
	return;
}