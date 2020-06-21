#include "rtmpamf.h"
#include "common/buffer.h"

#include <utility>
#include <vector>
#include <sstream>
#include <string.h>

// AMF0 marker
#define RTMP_AMF0_Number                     0x00
#define RTMP_AMF0_Boolean                     0x01
#define RTMP_AMF0_String                     0x02
#define RTMP_AMF0_Object                     0x03
#define RTMP_AMF0_MovieClip                 0x04 // reserved, not supported
#define RTMP_AMF0_Null                         0x05
#define RTMP_AMF0_Undefined                 0x06
#define RTMP_AMF0_Reference                 0x07
#define RTMP_AMF0_EcmaArray                 0x08
#define RTMP_AMF0_ObjectEnd                 0x09
#define RTMP_AMF0_StrictArray                 0x0A
#define RTMP_AMF0_Date                         0x0B
#define RTMP_AMF0_LongString                 0x0C
#define RTMP_AMF0_UnSupported                 0x0D
#define RTMP_AMF0_RecordSet                 0x0E // reserved, not supported
#define RTMP_AMF0_XmlDocument                 0x0F
#define RTMP_AMF0_TypedObject                 0x10
// AVM+ object is the AMF3 object.
#define RTMP_AMF0_AVMplusObject             0x11
// origin array whos data takes the same form as LengthValueBytes
#define RTMP_AMF0_OriginStrictArray         0x20

// User defined
#define RTMP_AMF0_Invalid                     0x3F

RtmpAmf0Any::RtmpAmf0Any()
{
    marker = RTMP_AMF0_Invalid;
}

RtmpAmf0Any::~RtmpAmf0Any()
{
}

bool RtmpAmf0Any::is_string()
{
    return marker == RTMP_AMF0_String;
}

bool RtmpAmf0Any::is_boolean()
{
    return marker == RTMP_AMF0_Boolean;
}

bool RtmpAmf0Any::is_number()
{
    return marker == RTMP_AMF0_Number;
}

bool RtmpAmf0Any::is_null()
{
    return marker == RTMP_AMF0_Null;
}

bool RtmpAmf0Any::is_undefined()
{
    return marker == RTMP_AMF0_Undefined;
}

bool RtmpAmf0Any::is_object()
{
    return marker == RTMP_AMF0_Object;
}

bool RtmpAmf0Any::is_ecma_array()
{
    return marker == RTMP_AMF0_EcmaArray;
}

bool RtmpAmf0Any::is_strict_array()
{
    return marker == RTMP_AMF0_StrictArray;
}

bool RtmpAmf0Any::is_date()
{
    return marker == RTMP_AMF0_Date;
}

bool RtmpAmf0Any::is_complex_object()
{
    return is_object() || is_object_eof() || is_ecma_array() || is_strict_array();
}

std::string RtmpAmf0Any::to_str()
{
    RtmpAmf0String* p = dynamic_cast<RtmpAmf0String*>(this);
    //srs_assert(p != NULL);
    return p->value;
}

const char* RtmpAmf0Any::to_str_raw()
{
    RtmpAmf0String* p = dynamic_cast<RtmpAmf0String*>(this);
    //srs_assert(p != NULL);
    return p->value.data();
}

bool RtmpAmf0Any::to_boolean()
{
    RtmpAmf0Boolean* p = dynamic_cast<RtmpAmf0Boolean*>(this);
    //srs_assert(p != NULL);
    return p->value;
}

double RtmpAmf0Any::to_number()
{
    RtmpAmf0Number* p = dynamic_cast<RtmpAmf0Number*>(this);
    //srs_assert(p != NULL);
    return p->value;
}

int64_t RtmpAmf0Any::to_date()
{
    RtmpAmf0Date* p = dynamic_cast<RtmpAmf0Date*>(this);
    //srs_assert(p != NULL);
    return p->date();
}

int16_t RtmpAmf0Any::to_date_time_zone()
{
    RtmpAmf0Date* p = dynamic_cast<RtmpAmf0Date*>(this);
    //srs_assert(p != NULL);
    return p->time_zone();
}

RtmpAmf0Object* RtmpAmf0Any::to_object()
{
    RtmpAmf0Object* p = dynamic_cast<RtmpAmf0Object*>(this);
    //srs_assert(p != NULL);
    return p;
}

RtmpAmf0EcmaArray* RtmpAmf0Any::to_ecma_array()
{
    RtmpAmf0EcmaArray* p = dynamic_cast<RtmpAmf0EcmaArray*>(this);
    //srs_assert(p != NULL);
    return p;
}

RtmpAmf0StrictArray* RtmpAmf0Any::to_strict_array()
{
    RtmpAmf0StrictArray* p = dynamic_cast<RtmpAmf0StrictArray*>(this);
    //srs_assert(p != NULL);
    return p;
}

