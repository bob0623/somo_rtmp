
#include "isnet.h"
#include "app.h"
#include "rtmpprotocol.h"

int main()
{
    SNStartup();

    App::Ins()->add_server( 1935, PROTOCOL_RTMP );
    App::Ins()->add_client("rtmp://rtmp1.video.somo.tech/video/2000U10000050", false);

    SNLoop();

    return 0;
}
