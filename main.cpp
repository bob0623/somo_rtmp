
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
    App::Ins()->add_forwarder("rtmp://rtmp1.video.somo.tech/video/2000U10000050");
    log(LOG_INFO, "SNLoop enter.");
    SNLoop();
    log(0, "SNLoop exit");
    return 0;
}