void RtmpAmf0Any::set_number(double value)
{
    RtmpAmf0Number* p = dynamic_cast<RtmpAmf0Number*>(this);
    //srs_assert(p != NULL);
    p->value = value;
}

bool RtmpAmf0Any::is_object_eof()
{
    return marker == RTMP_AMF0_ObjectEnd;
}

void srs_fill_level_spaces(std::stringstream& ss, int level)
{
    for (int i = 0; i < level; i++) {
        ss << "    ";
    }
}
void srs_amf0_do_print(RtmpAmf0Any* any, std::stringstream& ss, int level)
{
    std::ios_base::fmtflags oflags = ss.flags();
    
    if (any->is_boolean()) {
        ss << "Boolean " << (any->to_boolean()? "true":"false") << std::endl;
    } else if (any->is_number()) {
        ss << "Number " << std::fixed << any->to_number() << std::endl;
    } else if (any->is_string()) {
        ss << "String " << any->to_str() << std::endl;
    } else if (any->is_date()) {
        ss << "Date " << std::hex << any->to_date()
        << "/" << std::hex << any->to_date_time_zone() << std::endl;
    } else if (any->is_null()) {
        ss << "Null" << std::endl;
    } else if (any->is_undefined()) {
        ss << "Undefined" <<std::endl;
    } else if (any->is_ecma_array()) {
        RtmpAmf0EcmaArray* obj = any->to_ecma_array();
        ss << "EcmaArray " << "(" << obj->count() << " items)" << std::endl;
        for (int i = 0; i < obj->count(); i++) {
            srs_fill_level_spaces(ss, level + 1);
            ss << "Elem '" << obj->key_at(i) << "' ";
            if (obj->value_at(i)->is_complex_object()) {
                srs_amf0_do_print(obj->value_at(i), ss, level + 1);
            } else {
                srs_amf0_do_print(obj->value_at(i), ss, 0);
            }
        }
    } else if (any->is_strict_array()) {
        RtmpAmf0StrictArray* obj = any->to_strict_array();
        ss << "StrictArray " << "(" << obj->count() << " items)" << std::endl;
        for (int i = 0; i < obj->count(); i++) {
            srs_fill_level_spaces(ss, level + 1);
            ss << "Elem ";
            if (obj->at(i)->is_complex_object()) {
                srs_amf0_do_print(obj->at(i), ss, level + 1);
            } else {
                srs_amf0_do_print(obj->at(i), ss, 0);
            }
        }
    } else if (any->is_object()) {
        RtmpAmf0Object* obj = any->to_object();
        ss << "Object " << "(" << obj->count() << " items)" <<std::endl;
        for (int i = 0; i < obj->count(); i++) {
            srs_fill_level_spaces(ss, level + 1);
            ss << "Property '" << obj->key_at(i) << "' ";
            if (obj->value_at(i)->is_complex_object()) {
                srs_amf0_do_print(obj->value_at(i), ss, level + 1);
            } else {
                srs_amf0_do_print(obj->value_at(i), ss, 0);
            }
        }
    } else {
        ss << "Unknown" << std::endl;
    }
    
    ss.flags(oflags);
}

char* RtmpAmf0Any::human_print(char** pdata, int* psize)
{
    std::stringstream ss;
    
    ss.precision(1);
    
    srs_amf0_do_print(this, ss, 0);
    
    std::string str = ss.str();
    if (str.empty()) {
        return NULL;
    }
    
    char* data = new char[str.length() + 1];
    memcpy(data, str.data(), str.length());
    data[str.length()] = 0;
    
    if (pdata) {
        *pdata = data;
    }
    if (psize) {
        *psize = (int)str.length();
    }
    
    return data;
}

RtmpAmf0Any* RtmpAmf0Any::str(const char* value)
{
    return new RtmpAmf0String(value);
}

RtmpAmf0Any* RtmpAmf0Any::boolean(bool value)
{
    return new RtmpAmf0Boolean(value);
}

RtmpAmf0Any* RtmpAmf0Any::number(double value)
{
    return new RtmpAmf0Number(value);
}

RtmpAmf0Any* RtmpAmf0Any::null()
{
    return new RtmpAmf0Null();
}

RtmpAmf0Any* RtmpAmf0Any::undefined()
{
    return new RtmpAmf0Undefined();
}

RtmpAmf0Object* RtmpAmf0Any::object()
{
    return new RtmpAmf0Object();
}

RtmpAmf0Any* RtmpAmf0Any::object_eof()
{
    return new RtmpAmf0ObjectEOF();
}

RtmpAmf0EcmaArray* RtmpAmf0Any::ecma_array()
{
    return new RtmpAmf0EcmaArray();
}

RtmpAmf0StrictArray* RtmpAmf0Any::strict_array()
{
    return new RtmpAmf0StrictArray();
}

