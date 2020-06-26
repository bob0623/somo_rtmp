#pragma once

#include "connection.h"
#include "isnet.h"
#include <map>

/**
 * That's the map, a connection map to a RtmpStream. A Stream is either a RtmpPubsher or RtmpConsumer.
 * 
 * RtmpConnection->Connection->ISNLink
 * RtmpConnection->RtmpStream->RtmpPublisher/RtmpConsumer
 * RtmpSession-> 1*RtmpPublisher + N*RtmpConsumer
 */

class RtmpStream;
class RtmpBuffer;
class RtmpMsgBuffer;
class RtmpShakeHands;
class RtmpChunkStream;
class RtmpMessage;
class RtmpChunkMsg;
class RtmpConnection : public Connection {
public:
    RtmpConnection(ISNLink* link);
    ~RtmpConnection();

public:
    virtual int on_data(const char* data, int len);

public:
    RtmpStream* stream() { return m_pStream; }
    RtmpBuffer* buffer() { return m_pBuffer; }

private:
    int shake_hands(const char* data, int len);
    void handle_msg(RtmpMsgBuffer* msg_buf);
    RtmpChunkStream*get_chunk_stream(int cid);

private:
    RtmpShakeHands*  m_pShakeHands;
    uint64_t    m_nConnectStamp;
    bool        m_bShakeHands;
    std::map<int, RtmpChunkStream*>    m_mapStreams;
    RtmpStream*m_pStream;
    RtmpBuffer* m_pBuffer;
};




