#include "boxParser.h"


extern LOG::logManager* logger;
void boxParser::parseESDS(MP4::BinaryStream* stream, size_t length, trak* trakData) {
    boxParser::parseFullBox(stream); // 4byte



}
void boxParser::parseAVCC(MP4::BinaryStream* stream, size_t length, trak* trakData) {
    uint8_t configVersion = stream->readUnsignedChar();
    uint8_t avcProfileIndication = stream->readUnsignedChar();
    uint8_t profileCompatibility = stream->readUnsignedChar();
    uint8_t avcLevelIndication = stream->readUnsignedChar();
    

    uint8_t lengthSizeMinusOne = stream->readUnsignedChar();
    lengthSizeMinusOne = lengthSizeMinusOne & 0x03;

    uint8_t numOfSPS = stream->readUnsignedChar();
    numOfSPS = numOfSPS & 0x1F;
    for (int i = 0; i < numOfSPS; i++) {
        uint16_t lengthSPS = stream->readBigEndianUnsignedShort();
        stream->read(trakData->sps, lengthSPS);
        trakData->spsLength = lengthSPS;
        logger->logDump(logCode::LOG_I, __FUNCTION__, __LINE__, (char*)trakData->sps, trakData->spsLength, "sps");
        
    }
    uint8_t numOfPPS = stream->readUnsignedChar();
    for (int i = 0; i < numOfPPS; i++) {
        uint16_t lengthPPS = stream->readBigEndianUnsignedShort();
        stream->read(trakData->pps, lengthPPS);
        trakData->ppsLength = lengthPPS;
        logger->logDump(logCode::LOG_I, __FUNCTION__, __LINE__, (char*)trakData->pps, trakData->ppsLength, "pps");
    }   
}
void boxParser::parseSTSD(MP4::BinaryStream* stream, size_t length, trak* trakData) {
    boxParser::parseFullBox(stream); // 4byte

    uint32_t entryCnt = stream->readBigEndianUnsignedInteger();
    logger->log(logCode::LOG_P, __FUNCTION__, __LINE__, "entryCnt:%d / length:%d", entryCnt, length);
    std::cout << "entryCnt: " << entryCnt << "\n";
    for (int i = 0; i < entryCnt; i++) {
        uint32_t sampleEntryBoxSize = stream->readBigEndianUnsignedInteger();

        std::string sampleType = stream->readString(4);
        logger->log(logCode::LOG_P, __FUNCTION__, __LINE__, "sampleEntryBoxSize:%d / sampleType:%s", sampleEntryBoxSize, sampleType.c_str());
        trakData->setSampleType(sampleType);

        if (sampleType == "avc1") {
            stream->ignore(6); // reserved & predefined
            uint16_t dataReferenceIndex = stream->readBigEndianUnsignedShort();
            stream->ignore((2 + 2 + 12)); // reserved & predefined
            uint16_t width = stream->readBigEndianUnsignedShort();
            uint16_t height = stream->readBigEndianUnsignedShort();
            stream->ignore((4 + 4 + 4));
            uint16_t frameCnt = stream->readBigEndianUnsignedShort();
            std::string compressorname = stream->readString(32);
            uint16_t depth = stream->readBigEndianUnsignedShort();
            stream->ignore(2); // reserved & predefined

            uint32_t avcLength = stream->readBigEndianUnsignedInteger() - 8;
            std::string type = stream->readString(4);
            boxParser::parseAVCC(stream, avcLength, trakData);
        }
        else if (sampleType == "mp4a") {
            stream->ignore(6); // reserved & predefined
            uint16_t dataReferenceIndex = stream->readBigEndianUnsignedShort();
            stream->ignore(8 + 2 + 2 + 4 + 2 + 2);
        }
        else {
            stream->ignore(sampleEntryBoxSize - 8);
            int reservedSize = sampleEntryBoxSize - (6 + 2 + 16 + 4 + 4 + 4 + 4 + 2 + 32 + 2 + 2);
            //stream->ignore(reservedSize);
            logger->log(logCode::LOG_I, __FUNCTION__, __LINE__, "sampleEntryBoxSize:%d / reservedSize:%d", sampleEntryBoxSize, reservedSize);
        }
    }
}
void boxParser::parseSTSZ(MP4::BinaryStream* stream, size_t length, trak* trakData) {
    boxParser::parseFullBox(stream);

    uint32_t sampleSize = stream->readBigEndianUnsignedInteger();
    uint32_t sampleCnt = stream->readBigEndianUnsignedInteger();

    logger->log(logCode::LOG_P, __FUNCTION__, __LINE__, "sample total:[%d]/[%d]", trakData->getSampleCntTotal(), sampleCnt);
    int chunkCnt = trakData->getChunkCnt();
    for (int i = 1; i < chunkCnt+1; i++) {
        chunkInfo* cInfo = trakData->get(i);
        if (cInfo) {
            int chunkSampleCnt = cInfo->getSampleCnt();
            for (int j = 0; j < chunkSampleCnt; j++) {
                uint32_t entrySize = stream->readBigEndianUnsignedInteger();
                cInfo->add(j, entrySize);
                trakData->addSampleSize(entrySize);
                //logger->log(logCode::LOG_I, __FUNCTION__, __LINE__, "chunkNum:%d / sampleCnt:%d / sampleSize:%d", i, j, entrySize);
            }
        }
        else {
            logger->log(logCode::LOG_E, __FUNCTION__, __LINE__, "no chunk info [%d]", i);
        }        
    }
    logger->log(logCode::LOG_I, __FUNCTION__, __LINE__, "sample size total:[%d]", trakData->getSampleSizeTotal());
}

