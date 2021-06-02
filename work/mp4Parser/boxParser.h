#pragma once

#include <iostream>
#include <fstream>
#include <iomanip> 

#include "BinaryStream.h"
#include "log.h"
#include "trak.hpp"

namespace boxParser {
    std::pair<int, int> parseFullBox(MP4::BinaryStream* stream);
    void parseSTSC(MP4::BinaryStream* stream, size_t length, trak* trakData);
    void parseSTCO(MP4::BinaryStream* stream, size_t length, trak* trakData);
    void parseMVHD(MP4::BinaryStream* stream, size_t length);
    void parseMDHD(MP4::BinaryStream* stream, size_t length);
    void parseFTYP(MP4::BinaryStream* stream, size_t length);
    void parseSTSD(MP4::BinaryStream* stream, size_t length, trak* trakData);
    void parseSTSZ(MP4::BinaryStream* stream, size_t length, trak* trakData);
    void parseAVCC(MP4::BinaryStream* stream, size_t length, trak* trakData);
    void parseESDS(MP4::BinaryStream* stream, size_t length, trak* trakData);
}