RtmpAmf0Any* RtmpAmf0Any::date(int64_t value)
{
    return new RtmpAmf0Date(value);
}

bool RtmpAmf0Any::discovery(IOBuffer* stream, RtmpAmf0Any** ppvalue)
{
    bool err = true;

    // detect the object-eof specially
    if (rtmp_amf0_is_object_eof(stream)) {
        *ppvalue = new RtmpAmf0ObjectEOF();
        return false;
    }
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    char marker = stream->read_1bytes();
    
    // backward the 1byte marker.
    stream->skip(-1);
    
    switch (marker) {
        case RTMP_AMF0_String: {
            *ppvalue = RtmpAmf0Any::str();
            return err;
        }
        case RTMP_AMF0_Boolean: {
            *ppvalue = RtmpAmf0Any::boolean();
            return err;
        }
        case RTMP_AMF0_Number: {
            *ppvalue = RtmpAmf0Any::number();
            return err;
        }
        case RTMP_AMF0_Null: {
            *ppvalue = RtmpAmf0Any::null();
            return err;
        }
        case RTMP_AMF0_Undefined: {
            *ppvalue = RtmpAmf0Any::undefined();
            return err;
        }
        case RTMP_AMF0_Object: {
            *ppvalue = RtmpAmf0Any::object();
            return err;
        }
        case RTMP_AMF0_EcmaArray: {
            *ppvalue = RtmpAmf0Any::ecma_array();
            return err;
        }
        case RTMP_AMF0_StrictArray: {
            *ppvalue = RtmpAmf0Any::strict_array();
            return err;
        }
        case RTMP_AMF0_Date: {
            *ppvalue = RtmpAmf0Any::date();
            return err;
        }
        case RTMP_AMF0_Invalid:
        default: {
            return false;
        }
    }
}

RtmpUnSortedHashtable::RtmpUnSortedHashtable()
{
}

RtmpUnSortedHashtable::~RtmpUnSortedHashtable()
{
    clear();
}

int RtmpUnSortedHashtable::count()
{
    return (int)properties.size();
}

void RtmpUnSortedHashtable::clear()
{
    std::vector<SrsAmf0ObjectPropertyType>::iterator it;
    for (it = properties.begin(); it != properties.end(); ++it) {
        SrsAmf0ObjectPropertyType& elem = *it;
        RtmpAmf0Any* any = elem.second;
        free(any);
    }
    properties.clear();
}

std::string RtmpUnSortedHashtable::key_at(int index)
{
    //srs_assert(index < count());
    SrsAmf0ObjectPropertyType& elem = properties[index];
    return elem.first;
}

const char* RtmpUnSortedHashtable::key_raw_at(int index)
{
    //srs_assert(index < count());
    SrsAmf0ObjectPropertyType& elem = properties[index];
    return elem.first.data();
}

RtmpAmf0Any* RtmpUnSortedHashtable::value_at(int index)
{
    //srs_assert(index < count());
    SrsAmf0ObjectPropertyType& elem = properties[index];
    return elem.second;
}

void RtmpUnSortedHashtable::set(std::string key, RtmpAmf0Any* value)
{
    std::vector<SrsAmf0ObjectPropertyType>::iterator it;
    
    for (it = properties.begin(); it != properties.end(); ++it) {
        SrsAmf0ObjectPropertyType& elem = *it;
        std::string name = elem.first;
        RtmpAmf0Any* any = elem.second;
        
        if (key == name) {
            free(any);
            properties.erase(it);
            break;
        }
    }
    
    if (value) {
        properties.push_back(std::make_pair(key, value));
    }
}

RtmpAmf0Any* RtmpUnSortedHashtable::get_property(std::string name)
{
    std::vector<SrsAmf0ObjectPropertyType>::iterator it;
    
    for (it = properties.begin(); it != properties.end(); ++it) {
        SrsAmf0ObjectPropertyType& elem = *it;
        std::string key = elem.first;
        RtmpAmf0Any* any = elem.second;
        if (key == name) {
            return any;
        }
    }
    
    return NULL;
}

RtmpAmf0Any* RtmpUnSortedHashtable::ensure_property_string(std::string name)
{
    RtmpAmf0Any* prop = get_property(name);
    
    if (!prop) {
        return NULL;
    }
    
    if (!prop->is_string()) {
        return NULL;
    }
    
    return prop;
}

RtmpAmf0Any* RtmpUnSortedHashtable::ensure_property_number(std::string name)
{
    RtmpAmf0Any* prop = get_property(name);
    
    if (!prop) {
        return NULL;
    }
    
    if (!prop->is_number()) {
        return NULL;
    }
    
    return prop;
}

