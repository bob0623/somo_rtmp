#include "util.h"

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