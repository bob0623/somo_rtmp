#pragma once

#include <stdint.h>
#include <random>
#include <sys/time.h>
#include <string>

class Util {
public:
    static void     generate_random(char* bytes, int size);
    static uint64_t system_time_msec();
    static std::string get_url_protocol(const std::string& url);
    static std::string get_url_domain(const std::string& url);
    static std::string get_url_path(const std::string& url);
    static std::string get_url_rtmp_app(const std::string& url);
    static std::string get_url_rtmp_stream(const std::string& url);
    static std::string get_host_ip(const char* hostname);
};