void RtmpUnSortedHashtable::remove(std::string name)
{
    std::vector<SrsAmf0ObjectPropertyType>::iterator it;
    
    for (it = properties.begin(); it != properties.end();) {
        std::string key = it->first;
        RtmpAmf0Any* any = it->second;
        
        if (key == name) {
            free(any);
            
            it = properties.erase(it);
        } else {
            ++it;
        }
    }
}

void RtmpUnSortedHashtable::copy(RtmpUnSortedHashtable* src)
{
    std::vector<SrsAmf0ObjectPropertyType>::iterator it;
    for (it = src->properties.begin(); it != src->properties.end(); ++it) {
        SrsAmf0ObjectPropertyType& elem = *it;
        std::string key = elem.first;
        RtmpAmf0Any* any = elem.second;
        set(key, any->copy());
    }
}

RtmpAmf0ObjectEOF::RtmpAmf0ObjectEOF()
{
    marker = RTMP_AMF0_ObjectEnd;
}

RtmpAmf0ObjectEOF::~RtmpAmf0ObjectEOF()
{
}

int RtmpAmf0ObjectEOF::total_size()
{
    return RtmpAmf0Size::object_eof();
}

bool RtmpAmf0ObjectEOF::read(IOBuffer* stream)
{ 
    // value
    if (!stream->require(2)) {
        return false;
    }
    int16_t temp = stream->read_2bytes();
    if (temp != 0x00) {
        return false;
    }
    
    // marker
    if (!stream->require(1)) {
        return  false;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_ObjectEnd) {
        return false;
    }
    
    return true;
}

bool RtmpAmf0ObjectEOF::write(IOBuffer* stream)
{
    // value
    if (!stream->require(2)) {
        return false;
    }
    stream->write_2bytes(0x00);
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    stream->write_1bytes(RTMP_AMF0_ObjectEnd);
    
    return true;
}

RtmpAmf0Any* RtmpAmf0ObjectEOF::copy()
{
    return new RtmpAmf0ObjectEOF();
}

RtmpAmf0Object::RtmpAmf0Object()
{
    properties = new RtmpUnSortedHashtable();
    eof = new RtmpAmf0ObjectEOF();
    marker = RTMP_AMF0_Object;
}

RtmpAmf0Object::~RtmpAmf0Object()
{
    free(properties);
    free(eof);
}

int RtmpAmf0Object::total_size()
{
    int size = 1;
    
    for (int i = 0; i < properties->count(); i++){
        std::string name = key_at(i);
        RtmpAmf0Any* value = value_at(i);
        
        size += RtmpAmf0Size::utf8(name);
        size += RtmpAmf0Size::any(value);
    }
    
    size += RtmpAmf0Size::object_eof();
    
    return size;
}

bool RtmpAmf0Object::read(IOBuffer* stream)
{
    bool err = true;

    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Object) {
        return false;
    }
    
    // value
    while (!stream->empty()) {
        // detect whether is eof.
        if (rtmp_amf0_is_object_eof(stream)) {
            RtmpAmf0ObjectEOF pbj_eof;
            if ((err = pbj_eof.read(stream)) != true) {
                return false;
            }
            break;
        }
        
        // property-name: utf8 string
        std::string property_name;
        if ((err = rtmp_amf0_read_utf8(stream, property_name)) != true) {
            return false;
        }
        // property-value: any
        RtmpAmf0Any* property_value = NULL;
        if ((err = rtmp_amf0_read_any(stream, &property_value)) != true) {
            free(property_value);
            return false;
        }
        
        // add property
        this->set(property_name, property_value);
    }
    
    return true;
}

bool RtmpAmf0Object::write(IOBuffer* stream)
{
    bool err = true;

    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    stream->write_1bytes(RTMP_AMF0_Object);
    
    // value
    for (int i = 0; i < properties->count(); i++) {
        std::string name = this->key_at(i);
        RtmpAmf0Any* any = this->value_at(i);
        
        if ((err = rtmp_amf0_write_utf8(stream, name)) != true) {
            return false;
        }
        
        if ((err = rtmp_amf0_write_any(stream, any)) != true) {
            return false;
        }
    }
    
    if ((err = eof->write(stream)) != true) {
        return false;
    }
    
    return err;
}

RtmpAmf0Any* RtmpAmf0Object::copy()
{
    RtmpAmf0Object* copy = new RtmpAmf0Object();
    copy->properties->copy(properties);
    return copy;
}

void RtmpAmf0Object::clear()
{
    properties->clear();
}

int RtmpAmf0Object::count()
{
    return properties->count();
}

std::string RtmpAmf0Object::key_at(int index)
{
    return properties->key_at(index);
}

const char* RtmpAmf0Object::key_raw_at(int index)
{
    return properties->key_raw_at(index);
}

RtmpAmf0Any* RtmpAmf0Object::value_at(int index)
{
    return properties->value_at(index);
}

void RtmpAmf0Object::set(std::string key, RtmpAmf0Any* value)
{
    properties->set(key, value);
}

