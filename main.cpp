
#include "isnet.h"
#include "app.h"
#include "rtmpprotocol.h"

int main()
{
    SNStartup();

    App::Ins()->add_server( 1935, PROTOCOL_RTMP );

    SNLoop();

    return 0;
}
