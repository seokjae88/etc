#ifndef _MP4_BINARY_STREAM_
#pragma once
#define _MP4_BINARY_STREAM_

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "math.h"
#include "stdint.h"

typedef struct _matrix
{
    float a;
    float b;
    float u;
    float c;
    float d;
    float v;
    float x;
    float y;
    float w;
}
matrix;

namespace MP4
{
    class BinaryStream
    {
    private:


    protected:
        std::ifstream stream;

    public:
        BinaryStream();
        ~BinaryStream(void);

        uint8_t readUnsignedChar(void);
        int8_t readSignedChar(void);

        uint16_t readUnsignedShort(void);
        int16_t readSignedShort(void);
        uint16_t readBigEndianUnsignedShort(void);
        uint16_t readLittleEndianUnsignedShort(void);

        uint32_t readUnsignedInteger(void);
        int32_t readSignedInteger(void);
        uint32_t readBigEndianUnsignedInteger(void);
        uint32_t readLittleEndianUnsignedInteger(void);

        uint64_t readUnsignedLong(void);
        int64_t readSignedLong(void);
        uint64_t readBigEndianUnsignedLong(void);
        uint64_t readLittleEndianUnsignedLong(void);

        float readFloat(void);
        double readDouble(void);

        float readBigEndianFixedPoint(unsigned int integerLength, unsigned int fractionalLength);
        float readLittleEndianFixedPoint(unsigned int integerLength, unsigned int fractionalLength);

        std::string* readBigEndianISO639Code(void);
        std::string* readNULLTerminatedString(void);
        std::string* readUTF8String(void);
        std::string* readLongUTF8String(void);
        std::string readString(int size);

        bool good(void) const;
        bool eof(void) const;
        bool fail(void) const;
        bool bad(void) const;
        int peek(void);
        int get(void);
        int sync(void);
        std::streampos tellg(void);
        std::streamsize readsome(char* s, std::streamsize n);
        std::streamsize gcount(void) const;
        std::istream& get(char& c);
        std::istream& get(char* s, std::streamsize n);
        std::istream& get(char* s, std::streamsize n, char delim);
        std::istream& get(std::streambuf& sb);
        std::istream& get(std::streambuf& sb, char delim);
        std::istream& getline(char* s, std::streamsize n);
        std::istream& getline(char* s, std::streamsize n, char delim);
        std::istream& ignore(std::streamsize n = 1, int delim = EOF);
        std::istream& read(char* s, std::streamsize n);
        std::istream& putback(char c);
        std::istream& unget(void);
        std::istream& seekg(std::streampos pos);
        std::istream& seekg(std::streamoff off, std::ios_base::seekdir dir);
        bool open(const std::string& filename);
        void clear(void);
        void readMatrix(matrix* m);
    };
}

#endif /* MP4_BINARY_STREAM_ */