RtmpAmf0Any* RtmpAmf0Object::get_property(std::string name)
{
    return properties->get_property(name);
}

RtmpAmf0Any* RtmpAmf0Object::ensure_property_string(std::string name)
{
    return properties->ensure_property_string(name);
}

RtmpAmf0Any* RtmpAmf0Object::ensure_property_number(std::string name)
{
    return properties->ensure_property_number(name);
}

void RtmpAmf0Object::remove(std::string name)
{
    properties->remove(name);
}

RtmpAmf0EcmaArray::RtmpAmf0EcmaArray()
{
    _count = 0;
    properties = new RtmpUnSortedHashtable();
    eof = new RtmpAmf0ObjectEOF();
    marker = RTMP_AMF0_EcmaArray;
}

RtmpAmf0EcmaArray::~RtmpAmf0EcmaArray()
{
    free(properties);
    free(eof);
}

int RtmpAmf0EcmaArray::total_size()
{
    int size = 1 + 4;
    
    for (int i = 0; i < properties->count(); i++){
        std::string name = key_at(i);
        RtmpAmf0Any* value = value_at(i);
        
        size += RtmpAmf0Size::utf8(name);
        size += RtmpAmf0Size::any(value);
    }
    
    size += RtmpAmf0Size::object_eof();
    
    return size;
}

bool RtmpAmf0EcmaArray::read(IOBuffer* stream)
{
    bool err = true;
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_EcmaArray) {
        return false;
    }
    
    // count
    if (!stream->require(4)) {
        return false;
    }
    
    int32_t count = stream->read_4bytes();
    
    // value
    this->_count = count;
    
    while (!stream->empty()) {
        // detect whether is eof.
        if (rtmp_amf0_is_object_eof(stream)) {
            RtmpAmf0ObjectEOF pbj_eof;
            if ((err = pbj_eof.read(stream)) != true) {
                return false;
            }
            break;
        }
        
        // property-name: utf8 string
        std::string property_name;
        if ((err =rtmp_amf0_read_utf8(stream, property_name)) != true) {
            return false;
        }
        // property-value: any
        RtmpAmf0Any* property_value = NULL;
        if ((err = rtmp_amf0_read_any(stream, &property_value)) != true) {
            return false;
        }
        
        // add property
        this->set(property_name, property_value);
    }
    
    return err;
}

bool RtmpAmf0EcmaArray::write(IOBuffer* stream)
{
    bool err = true;
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    stream->write_1bytes(RTMP_AMF0_EcmaArray);
    
    // count
    if (!stream->require(4)) {
        return false;
    }
    
    stream->write_4bytes(this->_count);
    
    // value
    for (int i = 0; i < properties->count(); i++) {
        std::string name = this->key_at(i);
        RtmpAmf0Any* any = this->value_at(i);
        
        if ((err = rtmp_amf0_write_utf8(stream, name)) != true) {
            return false;
        }
        
        if ((err = rtmp_amf0_write_any(stream, any)) != true) {
            return false;
        }
    }
    
    if ((err = eof->write(stream)) != true) {
        return false;
    }
    
    return err;
}

RtmpAmf0Any* RtmpAmf0EcmaArray::copy()
{
    RtmpAmf0EcmaArray* copy = new RtmpAmf0EcmaArray();
    copy->properties->copy(properties);
    copy->_count = _count;
    return copy;
}

void RtmpAmf0EcmaArray::clear()
{
    properties->clear();
}

int RtmpAmf0EcmaArray::count()
{
    return properties->count();
}

std::string RtmpAmf0EcmaArray::key_at(int index)
{
    return properties->key_at(index);
}

const char* RtmpAmf0EcmaArray::key_raw_at(int index)
{
    return properties->key_raw_at(index);
}

RtmpAmf0Any* RtmpAmf0EcmaArray::value_at(int index)
{
    return properties->value_at(index);
}

void RtmpAmf0EcmaArray::set(std::string key, RtmpAmf0Any* value)
{
    properties->set(key, value);
}

RtmpAmf0Any* RtmpAmf0EcmaArray::get_property(std::string name)
{
    return properties->get_property(name);
}

RtmpAmf0Any* RtmpAmf0EcmaArray::ensure_property_string(std::string name)
{
    return properties->ensure_property_string(name);
}

RtmpAmf0Any* RtmpAmf0EcmaArray::ensure_property_number(std::string name)
{
    return properties->ensure_property_number(name);
}

RtmpAmf0StrictArray::RtmpAmf0StrictArray()
{
    marker = RTMP_AMF0_StrictArray;
    _count = 0;
}

RtmpAmf0StrictArray::~RtmpAmf0StrictArray()
{
    clear();
}

