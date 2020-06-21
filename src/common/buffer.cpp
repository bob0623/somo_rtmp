#include "buffer.h"

#include <string.h>

IOBuffer::IOBuffer()
{
    p = bytes = NULL;
    nb_bytes = 0;
    
    // TODO: support both little and big endian.
    //srs_assert(srs_is_little_endian());
}

IOBuffer::IOBuffer(char* b, int nb_b)
{
    p = bytes = b;
    nb_bytes = nb_b;
    
    // TODO: support both little and big endian.
    //srs_assert(srs_is_little_endian());
}

IOBuffer::~IOBuffer()
{
}

int IOBuffer::size()
{
    return nb_bytes;
}

int IOBuffer::pos()
{
    return (int)(p - bytes);
}

void IOBuffer::repos(int pos) {
    p = bytes + pos;
}

int IOBuffer::left()
{
    return nb_bytes - (int)(p - bytes);
}

bool IOBuffer::empty()
{
    return !bytes || (p >= bytes + nb_bytes);
}

bool IOBuffer::require(int required_size)
{
    //srs_assert(required_size >= 0);
    
    return required_size <= nb_bytes - (p - bytes);
}

void IOBuffer::skip(int size)
{
    //srs_assert(p);
    //srs_assert(p + size >= bytes);
    //srs_assert(p + size <= bytes + nb_bytes);
    
    p += size;
}

int8_t IOBuffer::read_1bytes()
{
    //srs_assert(require(1));
    
    return (int8_t)*p++;
}

int16_t IOBuffer::read_2bytes()
{
    //srs_assert(require(2));
    
    int16_t value;
    char* pp = (char*)&value;
    pp[1] = *p++;
    pp[0] = *p++;
    
    return value;
}

int32_t IOBuffer::read_3bytes()
{
    //srs_assert(require(3));
    
    int32_t value = 0x00;
    char* pp = (char*)&value;
    pp[2] = *p++;
    pp[1] = *p++;
    pp[0] = *p++;
    
    return value;
}

int32_t IOBuffer::read_4bytes()
{
    //srs_assert(require(4));
    
    int32_t value;
    char* pp = (char*)&value;
    pp[3] = *p++;
    pp[2] = *p++;
    pp[1] = *p++;
    pp[0] = *p++;
    
    return value;
}

int64_t IOBuffer::read_8bytes()
{
    //srs_assert(require(8));
    
    int64_t value;
    char* pp = (char*)&value;
    pp[7] = *p++;
    pp[6] = *p++;
    pp[5] = *p++;
    pp[4] = *p++;
    pp[3] = *p++;
    pp[2] = *p++;
    pp[1] = *p++;
    pp[0] = *p++;
    
    return value;
}

std::string IOBuffer::read_string(int len)
{
    //srs_assert(require(len));
    
    std::string value;
    value.append(p, len);
    
    p += len;
    
    return value;
}

void IOBuffer::read_bytes(char* data, int size)
{
    //srs_assert(require(size));
    
    memcpy(data, p, size);
    
    p += size;
}

void IOBuffer::write_1bytes(int8_t value)
{
    //srs_assert(require(1));
    
    *p++ = value;
}

void IOBuffer::write_2bytes(int16_t value)
{
    //srs_assert(require(2));
    
    char* pp = (char*)&value;
    *p++ = pp[1];
    *p++ = pp[0];
}

void IOBuffer::write_4bytes(int32_t value)
{
    //srs_assert(require(4));
    
    char* pp = (char*)&value;
    *p++ = pp[3];
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
}

void IOBuffer::write_3bytes(int32_t value)
{
    //srs_assert(require(3));
    
    char* pp = (char*)&value;
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
}

void IOBuffer::write_8bytes(int64_t value)
{
    //srs_assert(require(8));
    
    char* pp = (char*)&value;
    *p++ = pp[7];
    *p++ = pp[6];
    *p++ = pp[5];
    *p++ = pp[4];
    *p++ = pp[3];
    *p++ = pp[2];
    *p++ = pp[1];
    *p++ = pp[0];
}

void IOBuffer::write_string(std::string value)
{
    //srs_assert(require((int)value.length()));
    
    memcpy(p, value.data(), value.length());
    p += value.length();
}

void IOBuffer::write_bytes(char* data, int size)
{
    //srs_assert(require(size));
    
    memcpy(p, data, size);
    p += size;
}