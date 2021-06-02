#include "BinaryStream.h"

using namespace MP4;

BinaryStream::BinaryStream()
{
}

BinaryStream::~BinaryStream(void)
{
    if (this->stream.is_open())
    {
        this->stream.close();
    }
}
bool BinaryStream::open(const std::string& filename) {
    this->stream.open(filename, std::ios::binary | std::ios::in);

    if (this->stream.is_open() == false || this->stream.good() == false)
    {
        std::cout << "file open fail!!\n";
        return false;
    }
    return true;
}
uint8_t BinaryStream::readUnsignedChar(void)
{
    uint8_t n;

    this->read((char*)&n, 1);

    return n;
}


int8_t BinaryStream::readSignedChar(void)
{
    int8_t n;

    this->read((char*)&n, 1);

    return n;
}


uint16_t BinaryStream::readUnsignedShort(void)
{
    uint16_t n;

    this->read((char*)&n, 2);

    return n;
}

int16_t BinaryStream::readSignedShort(void)
{
    int16_t n;

    this->read((char*)&n, 2);

    return n;
}

uint16_t BinaryStream::readBigEndianUnsignedShort(void)
{
    uint8_t  c[2];
    uint16_t n;

    this->read((char*)c, 2);

    n = (uint16_t)c[0] << 8
        | (uint16_t)c[1];

    return n;
}

uint16_t BinaryStream::readLittleEndianUnsignedShort(void)
{
    uint8_t  c[2];
    uint16_t n;

    this->read((char*)c, 2);

    n = (uint16_t)c[1] << 8
        | (uint16_t)c[0];

    return n;
}


uint32_t BinaryStream::readUnsignedInteger(void)
{
    uint32_t n;

    this->read((char*)&n, 4);

    return n;
}

int32_t BinaryStream::readSignedInteger(void)
{
    int32_t n;

    this->read((char*)&n, 4);

    return n;
}

uint32_t BinaryStream::readBigEndianUnsignedInteger(void)
{
    uint8_t  c[4];
    uint32_t n;

    this->read((char*)c, 4);

    n = (uint32_t)c[0] << 24
        | (uint32_t)c[1] << 16
        | (uint32_t)c[2] << 8
        | (uint32_t)c[3];

    return n;
}

uint32_t BinaryStream::readLittleEndianUnsignedInteger(void)
{
    uint8_t  c[4];
    uint32_t n;

    this->read((char*)c, 4);

    n = (uint32_t)c[3] << 24
        | (uint32_t)c[2] << 16
        | (uint32_t)c[1] << 8
        | (uint32_t)c[0];

    return n;
}


uint64_t BinaryStream::readUnsignedLong(void)
{
    uint64_t n;

    this->read((char*)&n, 8);

    return n;
}

int64_t BinaryStream::readSignedLong(void)
{
    int64_t n;

    this->read((char*)&n, 8);

    return n;
}

uint64_t BinaryStream::readBigEndianUnsignedLong(void)
{
    uint8_t  c[8];
    uint64_t n;

    this->read((char*)c, 8);

    n = (uint64_t)c[0] << 56
        | (uint64_t)c[1] << 48
        | (uint64_t)c[2] << 40
        | (uint64_t)c[3] << 32
        | (uint64_t)c[4] << 24
        | (uint64_t)c[5] << 16
        | (uint64_t)c[6] << 8
        | (uint64_t)c[7];

    return n;
}

uint64_t BinaryStream::readLittleEndianUnsignedLong(void)
{
    uint8_t  c[8];
    uint64_t n;

    this->read((char*)c, 8);

    n = (uint64_t)c[7] << 56
        | (uint64_t)c[6] << 48
        | (uint64_t)c[5] << 40
        | (uint64_t)c[4] << 32
        | (uint64_t)c[3] << 24
        | (uint64_t)c[2] << 16
        | (uint64_t)c[1] << 8
        | (uint64_t)c[0];

    return n;
}

