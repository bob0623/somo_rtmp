#include "rtmpserver.h"
#include "rtmpconnection.h"
#include "common/logger.h"

#define __CLASS__   "RtmpServer"

RtmpServer::RtmpServer(Protocol* protocol) 
: Server(protocol)
{

}

RtmpServer::~RtmpServer() {
    clear();
}

