#pragma once

#include <isnet.h>
#include <string>

/**
 * Connection is the base object wrap ISNLink.
 * 
 * The most important function is on_data, RtmpConnection and RtpConnection should implement this to handle protocol data.
 * 
 */

class Session;
class Connection {
public:
    Connection(const std::string& ip, short port, ISNLinkHandler* handler);
    Connection(ISNLink* link);
    ~Connection();

public:
    /**
     * to handle protocol data
     */
    virtual int    on_data(const char* data, int len) = 0;

    /**
     * when connection error happends, clear the scenario and re-connect!
     */
    virtual void   clear() = 0;

    /**
     * return RtmpSession for RTMP protocol
     */
    virtual Session*    session() = 0;

    /**
     * check if the connection is still alive.
     */
    virtual bool    is_alive() = 0;

public:
    void    send(const char* data, int len);
    void    close();
    int     linkid() { return m_pLink->linkid(); }

private:
    ISNLink*    m_pLink;

};
