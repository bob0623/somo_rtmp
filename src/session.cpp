#include "session.h"
#include "connection.h"

#include "common/logger.h"

#define     __CLASS__   "Session"

Session::Session(uint32_t sid) 
: m_nSid(sid)
{

}

Session::~Session() {

}

void    Session::set_publisher(Publisher* publisher) {
    

}

void    Session::add_consumer(Consumer* consumer) {

}

void    Session::remove_consumer(uint32_t linkid) {

}