int RtmpAmf0StrictArray::total_size()
{
    int size = 1 + 4;
    
    for (int i = 0; i < (int)properties.size(); i++){
        RtmpAmf0Any* any = properties[i];
        size += any->total_size();
    }
    
    return size;
}

bool RtmpAmf0StrictArray::read(IOBuffer* stream)
{
    bool err = true;
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_StrictArray) {
        return false;
    }
    
    // count
    if (!stream->require(4)) {
        return false;
    }
    
    int32_t count = stream->read_4bytes();
    
    // value
    this->_count = count;
    
    for (int i = 0; i < count && !stream->empty(); i++) {
        // property-value: any
        RtmpAmf0Any* elem = NULL;
        if ((err = rtmp_amf0_read_any(stream, &elem)) != true) {
            return false;
        }
        
        // add property
        properties.push_back(elem);
    }
    
    return err;
}

bool RtmpAmf0StrictArray::write(IOBuffer* stream)
{
    bool err = true;
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    stream->write_1bytes(RTMP_AMF0_StrictArray);
    
    // count
    if (!stream->require(4)) {
        return false;
    }
    
    stream->write_4bytes(this->_count);
    
    // value
    for (int i = 0; i < (int)properties.size(); i++) {
        RtmpAmf0Any* any = properties[i];
        
        if ((err = rtmp_amf0_write_any(stream, any)) != true) {
            return false;
        }
    }
    
    return err;
}

RtmpAmf0Any* RtmpAmf0StrictArray::copy()
{
    RtmpAmf0StrictArray* copy = new RtmpAmf0StrictArray();
    
    std::vector<RtmpAmf0Any*>::iterator it;
    for (it = properties.begin(); it != properties.end(); ++it) {
        RtmpAmf0Any* any = *it;
        copy->append(any->copy());
    }
    
    copy->_count = _count;
    return copy;
}

void RtmpAmf0StrictArray::clear()
{
    std::vector<RtmpAmf0Any*>::iterator it;
    for (it = properties.begin(); it != properties.end(); ++it) {
        RtmpAmf0Any* any = *it;
        free(any);
    }
    properties.clear();
}

int RtmpAmf0StrictArray::count()
{
    return (int)properties.size();
}

RtmpAmf0Any* RtmpAmf0StrictArray::at(int index)
{
    //srs_assert(index < (int)properties.size());
    return properties.at(index);
}

void RtmpAmf0StrictArray::append(RtmpAmf0Any* any)
{
    properties.push_back(any);
    _count = (int32_t)properties.size();
}

int RtmpAmf0Size::utf8(std::string value)
{
    return (int)(2 + value.length());
}

int RtmpAmf0Size::str(std::string value)
{
    return 1 + RtmpAmf0Size::utf8(value);
}

int RtmpAmf0Size::number()
{
    return 1 + 8;
}

int RtmpAmf0Size::date()
{
    return 1 + 8 + 2;
}

int RtmpAmf0Size::null()
{
    return 1;
}

int RtmpAmf0Size::undefined()
{
    return 1;
}

int RtmpAmf0Size::boolean()
{
    return 1 + 1;
}

int RtmpAmf0Size::object(RtmpAmf0Object* obj)
{
    if (!obj) {
        return 0;
    }
    
    return obj->total_size();
}

int RtmpAmf0Size::object_eof()
{
    return 2 + 1;
}

int RtmpAmf0Size::ecma_array(RtmpAmf0EcmaArray* arr)
{
    if (!arr) {
        return 0;
    }
    
    return arr->total_size();
}

int RtmpAmf0Size::strict_array(RtmpAmf0StrictArray* arr)
{
    if (!arr) {
        return 0;
    }
    
    return arr->total_size();
}

int RtmpAmf0Size::any(RtmpAmf0Any* o)
{
    if (!o) {
        return 0;
    }
    
    return o->total_size();
}

RtmpAmf0String::RtmpAmf0String(const char* _value)
{
    marker = RTMP_AMF0_String;
    if (_value) {
        value = _value;
    }
}

RtmpAmf0String::~RtmpAmf0String()
{
}

int RtmpAmf0String::total_size()
{
    return RtmpAmf0Size::str(value);
}

bool RtmpAmf0String::read(IOBuffer* stream)
{
    return rtmp_amf0_read_string(stream, value);
}

bool RtmpAmf0String::write(IOBuffer* stream)
{
    return rtmp_amf0_write_string(stream, value);
}

RtmpAmf0Any* RtmpAmf0String::copy()
{
    RtmpAmf0String* copy = new RtmpAmf0String(value.c_str());
    return copy;
}

RtmpAmf0Boolean::RtmpAmf0Boolean(bool _value)
{
    marker = RTMP_AMF0_Boolean;
    value = _value;
}

RtmpAmf0Boolean::~RtmpAmf0Boolean()
{
}

int RtmpAmf0Boolean::total_size()
{
    return RtmpAmf0Size::boolean();
}

