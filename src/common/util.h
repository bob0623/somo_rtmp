#pragma once

#include <stdint.h>
#include <random>
#include <sys/time.h>

class Util {
public:
    static void     generate_random(char* bytes, int size);
    static uint64_t system_time_msec();

};