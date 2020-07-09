#pragma once

#include "connection.h"
#include "isnet.h"
#include <map>

/**
 * A connection map to a RtmpStream. A Stream is either a RtmpPubsher or RtmpConsumer.
 * 
 * RtmpConnection->Connection->ISNLink
 * RtmpConnection->RtmpStream->RtmpPublisher/RtmpConsumer
 * RtmpSession-> 1*RtmpPublisher + N*RtmpConsumer
 */

class RtmpStream;
class RtmpBuffer;
class RtmpMsgBuffer;
class RtmpShakeHands_Server;
class RtmpShakeHands_Client;
class RtmpChunkStream;
class RtmpMessage;
class RtmpChunkMsg;
class RtmpConnection : public Connection {
public:
    RtmpConnection(const std::string& ip, short port, const std::string& path, bool player, ISNLinkHandler* handler);
    RtmpConnection(ISNLink* link);
    ~RtmpConnection();

public:
    virtual int on_data(const char* data, int len);
    virtual void clear();
    virtual Session* session();
    virtual bool is_alive();

public:
    RtmpStream* stream() { return m_pStream; }
    RtmpBuffer* buffer() { return m_pBuffer; }
    bool is_client() { return m_bClient; }
    bool is_player() { return m_bPlayer; }
    RtmpChunkStream*get_chunk_stream(int cid);
    std::string path() { return m_strRtmpPath; }

public:
    void start_shake_hands();
    int shake_hands(const char* data, int len);
    void handle_msg(RtmpMsgBuffer* msg_buf);

private:
    RtmpShakeHands_Server*  m_pSHServer;
    RtmpShakeHands_Client*  m_pSHClient;
    uint64_t    m_nConnectStamp;
    bool        m_bClient;
    bool        m_bPlayer;
    bool        m_bShakeHands;
    std::map<int, RtmpChunkStream*>    m_mapStreams;
    RtmpStream*m_pStream;
    RtmpBuffer* m_pBuffer;
    //client only, the app&stream info.
    std::string m_strRtmpPath;
    uint64_t    m_nLastDataStamp;
};