bool RtmpAmf0Boolean::read(IOBuffer* stream)
{
    return rtmp_amf0_read_boolean(stream, value);
}

bool RtmpAmf0Boolean::write(IOBuffer* stream)
{
    return rtmp_amf0_write_boolean(stream, value);
}

RtmpAmf0Any* RtmpAmf0Boolean::copy()
{
    RtmpAmf0Boolean* copy = new RtmpAmf0Boolean(value);
    return copy;
}

RtmpAmf0Number::RtmpAmf0Number(double _value)
{
    marker = RTMP_AMF0_Number;
    value = _value;
}

RtmpAmf0Number::~RtmpAmf0Number()
{
}

int RtmpAmf0Number::total_size()
{
    return RtmpAmf0Size::number();
}

bool RtmpAmf0Number::read(IOBuffer* stream)
{
    return rtmp_amf0_read_number(stream, value);
}

bool RtmpAmf0Number::write(IOBuffer* stream)
{
    return rtmp_amf0_write_number(stream, value);
}

RtmpAmf0Any* RtmpAmf0Number::copy()
{
    RtmpAmf0Number* copy = new RtmpAmf0Number(value);
    return copy;
}

RtmpAmf0Date::RtmpAmf0Date(int64_t value)
{
    marker = RTMP_AMF0_Date;
    _date_value = value;
    _time_zone = 0;
}

RtmpAmf0Date::~RtmpAmf0Date()
{
}

int RtmpAmf0Date::total_size()
{
    return RtmpAmf0Size::date();
}

bool RtmpAmf0Date::read(IOBuffer* stream)
{
    bool err = true;
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Date) {
        return false;
    }
    
    // date value
    // An ActionScript Date is serialized as the number of milliseconds
    // elapsed since the epoch of midnight on 1st Jan 1970 in the UTC
    // time zone.
    if (!stream->require(8)) {
        return false;
    }
    
    _date_value = stream->read_8bytes();
    
    // time zone
    // While the design of this type reserves room for time zone offset
    // information, it should not be filled in, nor used, as it is unconventional
    // to change time zones when serializing dates on a network. It is suggested
    // that the time zone be queried independently as needed.
    if (!stream->require(2)) {
        return false;
    }
    
    _time_zone = stream->read_2bytes();
    
    return err;
}

bool RtmpAmf0Date::write(IOBuffer* stream)
{
    bool err = true;
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    stream->write_1bytes(RTMP_AMF0_Date);
    
    // date value
    if (!stream->require(8)) {
        return false;
    }
    
    stream->write_8bytes(_date_value);
    
    // time zone
    if (!stream->require(2)) {
        return false;
    }
    
    stream->write_2bytes(_time_zone);
    
    return true;
}

RtmpAmf0Any* RtmpAmf0Date::copy()
{
    RtmpAmf0Date* copy = new RtmpAmf0Date(0);
    
    copy->_date_value = _date_value;
    copy->_time_zone = _time_zone;
    
    return copy;
}

int64_t RtmpAmf0Date::date()
{
    return _date_value;
}

int16_t RtmpAmf0Date::time_zone()
{
    return _time_zone;
}

RtmpAmf0Null::RtmpAmf0Null()
{
    marker = RTMP_AMF0_Null;
}

RtmpAmf0Null::~RtmpAmf0Null()
{
}

int RtmpAmf0Null::total_size()
{
    return RtmpAmf0Size::null();
}

bool RtmpAmf0Null::read(IOBuffer* stream)
{
    return rtmp_amf0_read_null(stream);
}

bool RtmpAmf0Null::write(IOBuffer* stream)
{
    return rtmp_amf0_write_null(stream);
}

RtmpAmf0Any* RtmpAmf0Null::copy()
{
    RtmpAmf0Null* copy = new RtmpAmf0Null();
    return copy;
}

RtmpAmf0Undefined::RtmpAmf0Undefined()
{
    marker = RTMP_AMF0_Undefined;
}

RtmpAmf0Undefined::~RtmpAmf0Undefined()
{
}

int RtmpAmf0Undefined::total_size()
{
    return RtmpAmf0Size::undefined();
}

bool RtmpAmf0Undefined::read(IOBuffer* stream)
{
    return rtmp_amf0_read_undefined(stream);
}

bool RtmpAmf0Undefined::write(IOBuffer* stream)
{
    return rtmp_amf0_write_undefined(stream);
}

RtmpAmf0Any* RtmpAmf0Undefined::copy()
{
    RtmpAmf0Undefined* copy = new RtmpAmf0Undefined();
    return copy;
}

bool rtmp_amf0_read_any(IOBuffer* stream, RtmpAmf0Any** ppvalue)
{
    bool err = true;
    
    if ((err = RtmpAmf0Any::discovery(stream, ppvalue)) != true) {
        return false;
    }
    
    //srs_assert(*ppvalue);
    
    if ((err = (*ppvalue)->read(stream)) != true) {
        free(*ppvalue);
        return false;
    }
    
    return err;
}