float BinaryStream::readFloat(void)
{
    uint8_t  exp;
    uint32_t mantissa;
    uint32_t binary;
    float    flt;
    bool     sign;
    int      i;

    binary = this->readBigEndianUnsignedInteger();
    sign = binary >> 31;
    exp = ((binary >> 23) & 0xFF);
    mantissa = (binary & 0x7FFFFF);

    if (exp == 0 && mantissa == 0)
    {
        return 0;
    }
    else if (exp == 255 && mantissa == 0)
    {
        return (sign == 0) ? INFINITY : -INFINITY;
    }
    else if (exp == 255 && mantissa != 0)
    {
        return NAN;
    }
    else if (exp == 0 && mantissa != 0)
    {
        exp = -126;
    }
    else
    {
        exp = exp - 127;
        mantissa = mantissa | 0x800000;
    }

    flt = 0;

    for (i = 0; i > -24; i--)
    {
        if (mantissa & (1 << (i + 23)))
        {
            flt += pow(2, i + exp);
        }
    }

    return (sign == 0) ? flt : -flt;
}

double BinaryStream::readDouble(void)
{
    uint16_t  exp;
    uint64_t mantissa;
    uint64_t binary;
    float    flt;
    bool     sign;
    int      i;

    binary = this->readBigEndianUnsignedLong();
    sign = binary >> 63;
    exp = ((binary >> 52) & 0x7FF);
    mantissa = (binary & 0xFFFFFFFFFFFFF);

    if (exp == 0 && mantissa == 0)
    {
        return 0;
    }
    else if (exp == 0x7FF && mantissa == 0)
    {
        return (sign == 0) ? INFINITY : -INFINITY;
    }
    else if (exp == 0x7FF && mantissa != 0)
    {
        return NAN;
    }
    else
    {
        exp = exp - 1023;
    }

    flt = 0;

    for (i = 0; i > -24; i--)
    {
        if (mantissa & (1 << (i + 23)))
        {
            flt += pow(2, i + exp);
        }
    }

    return (sign == 0) ? flt : -flt;
}

float BinaryStream::readBigEndianFixedPoint(unsigned int integerLength, unsigned int fractionalLength)
{
    uint32_t n;
    unsigned int integer;
    unsigned int fractionalMask;
    unsigned int fractional;

    if (integerLength + fractionalLength == 16)
    {
        n = this->readBigEndianUnsignedShort();
    }
    else
    {
        n = this->readBigEndianUnsignedInteger();
    }

    integer = n >> fractionalLength;
    fractionalMask = pow(2, fractionalLength) - 1;
    fractional = (n & fractionalMask) / (1 << fractionalLength);

    return integer + fractional;
}

float BinaryStream::readLittleEndianFixedPoint(unsigned int integerLength, unsigned int fractionalLength)
{
    uint32_t n;
    unsigned int integer;
    unsigned int fractionalMask;
    unsigned int fractional;

    if (integerLength + fractionalLength == 16)
    {
        n = this->readLittleEndianUnsignedShort();
    }
    else
    {
        n = this->readLittleEndianUnsignedInteger();
    }

    integer = n >> fractionalLength;
    fractionalMask = pow(2, fractionalLength) - 1;
    fractional = (n & fractionalMask) / (1 << fractionalLength);

    return integer + fractional;
}

std::string* BinaryStream::readBigEndianISO639Code(void)
{
    unsigned char c1;
    unsigned char c2;
    unsigned char c3;
    uint16_t      n;
    std::string* s;

    s = new std::string();
    n = this->readBigEndianUnsignedShort();

    c1 = (n & 0x7C00) >> 10;  // Mask is 0111 1100 0000 0000
    c2 = (n & 0x03E0) >> 5;   // Mask is 0000 0011 1110 0000
    c3 = (n & 0x001F);        // Mask is 0000 0000 0001 1111

    c1 += 0x60;
    c2 += 0x60;
    c3 += 0x60;

    s->append((char*)&c1, 1);
    s->append((char*)&c2, 1);
    s->append((char*)&c3, 1);

    return s;
}

