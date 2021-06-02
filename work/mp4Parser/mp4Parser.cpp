
#include <conio.h>
#include <atlconv.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>

#include <boost/lexical_cast.hpp>

#include "BinaryStream.h"
#include "log.h"
#include "boxParser.h"
#include "trak.hpp"
#include "mkvParser.h"


#define MKV_FILE "D:/jsj/2. 개발업무/src/work/mp4Parser/Release/file3.mkv"
#define MP4_FILE "D:/jsj/2. 개발업무/src/work/mp4Parser/Release/file3.mp4"


HANDLE hStdin;
DWORD fdwSaveOldMode;

void ErrorExit(LPCSTR);
std::string getThisPath();
void makeAudioFile(trak* audioTrak);
void makeVideoFile(trak* videoTrak);

LOG::logManager* logger;

std::string outFile[2] = { "D:/jsj/2. 개발업무/src/work/mp4Parser/Release/aa.h264", "D:/jsj/2. 개발업무/src/work/mp4Parser/Release/bb.aac" };


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

    logger = new LOG::logManager("mp4Parser.log");
    
    //mkvParse();

    std::map<int, trak*> trakData;
    if (mp4Parse(MP4_FILE, trakData)) {
        std::cout << "MP4 File Parse!!\n";
        system("pause");
        
        makeAudioFile(trakData[1]);
        //makeVideoFile(trakData[0]);
    }    
    
    std::cout << "1231232133213\n";
    delete logger;
}
char* makeAudioHeader(int dataLength) {
    char aacHeader[8];
    memset(aacHeader, 0x0, 8);
    aacHeader[0] = 0xff;
    aacHeader[1] = 0xf1;
    aacHeader[2] = 0x50;
    aacHeader[3] = 0x80 | (dataLength>>11);
    aacHeader[4] = (dataLength&0x7FF)>>3;
    aacHeader[5] = ((dataLength & 0x7) << 5) | 0x1f;
    aacHeader[6] = 0xfc;

    return aacHeader;
}

void makeAudioFile(trak* audioTrak) {
    std::ifstream iStream;
    iStream.open("D:/jsj/2. 개발업무/src/work/mp4Parser/Release/file3.mp4", std::ios::in | std::ios::binary);
    if (iStream.is_open()) {
        std::ofstream oStream;
        oStream.open(outFile[1], std::ios::out | std::ios::binary);
        if (oStream.is_open()) {
            std::ofstream splitStream;
            for (int j = 1; j < audioTrak->chunkData.size() + 1; j++) {

                std::string splitFile = "D:/jsj/2. 개발업무/src/work/mp4Parser/audio/a" + std::to_string(j) + ".aac";
                splitStream.open(splitFile, std::ios::out | std::ios::binary);

                chunkInfo* pChunk = audioTrak->get(j);
                iStream.seekg(pChunk->getOffset());

                for (int k = 0; k < pChunk->sampleData.size(); k++) {
                    int sampleSize = pChunk->sampleData[k];
                    oStream.write(makeAudioHeader(sampleSize+7), 7);
                    splitStream.write(makeAudioHeader(sampleSize + 7), 7);

                    char* readData = new char[sampleSize];
                    iStream.read(readData, sampleSize);
                    oStream.write(readData, sampleSize);
                    splitStream.write(readData, sampleSize);
                }

                splitStream.close();
            }
            oStream.close();
        }
        iStream.close();
    }
}

void makeVideoFile(trak* videoTrak) {
    std::ifstream iStream;
    iStream.open("D:/jsj/2. 개발업무/src/work/mp4Parser/Release/file3.mp4", std::ios::in | std::ios::binary);
    if (iStream.is_open()) {
        std::ofstream oStream;
        oStream.open(outFile[0], std::ios::out | std::ios::binary);
        if (oStream.is_open()) {
            std::ofstream iframeFile;
            int iframeCnt = 0;
            char nalhex[4] = { 0x00, 0x00, 0x00, 0x01 };
            oStream.write(nalhex, 4);
            oStream.write(videoTrak->sps, videoTrak->spsLength);
            oStream.write(nalhex, 4);
            oStream.write(videoTrak->pps, videoTrak->ppsLength);

            int prevOffset = 0;
            for (int j = 1; j < videoTrak->chunkData.size() + 1; j++) {
                chunkInfo* pChunk = videoTrak->get(j);
                iStream.seekg(pChunk->getOffset());
                //std::cout << "chunkSize:" << (pChunk->getOffset()-prevOffset) << "]\n";
                int chunkSize = (pChunk->getOffset() - prevOffset);
                prevOffset = pChunk->getOffset();
                for (int k = 0; k < pChunk->sampleData.size(); k++) {
                    int sampleSize = pChunk->sampleData[k];
                    //logger->log(logCode::LOG_I, __FUNCTION__, __LINE__, "chunkSize:%d / sampleSize:%d", chunkSize, sampleSize);
                    //std::cout << "sampleSize:[" << sampleSize << "]\n";
                    char* readData = new char[sampleSize];
                    iStream.read(readData, sampleSize);

                    uint8_t naluType = readData[5] & 0x1f;
                    if (naluType == 0x05) {
                        if (iframeFile.is_open()) {
                            iframeFile.close();
                        }
                        std::string ifileName = "D:/jsj/2. 개발업무/src/work/mp4Parser/iFrame/i" + std::to_string(iframeCnt) + ".h264";
                        iframeFile.open(ifileName, std::ios::out | std::ios::binary);
                        iframeCnt++;
                        iframeFile.write(nalhex, 4);
                        iframeFile.write(videoTrak->sps, videoTrak->spsLength);
                        iframeFile.write(nalhex, 4);
                        iframeFile.write(videoTrak->pps, videoTrak->ppsLength);
                    }

                    oStream.write(nalhex, 4);
                    oStream.write(readData + 4, sampleSize - 4);

                    if (iframeFile.is_open()) {
                        iframeFile.write(nalhex, 4);
                        iframeFile.write(readData + 4, sampleSize - 4);
                    }
                    //logger->logDump(logCode::LOG_I, __FUNCTION__, __LINE__, readData, sampleSize, "test [%d]", sampleSize);
                    //system("pause");
                }
            }
            if (iframeFile.is_open()) {
                iframeFile.close();
            }
            oStream.close();
        }

        iStream.close();
    }
}