bool rtmp_amf0_read_string(IOBuffer* stream, std::string& value)
{
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_String) {
        return false;
    }
    
    return rtmp_amf0_read_utf8(stream, value);
}

bool rtmp_amf0_write_string(IOBuffer* stream, std::string value)
{   
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    stream->write_1bytes(RTMP_AMF0_String);
    
    return rtmp_amf0_write_utf8(stream, value);
}

bool rtmp_amf0_read_boolean(IOBuffer* stream, bool& value)
{
    bool err = true;
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Boolean) {
        return false;
    }
    
    // value
    if (!stream->require(1)) {
        return false;
    }
    
    value = (stream->read_1bytes() != 0);
    
    return err;
}

bool rtmp_amf0_write_boolean(IOBuffer* stream, bool value)
{
    bool err = true;
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    stream->write_1bytes(RTMP_AMF0_Boolean);
    
    // value
    if (!stream->require(1)) {
        return false;
    }
    
    if (value) {
        stream->write_1bytes(0x01);
    } else {
        stream->write_1bytes(0x00);
    }
    
    return err;
}

bool rtmp_amf0_read_number(IOBuffer* stream, double& value)
{
    bool err = true;
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Number) {
        return false;
    }
    
    // value
    if (!stream->require(8)) {
        return false;
    }
    
    int64_t temp = stream->read_8bytes();
    memcpy(&value, &temp, 8);
    
    return err;
}

bool rtmp_amf0_write_number(IOBuffer* stream, double value)
{
    bool err = true;
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    stream->write_1bytes(RTMP_AMF0_Number);
    
    // value
    if (!stream->require(8)) {
        return false;
    }
    
    int64_t temp = 0x00;
    memcpy(&temp, &value, 8);
    stream->write_8bytes(temp);
    
    return err;
}

bool rtmp_amf0_read_null(IOBuffer* stream)
{
    bool err = true;
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Null) {
        return false;
    }
    
    return err;
}

bool rtmp_amf0_write_null(IOBuffer* stream)
{
    bool err = true;
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    stream->write_1bytes(RTMP_AMF0_Null);
    
    return err;
}

bool rtmp_amf0_read_undefined(IOBuffer* stream)
{
    bool err = true;
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    char marker = stream->read_1bytes();
    if (marker != RTMP_AMF0_Undefined) {
        return false;
    }
    
    return err;
}

bool rtmp_amf0_write_undefined(IOBuffer* stream)
{
    bool err = true;
    
    // marker
    if (!stream->require(1)) {
        return false;
    }
    
    stream->write_1bytes(RTMP_AMF0_Undefined);
    
    return err;
}


bool rtmp_amf0_read_utf8(IOBuffer* stream, std::string& value)
{
    bool err = true;
    
    // len
    if (!stream->require(2)) {
        return false;
    }
    int16_t len = stream->read_2bytes();
    
    // empty string
    if (len <= 0) {
        return err;
    }
    
    // data
    if (!stream->require(len)) {
        return false;
    }
    std::string str = stream->read_string(len);
    
    // support utf8-1 only
    // 1.3.1 Strings and UTF-8
    // UTF8-1 = %x00-7F
    // TODO: support other utf-8 strings
    /*for (int i = 0; i < len; i++) {
        char ch = *(str.data() + i);
        if ((ch & 0x80) != 0) {
        ret = ERROR_RTMP_AMF0_DECODE;
        srs_error("ignored. only support utf8-1, 0x00-0x7F, actual is %#x. ret=%d", (int)ch, ret);
        ret = ERROR_SUCCESS;
        }
        }*/
    
    value = str;
    
    return err;
}

bool rtmp_amf0_write_utf8(IOBuffer* stream, std::string value)
{
    bool err = true;
    
    // len
    if (!stream->require(2)) {
        return false;
    }
    stream->write_2bytes((int16_t)value.length());
    
    // empty string
    if (value.length() <= 0) {
        return err;
    }
    
    // data
    if (!stream->require((int)value.length())) {
        return false;
    }
    stream->write_string(value);
    
    return err;
}

bool rtmp_amf0_is_object_eof(IOBuffer* stream)
{
    // detect the object-eof specially
    if (stream->require(3)) {
        int32_t flag = stream->read_3bytes();
        stream->skip(-3);
        
        return 0x09 == flag;
    }
    
    return false;
}

bool rtmp_amf0_write_object_eof(IOBuffer* stream, RtmpAmf0ObjectEOF* value)
{
    return value->write(stream);
}

bool rtmp_amf0_write_any(IOBuffer* stream, RtmpAmf0Any* value)
{
    return value->write(stream);
}


