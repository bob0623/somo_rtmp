#include "session.h"
#include "connection.h"

#include "common/logger.h"

#define     __CLASS__   "Session"

Session::Session(const std::string& stream) 
: m_strStream(stream)
{

}

Session::~Session() {

}

void    Session::set_publisher(Publisher* publisher) {
    m_pPublisher = publisher;
}

void    Session::add_consumer(Consumer* consumer) {

}

void    Session::remove_consumer(uint32_t linkid) {

}

