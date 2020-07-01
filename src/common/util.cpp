#include "util.h"

#include "common/logger.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#define __CLASS__   "Util"

void Util::generate_random(char* bytes, int size)
{
    static bool _random_initialized = false;
    if (!_random_initialized) {
        srand(0);
        _random_initialized = true;
    }
    
    for (int i = 0; i < size; i++) {
        // the common value in [0x0f, 0xf0]
        bytes[i] = 0x0f + (rand() % (256 - 0x0f - 0x0f));
    }
}

uint64_t Util::system_time_msec()
{
    struct timeval _tv;
    ::gettimeofday(&_tv, NULL);
    return (_tv.tv_sec*1000) + (_tv.tv_usec/1000);
}

std::string Util::get_url_protocol(const std::string& url) {
    int pos = url.find_first_of("://");
    if( pos == -1 )
        return "";
    
    return url.substr(0, pos);
}

std::string Util::get_url_domain(const std::string& url) {
    std::string url_left = url;
    int pos = url_left.find_first_of("://");
    if( pos == -1 ) {
        FUNLOG(Error, "util get url domain failed, find prfix failed! url=%s", url.c_str());
        return "";
    }
    url_left = url_left.substr(pos+3);
    //FUNLOG(Info, "util get url domain, url=%s, no_prefix=%s", url.c_str(), url_left.c_str());

    pos = url_left.find_first_of("/");
    if( pos == -1 ) {
        FUNLOG(Error, "util get url domain failed, find / failed! url=%s", url.c_str());
        return "";
    }
    
    return url_left.substr(0, pos);
}

std::string Util::get_url_path(const std::string& url) {
    std::string url_left = url;
    int pos = url_left.find_first_of("://");
    if( pos == -1 )
        return "";
    
    url_left = url_left.substr(pos+3);
    pos = url_left.find_first_of("/");
    if( pos == -1 )
        return "";
    
    url_left = url_left.substr(pos+1);

    return url_left;
}

std::string Util::get_host_ip(const char* hostname)
{
    struct hostent *host = gethostbyname(hostname);
    if (host == NULL)
    {
        return 0;
    }

    uint32_t addr_val = *(u_long *)host->h_addr;
    struct in_addr addr;
    memcpy(&addr, &addr_val, 4);
    return inet_ntoa(addr);
}
