#pragma once

#include "server.h"
#include <map>

class RtmpConnection;
class RtmpServer : public Server {
public:
    RtmpServer(Protocol* protocol);
    ~RtmpServer();

public:

};