void boxParser::parseSTCO(MP4::BinaryStream* stream, size_t length, trak* trakData) {
    boxParser::parseFullBox(stream);

    uint32_t chunkCnt = stream->readBigEndianUnsignedInteger();
    trakData->setChunkCnt(chunkCnt);
    logger->log(logCode::LOG_I, __FUNCTION__, __LINE__, "chunkCnt:%d", chunkCnt);

    for (int i = 1; i < chunkCnt+1; i++) {
        uint32_t chunkOffset = stream->readBigEndianUnsignedInteger();

        chunkInfo* cInfo = trakData->get(i);
        if (cInfo == nullptr) {
            cInfo = new chunkInfo();
            trakData->add(i, cInfo);
        }
        cInfo->setOffset(chunkOffset);                
    }
}
void boxParser::parseSTSC(MP4::BinaryStream* stream, size_t length, trak* trakData) {
    boxParser::parseFullBox(stream);

    uint32_t entryCnt = stream->readBigEndianUnsignedInteger();
    logger->log(logCode::LOG_I, __FUNCTION__, __LINE__, "entryCnt:%d", entryCnt);
    
    uint32_t nextFirstChunk = 1;
    uint32_t prevSampleCnt = 0;
    for (int i = 0; i < entryCnt; i++) {
        uint32_t firstChunk = stream->readBigEndianUnsignedInteger();
        uint32_t sampleCnt = stream->readBigEndianUnsignedInteger();
        uint32_t sampleIndex = stream->readBigEndianUnsignedInteger();

        if (firstChunk != nextFirstChunk) {
            for (int j = nextFirstChunk; j < firstChunk; j++) {
                chunkInfo* cInfo = trakData->get(j);
                if (cInfo == nullptr) {
                    cInfo = new chunkInfo();
                    trakData->add(j, cInfo);
                }
                cInfo->setIndex(sampleIndex);
                cInfo->setSampleCnt(prevSampleCnt);
                trakData->addSampleCnt(prevSampleCnt);

                //logger->log(logCode::LOG_I, __FUNCTION__, __LINE__, "1chunkNum:%d / sampleCnt:%d / sampleIndex:%d", j, prevSampleCnt, sampleIndex);
            }
        }
        chunkInfo* cInfo = trakData->get(firstChunk);
        if (cInfo == nullptr) {
            cInfo = new chunkInfo();
            trakData->add(firstChunk, cInfo);
        }
        cInfo->setIndex(sampleIndex);
        cInfo->setSampleCnt(sampleCnt);
        prevSampleCnt = sampleCnt;
        trakData->addSampleCnt(sampleCnt);
        

        //logger->log(logCode::LOG_I, __FUNCTION__, __LINE__, "2chunkNum:%d / sampleCnt:%d / sampleIndex:%d", firstChunk, sampleCnt, sampleIndex);

        nextFirstChunk = firstChunk + 1;
        //logger->log(logCode::LOG_I, __FUNCTION__, __LINE__, "firstChunk:%d / nextFirstChunk:%d", firstChunk, nextFirstChunk);
    }    
}
void boxParser::parseMVHD(MP4::BinaryStream* stream, size_t length) {
    uint8_t  _version;
    uint32_t _flags;
    std::tie(_version, _flags) = boxParser::parseFullBox(stream);
    
    uint64_t _creationTime;
    uint64_t _modificationTime;
    uint32_t _timeScale;
    uint64_t _duration;
    float    _rate;
    float    _volume;
    matrix   _matrix;
    uint32_t _nextTrackId;

    if (_version == 1)
    {
        _creationTime = stream->readBigEndianUnsignedLong();
        _modificationTime = stream->readBigEndianUnsignedLong();
        _timeScale = stream->readBigEndianUnsignedInteger();
        _duration = stream->readBigEndianUnsignedLong();
        _rate = stream->readBigEndianFixedPoint(16, 16);
        _volume = stream->readBigEndianFixedPoint(8, 8);

        stream->ignore(10);
        stream->readMatrix(&(_matrix));
        stream->ignore(24);

        _nextTrackId = stream->readBigEndianUnsignedInteger();
    }
    else
    {
        _creationTime = stream->readBigEndianUnsignedInteger();
        _modificationTime = stream->readBigEndianUnsignedInteger();
        _timeScale = stream->readBigEndianUnsignedInteger();
        _duration = stream->readBigEndianUnsignedInteger();
        _rate = stream->readBigEndianFixedPoint(16, 16);
        _volume = stream->readBigEndianFixedPoint(8, 8);

        stream->ignore(10);
        stream->readMatrix(&(_matrix));
        stream->ignore(24);

        _nextTrackId = stream->readBigEndianUnsignedInteger();
    }
    std::cout << "version: " << _version << "\n";
    std::cout << "_flags: " << _flags << "\n";
    std::cout << "_creationTime: " << _creationTime << "\n";
    std::cout << "_modificationTime: " << _modificationTime << "\n";
    std::cout << "_timeScale: " << _timeScale << "\n";
    std::cout << "_duration: " << _duration << "\n";
    std::cout << "_rate: " << _rate << "\n";
    std::cout << "_volume: " << _volume << "\n";
    std::cout << "_nextTrackId: " << _nextTrackId << "\n";
    std::cout << "_matrix: " << _matrix.a << " / " << _matrix.b << " / " << _matrix.u << " / " << _matrix.c << " / " << _matrix.d << " / " << _matrix.v << " / " << _matrix.x << " / " << _matrix.y << " / " << _matrix.w << "\n";
}
void boxParser::parseMDHD(MP4::BinaryStream* stream, size_t length) {
    uint8_t  _version;
    uint32_t _flags;
    std::tie(_version, _flags) = boxParser::parseFullBox(stream);

    uint64_t      _creationTime;
    uint64_t      _modificationTime;
    uint32_t      _timeScale;
    uint32_t      _duration;
    std::string* _language;

    size_t parsedLength;

    if (_version == 1)
    {
        parsedLength = 30;
        _creationTime = stream->readBigEndianUnsignedLong();
        _modificationTime = stream->readBigEndianUnsignedLong();
    }
    else
    {
        parsedLength = 22;
        _creationTime = stream->readBigEndianUnsignedInteger();
        _modificationTime = stream->readBigEndianUnsignedInteger();
    }

    _timeScale = stream->readBigEndianUnsignedInteger();
    _duration = stream->readBigEndianUnsignedInteger();
    _language = stream->readBigEndianISO639Code();

    stream->ignore(length - parsedLength);

    std::cout << "version: " << _version << "\n";
    std::cout << "_flags: " << _flags << "\n";
    std::cout << "_creationTime: " << _creationTime << "\n";
    std::cout << "_modificationTime: " << _modificationTime << "\n";
    std::cout << "_timeScale: " << _timeScale << "\n";
    std::cout << "_duration: " << _duration << "\n";
    std::cout << "_language: " << _language << "\n";
}
void boxParser::parseFTYP(MP4::BinaryStream* stream, size_t length) {
    std::string _majorBrand;
    uint32_t _minorVersion;
    std::vector< std::string* > _compatibleBrands;

    std::string* s;
    char brand[5];

    memset(brand, 0, 5);
    stream->read(brand, 4);
    _majorBrand.append(brand);
    _minorVersion = stream->readBigEndianUnsignedInteger();

    if (length > 8)
    {
        length -= 8;

        while (length > 0)
        {
            stream->read(brand, 4);

            length -= 4;
            s = new std::string(brand);

            _compatibleBrands.push_back(s);
        }
    }
    std::cout << "_majorBrand: " << _majorBrand << "\n";
    std::cout << "_minorVersion: " << _minorVersion << "\n";
    for (auto b : _compatibleBrands) {
        std::cout << "_compatibleBrands: " << b << "\n";
    }
}
std::pair<int, int> boxParser::parseFullBox(MP4::BinaryStream* stream) {
    uint8_t  _version;
    uint32_t _flags;

    uint32_t data;

    data = stream->readBigEndianUnsignedInteger();

    _version = data >> 24;
    _flags = data & 0x00FFFFFF;

    return std::make_pair(_version, _flags);
}