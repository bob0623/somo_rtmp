
#include "isnet.h"
#include "app.h"
#include "rtmpprotocol.h"

int main()
{
    SNStartup();

    App::Ins()->add_server( 1935, PROTOCOL_RTMP );
    App::Ins()->add_forwarder("rtmp://rtmp1.video.somo.tech/video/2000U10000050");

    SNLoop();

    return 0;
}
