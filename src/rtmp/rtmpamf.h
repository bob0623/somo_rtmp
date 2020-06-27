#pragma once

/**
 * This file is copy from SRS(https://github.com/ossrs/srs)
 * 
 */

#include <string>
#include <vector>

class IOBuffer;
class RtmpAmf0Object;
class RtmpAmf0EcmaArray;
class RtmpAmf0StrictArray;
class RtmpJsonAny;
class RtmpUnSortedHashtable;
class RtmpAmf0ObjectEOF;
class RtmpAmf0Date;

class RtmpAmf0Any
{
public:
    RtmpAmf0Any();
    virtual ~RtmpAmf0Any();

public:
    virtual bool is_string();
    virtual bool is_boolean();
    virtual bool is_number();
    virtual bool is_null();
    virtual bool is_undefined();
    virtual bool is_object();
    virtual bool is_object_eof();
    virtual bool is_ecma_array();
    virtual bool is_strict_array();
    virtual bool is_date();
    virtual bool is_complex_object();

public:
    virtual std::string to_str();
    virtual const char* to_str_raw();
    virtual bool to_boolean();
    virtual double to_number();
    virtual int64_t to_date();
    virtual int16_t to_date_time_zone();
    virtual RtmpAmf0Object* to_object();
    virtual RtmpAmf0EcmaArray* to_ecma_array();
    virtual RtmpAmf0StrictArray* to_strict_array();

public:
    virtual void set_number(double value);

public:
    virtual int total_size() = 0;
    virtual bool read(IOBuffer* stream) = 0;
    virtual bool write(IOBuffer* stream) = 0;
    virtual RtmpAmf0Any* copy() = 0;
    virtual char* human_print(char** pdata, int* psize);
public:
    static RtmpAmf0Any* str(const char* value = NULL);
    static RtmpAmf0Any* boolean(bool value = false);
    static RtmpAmf0Any* number(double value = 0.0);
    static RtmpAmf0Any* date(int64_t value = 0);
    static RtmpAmf0Any* null();
    static RtmpAmf0Any* undefined();
    static RtmpAmf0Object* object();
    static RtmpAmf0Any* object_eof();
    static RtmpAmf0EcmaArray* ecma_array();
    static RtmpAmf0StrictArray* strict_array();

public:
    static bool discovery(IOBuffer* stream, RtmpAmf0Any** ppvalue);

protected:
    char marker;
};

class RtmpAmf0Object : public RtmpAmf0Any
{
private:
    RtmpUnSortedHashtable* properties;
    RtmpAmf0ObjectEOF* eof;
private:
    friend class RtmpAmf0Any;
    RtmpAmf0Object();
public:
    virtual ~RtmpAmf0Object();
public:
    virtual int total_size();
    virtual bool read(IOBuffer* stream);
    virtual bool write(IOBuffer* stream);
    virtual RtmpAmf0Any* copy();

public:
    virtual void clear();
    virtual int count();
    virtual std::string key_at(int index);
    virtual const char* key_raw_at(int index);
    virtual RtmpAmf0Any* value_at(int index);

public:
    virtual void set(std::string key, RtmpAmf0Any* value);
    virtual RtmpAmf0Any* get_property(std::string name);
    virtual RtmpAmf0Any* ensure_property_string(std::string name);
    virtual RtmpAmf0Any* ensure_property_number(std::string name);
    virtual void remove(std::string name);
};

class RtmpAmf0EcmaArray : public RtmpAmf0Any
{
private:
    RtmpUnSortedHashtable* properties;
    RtmpAmf0ObjectEOF* eof;
    int32_t _count;
private:
    friend class RtmpAmf0Any;
    RtmpAmf0EcmaArray();
public:
    virtual ~RtmpAmf0EcmaArray();
    // serialize/deserialize to/from stream.
public:
    virtual int total_size();
    virtual bool read(IOBuffer* stream);
    virtual bool write(IOBuffer* stream);
    virtual RtmpAmf0Any* copy();

public:
    virtual void clear();
    virtual int count();
    virtual std::string key_at(int index);
    virtual const char* key_raw_at(int index);
    virtual RtmpAmf0Any* value_at(int index);

public:
    virtual void set(std::string key, RtmpAmf0Any* value);
    virtual RtmpAmf0Any* get_property(std::string name);
    virtual RtmpAmf0Any* ensure_property_string(std::string name);
    virtual RtmpAmf0Any* ensure_property_number(std::string name);
};

class RtmpAmf0StrictArray : public RtmpAmf0Any
{
private:
    std::vector<RtmpAmf0Any*> properties;
    int32_t _count;
private:
    friend class RtmpAmf0Any;

    RtmpAmf0StrictArray();
public:
    virtual ~RtmpAmf0StrictArray();

public:
    virtual int total_size();
    virtual bool read(IOBuffer* stream);
    virtual bool write(IOBuffer* stream);
    virtual RtmpAmf0Any* copy();

public:
    virtual void clear();
    virtual int count();
    virtual RtmpAmf0Any* at(int index);

public:
    virtual void append(RtmpAmf0Any* any);
};

class RtmpAmf0Size
{
public:
    static int utf8(std::string value);
    static int str(std::string value);
    static int number();
    static int date();
    static int null();
    static int undefined();
    static int boolean();
    static int object(RtmpAmf0Object* obj);
    static int object_eof();
    static int ecma_array(RtmpAmf0EcmaArray* arr);
    static int strict_array(RtmpAmf0StrictArray* arr);
    static int any(RtmpAmf0Any* o);
};

