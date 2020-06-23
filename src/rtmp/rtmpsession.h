#pragma once

#include "session.h"
#include <string>

class RtmpSession : public Session {
public:
    RtmpSession(const std::string& stream);
    ~RtmpSession();

public:
    

};

