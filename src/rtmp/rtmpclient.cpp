#include "rtmpclient.h"
#include "rtmpconnection.h"
#include "rtmpshakehands.h"
#include "rtmpstream.h"
#include "rtmpformat.h"

#include "common/util.h"
#include "common/logger.h"

#define __CLASS__   "RtmpClient"

RtmpClient::RtmpClient(Protocol* protocol, const std::string& url)
: Client(protocol, url)
{ 
    FUNLOG(Info, "rtmp client new, url=%s", url.c_str());
}

RtmpClient::~RtmpClient() {

}

void RtmpClient::on_connected(ISNLink* pLink) {
    RtmpConnection* conn = (RtmpConnection*)connection();
    if( conn == NULL ) {
        FUNLOG(Error, "rtmp client on connected, conn==NULL for linkid=%d", pLink->linkid());
        return;
    }

    conn->start_shake_hands();
}