std::string* BinaryStream::readNULLTerminatedString(void)
{
    unsigned char c;
    std::string* s;

    this->get((char*)&c, 1);

    s = new std::string;

    while (c != 0)
    {
        s->append(1, c);

        this->get((char*)&c, 1);
    }

    return s;
}

std::string* BinaryStream::readUTF8String(void)
{
    unsigned char c;
    uint16_t      length;
    uint16_t      i;
    std::string* s;

    length = this->readBigEndianUnsignedShort();
    s = new std::string;

    for (i = 0; i < length; i++)
    {
        this->get((char*)&c, 1);
        s->append(1, c);
    }

    return NULL;
}

std::string* BinaryStream::readLongUTF8String(void)
{
    unsigned char c;
    uint32_t      length;
    uint32_t      i;
    std::string* s;

    length = this->readBigEndianUnsignedInteger();
    s = new std::string;

    for (i = 0; i < length; i++)
    {
        this->get((char*)&c, 1);
        s->append(1, c);
    }

    return NULL;
}
std::string BinaryStream::readString(int size) {
    char* ret = new char[size + 1];
    memset(ret, 0x0, size + 1);
    this->read(ret, size);
    return ret;
}
bool BinaryStream::good(void) const
{
    return stream.good();
}

bool BinaryStream::eof(void) const
{
    return stream.eof();
}

bool BinaryStream::fail(void) const
{
    return stream.fail();
}

bool BinaryStream::bad(void) const
{
    return stream.bad();
}

int BinaryStream::peek(void)
{
    return stream.peek();
}

int BinaryStream::get(void)
{
    return stream.get();
}

int BinaryStream::sync(void)
{
    return stream.sync();
}

std::streampos BinaryStream::tellg(void)
{
    return stream.tellg();
}

std::streamsize BinaryStream::readsome(char* s, std::streamsize n)
{
    return stream.readsome(s, n);
}

std::streamsize BinaryStream::gcount(void) const
{
    return stream.gcount();
}

std::istream& BinaryStream::get(char& c)
{
    return stream.get(c);
}

std::istream& BinaryStream::get(char* s, std::streamsize n)
{
    return stream.get(s, n);
}

std::istream& BinaryStream::get(char* s, std::streamsize n, char delim)
{
    return stream.get(s, n, delim);
}

std::istream& BinaryStream::get(std::streambuf& sb)
{
    return stream.get(sb);
}

std::istream& BinaryStream::get(std::streambuf& sb, char delim)
{
    return stream.get(sb, delim);
}

std::istream& BinaryStream::getline(char* s, std::streamsize n)
{
    return stream.getline(s, n);
}

std::istream& BinaryStream::getline(char* s, std::streamsize n, char delim)
{
    return stream.getline(s, n, delim);
}

std::istream& BinaryStream::ignore(std::streamsize n, int delim)
{
    return stream.ignore(n, delim);
}

std::istream& BinaryStream::read(char* s, std::streamsize n)
{
    return stream.read(s, n);
}

std::istream& BinaryStream::putback(char c)
{
    return stream.putback(c);
}

std::istream& BinaryStream::unget(void)
{
    return stream.unget();
}

std::istream& BinaryStream::seekg(std::streampos pos)
{
    return stream.seekg(pos);
}

std::istream& BinaryStream::seekg(std::streamoff off, std::ios_base::seekdir dir)
{
    return stream.seekg(off, dir);
}

void BinaryStream::clear(void)
{
    return stream.clear();
}
void BinaryStream::readMatrix(matrix* m)
{
    m->a = this->readBigEndianFixedPoint(16, 16);
    m->b = this->readBigEndianFixedPoint(16, 16);
    m->u = this->readBigEndianFixedPoint(2, 30);
    m->c = this->readBigEndianFixedPoint(16, 16);
    m->d = this->readBigEndianFixedPoint(16, 16);
    m->v = this->readBigEndianFixedPoint(2, 30);
    m->x = this->readBigEndianFixedPoint(16, 16);
    m->y = this->readBigEndianFixedPoint(16, 16);
    m->w = this->readBigEndianFixedPoint(2, 30);
}