bool mp4Parse(const std::string &filename, std::map<int, trak*> trakData) {
    MP4::BinaryStream* readStream = new MP4::BinaryStream();
    if (readStream->open(filename) == false) {
        return false;
    }

    char type[5];
    memset(type, 0x0, 5);
    uint32_t length;
    uint64_t dataLength;

    int mdat_cnt = 0;
    int trakNum = 0;

    while (!readStream->eof()) {
        length = readStream->readBigEndianUnsignedInteger();
        dataLength = 0;

        readStream->read((char*)type, 4);

        if (length == 1)
        {
            dataLength = readStream->readBigEndianUnsignedInteger() - 16;
        }
        else
        {
            dataLength = length - 8;
        }
        logger->log(logCode::LOG_P, __FUNCTION__, __LINE__, "type:%s / dataLength:%d / length:%d", type, dataLength, length);
        if (strcmp(type, "dinf") == 0
            || strcmp(type, "edts") == 0
            || strcmp(type, "ipro") == 0
            || strcmp(type, "mdia") == 0
            || strcmp(type, "meta") == 0
            || strcmp(type, "mfra") == 0
            || strcmp(type, "minf") == 0
            || strcmp(type, "moof") == 0
            || strcmp(type, "moov") == 0
            || strcmp(type, "mvex") == 0
            || strcmp(type, "sinf") == 0
            || strcmp(type, "skip") == 0
            || strcmp(type, "stbl") == 0
            || strcmp(type, "traf") == 0
            || strcmp(type, "trak") == 0)
        {
            if (strcmp(type, "trak") == 0) {
                if (trakData[trakNum]) {
                    trakNum++;
                }
                trakData[trakNum] = new trak();
            }
            std::cout << "container!\n";
            continue;
        }

        // ftyp mdhd MVHD
        if (strcmp(type, "ftyp") == 0) {
            boxParser::parseFTYP(readStream, dataLength);
        }
        else if (strcmp(type, "mdhd") == 0) {
            boxParser::parseMDHD(readStream, dataLength);
        }
        else if (strcmp(type, "mvhd") == 0) {
            boxParser::parseMVHD(readStream, dataLength);
        }
        else if (strcmp(type, "stco") == 0) {
            boxParser::parseSTCO(readStream, dataLength, trakData[trakNum]);
        }
        else if (strcmp(type, "stsd") == 0) {
            boxParser::parseSTSD(readStream, dataLength, trakData[trakNum]);
        }
        else if (strcmp(type, "stsz") == 0) {
            //readStream->ignore(dataLength);
            boxParser::parseSTSZ(readStream, dataLength, trakData[trakNum]);
        }
        else if (strcmp(type, "stsc") == 0) {
            boxParser::parseSTSC(readStream, dataLength, trakData[trakNum]);
        }
        else if (strcmp(type, "mdat") == 0) {
            std::cout << dataLength << "\n";
            readStream->ignore(dataLength);
        }
        else {
            readStream->ignore(dataLength);
        }

        //system("pause");
        Sleep(100);
    }
    delete readStream;
}

void mkvParse() {
    MP4::BinaryStream* mkvStream = new MP4::BinaryStream();
    if (mkvStream->open(MKV_FILE) == false) {
        return;
    }

    while (!mkvStream->eof()) {
        uint8_t mkvId[4];
        memset(mkvId, 0x0, 4);

        uint8_t first;
        first = mkvStream->readUnsignedChar();

        int idSize = getMkvIdSize(first);
        mkvId[0] = first;
        for (int i = 1; i < idSize; i++) {
            mkvId[i] = mkvStream->readUnsignedChar();
        }
        uint32_t id = getMkvId(mkvId, idSize);
        mkv_register_t element = mkv_register_id(id);


        uint64_t dataLength;
        dataLength = getMkvElementSize(mkvStream);

        logger->log(logCode::LOG_I, __FUNCTION__, __LINE__, "id:%x / element name:%s / type:%d / dataLength:%d", id, element.name.c_str(), element.type, dataLength);
        
        if (element.type == ELEMENT_TYPE_MASTER) {
            if (element.id == ELEMENT_SEEK_HEAD
                || element.id == ELEMENT_TRACKS) {
                mkvStream->ignore(dataLength);
            }
            continue;
        }
        else if (element.type == ELEMENT_TYPE_UNKNOWN) {
            break;;
        }
        else {
            mkvStream->ignore(dataLength);
        }
    }
    delete mkvStream;
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