extern bool rtmp_amf0_read_any(IOBuffer* stream, RtmpAmf0Any** ppvalue);
extern bool rtmp_amf0_read_string(IOBuffer* stream, std::string& value);
extern bool rtmp_amf0_write_string(IOBuffer* stream, std::string value);
extern bool rtmp_amf0_read_boolean(IOBuffer* stream, bool& value);
extern bool rtmp_amf0_write_boolean(IOBuffer* stream, bool value);
extern bool rtmp_amf0_read_number(IOBuffer* stream, double& value);
extern bool rtmp_amf0_write_number(IOBuffer* stream, double value);
extern bool rtmp_amf0_read_null(IOBuffer* stream);
extern bool rtmp_amf0_write_null(IOBuffer* stream);
extern bool rtmp_amf0_read_undefined(IOBuffer* stream);
extern bool rtmp_amf0_write_undefined(IOBuffer* stream);

class RtmpAmf0String : public RtmpAmf0Any
{
public:
    std::string value;
private:
    friend class RtmpAmf0Any;
    RtmpAmf0String(const char* _value);
public:
    virtual ~RtmpAmf0String();
public:
    virtual int total_size();
    virtual bool read(IOBuffer* stream);
    virtual bool write(IOBuffer* stream);
    virtual RtmpAmf0Any* copy();
};

class RtmpAmf0Boolean : public RtmpAmf0Any
{
public:
    bool value;
private:
    friend class RtmpAmf0Any;
    RtmpAmf0Boolean(bool _value);
public:
    virtual ~RtmpAmf0Boolean();
public:
    virtual int total_size();
    virtual bool read(IOBuffer* stream);
    virtual bool write(IOBuffer* stream);
    virtual RtmpAmf0Any* copy();
};

class RtmpAmf0Number : public RtmpAmf0Any
{
public:
    double value;
private:
    friend class RtmpAmf0Any;

    RtmpAmf0Number(double _value);
public:
    virtual ~RtmpAmf0Number();
public:
    virtual int total_size();
    virtual bool read(IOBuffer* stream);
    virtual bool write(IOBuffer* stream);
    virtual RtmpAmf0Any* copy();
};

class RtmpAmf0Date : public RtmpAmf0Any
{
private:
    int64_t _date_value;
    int16_t _time_zone;
private:
    friend class RtmpAmf0Any;

    RtmpAmf0Date(int64_t value);
public:
    virtual ~RtmpAmf0Date();

public:
    virtual int total_size();
    virtual bool read(IOBuffer* stream);
    virtual bool write(IOBuffer* stream);
    virtual RtmpAmf0Any* copy();
public:
    virtual int64_t date();
    virtual int16_t time_zone();
};

class RtmpAmf0Null : public RtmpAmf0Any
{
private:
    friend class RtmpAmf0Any;
    RtmpAmf0Null();
public:
    virtual ~RtmpAmf0Null();
public:
    virtual int total_size();
    virtual bool read(IOBuffer* stream);
    virtual bool write(IOBuffer* stream);
    virtual RtmpAmf0Any* copy();
};

class RtmpAmf0Undefined : public RtmpAmf0Any
{
private:
    friend class RtmpAmf0Any;
    RtmpAmf0Undefined();
public:
    virtual ~RtmpAmf0Undefined();
public:
    virtual int total_size();
    virtual bool read(IOBuffer* stream);
    virtual bool write(IOBuffer* stream);
    virtual RtmpAmf0Any* copy();
};

class RtmpUnSortedHashtable
{
private:
    typedef std::pair<std::string, RtmpAmf0Any*> SrsAmf0ObjectPropertyType;
    std::vector<SrsAmf0ObjectPropertyType> properties;
public:
    RtmpUnSortedHashtable();
    virtual ~RtmpUnSortedHashtable();
public:
    virtual int count();
    virtual void clear();
    virtual std::string key_at(int index);
    virtual const char* key_raw_at(int index);
    virtual RtmpAmf0Any* value_at(int index);
    virtual void set(std::string key, RtmpAmf0Any* value);
public:
    virtual RtmpAmf0Any* get_property(std::string name);
    virtual RtmpAmf0Any* ensure_property_string(std::string name);
    virtual RtmpAmf0Any* ensure_property_number(std::string name);
    virtual void remove(std::string name);
public:
    virtual void copy(RtmpUnSortedHashtable* src);
};

class RtmpAmf0ObjectEOF : public RtmpAmf0Any
{
public:
    RtmpAmf0ObjectEOF();
    virtual ~RtmpAmf0ObjectEOF();
public:
    virtual int total_size();
    virtual bool read(IOBuffer* stream);
    virtual bool write(IOBuffer* stream);
    virtual RtmpAmf0Any* copy();
};

extern bool rtmp_amf0_read_utf8(IOBuffer* stream, std::string& value);
extern bool rtmp_amf0_write_utf8(IOBuffer* stream, std::string value);

extern bool rtmp_amf0_is_object_eof(IOBuffer* stream);
extern bool rtmp_amf0_write_object_eof(IOBuffer* stream, RtmpAmf0ObjectEOF* value);

extern bool rtmp_amf0_write_any(IOBuffer* stream, RtmpAmf0Any* value);

