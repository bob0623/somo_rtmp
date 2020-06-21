
#include "isnet.h"
#include "app.h"
#include "rtmpprotocol.h"

int main()
{
    SNStartup();

    App::Ins()->add_server( 1935, new RtmpProtocol() );

    SNLoop();

    return 0;
}
