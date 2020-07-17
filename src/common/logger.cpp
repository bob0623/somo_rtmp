#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdio.h> 
#include <sys/time.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>
#include "spdlog/details/registry-inl.h"

bool g_log_init = false;
int g_log_level = Info; //default
struct timeval g_tv;
spdlog::logger g_logger("somomcu");

void init_sys_log()
{
#if defined(__APPLE__)
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v");
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("log/messages.log", true);
    file_sink->set_level(spdlog::level::trace);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] %v");
    g_logger = spdlog::logger("somomcu", {console_sink, file_sink});
    g_logger.set_level(spdlog::level::trace);
#else
    ::openlog(NULL, LOG_PID, LOG_LOCAL0);
#endif
}

void log(int level, const char *fmt, ...)
{
    if( !g_log_init ) {
        init_sys_log();
        g_log_init = true;
    }

#if defined(__APPLE__)
    char tmp[2048] = { 0 };
	va_list args;
	va_start(args, fmt);
	vsprintf(tmp, fmt, args);
	va_end(args);
    switch (level)
    {
    case Info:
        g_logger.info(tmp);
        break;
    case Debug:
        g_logger.debug(tmp);
        break;
    case Notice:
        g_logger.trace(tmp);
        break;
    case Warn:
        g_logger.warn(tmp);
        break;
    case Error:
        g_logger.error(tmp);
        break;
    case Emerg:
        g_logger.critical(tmp);
        break;    
    default:
        g_logger.info(tmp);
        break;
    }
#else
    if ( level > g_log_level )
        return;
    va_list    param;
    ::va_start(param, fmt);
    ::vsyslog(level, fmt, param);
    ::va_end(param);
#endif
}

void set_syslog_level(int level)
{
    g_log_level = level;
}
