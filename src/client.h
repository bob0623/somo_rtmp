#pragma once

#include "isnet.h"
#include <string>

struct ISNLink;
class Protocol;
class Connection;
class Client : public ISNLinkHandler {
public:
    /**
     * A client must be specified as player or not.
     * For Rtmp, player send PLAY command, otherwise send FCPublish&publish command to server.
     */
    Client(Protocol* protocol, const std::string& url, bool player);

    virtual ~Client();

public:
    virtual void on_connected(ISNLink* pLink);
    virtual void on_close(ISNLink* pLink);
    virtual int  on_data(const char* data, size_t len, ISNLink* pLink);

    /**
     * handle meta data, for RTMP it's rtmp data command.
     */
    virtual void on_meta_data(const char* data, int len) = 0;
    
    /**
     * handle rtmp video tag.
     */
    virtual void on_video_rtmp(const char* data, int len) = 0;
    
    /**
     * handle rtmp video sequence header.
     */
    virtual void on_video_rtmp_sh(const char* data, int len) = 0;

public:
    Connection*     connection() { return m_pConnection; }
    std::string     url() { return m_strUrl; }
    std::string     ip() { return m_strIp; }
    short   port() { return m_nPort; }
    std::string     path() { return m_strPath; }
    bool            is_player() { return m_bPlayer; }

private:
    void    connect();

protected:
    Protocol*       m_pProtocol;
    std::string     m_strUrl;
    std::string     m_strIp;
    short           m_nPort;
    std::string     m_strPath;
    bool            m_bPlayer;
    Connection*     m_pConnection;
};

