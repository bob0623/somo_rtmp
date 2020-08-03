
#include "isnet.h"
#include "app.h"
#include "rtmpprotocol.h"
#include "common/logger.h"

int main()
{
    log(LOG_INFO, "main entry.");
    SNStartup();
    log(LOG_INFO, "SNSStartup over.");
    App::Ins()->add_server( 1935, PROTOCOL_RTMP );
    App::Ins()->add_forwarder("rtmp://192.168.1.18/video/100U101");
    
    //App::Ins()->add_server( 8000, PROTOCOL_RTP );

    SNLoop();
    log(0, "SNLoop exit");
    return 0;
}
