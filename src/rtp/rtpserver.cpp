#include "rtpserver.h"

#define __CLASS__   "RtpServer"

RtpServer::RtpServer(Protocol* protocol) 
: Server(protocol)
{

}

RtpServer::~RtpServer() {
    clear();
}

