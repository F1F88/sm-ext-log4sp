#ifndef _LOG4SP_PROXY_SYNC_LOGGER_PROXY_INL_H_
#define _LOG4SP_PROXY_SYNC_LOGGER_PROXY_INL_H_

#include "log4sp/adapter/logger_handler.h"

#include "log4sp/proxy/sync_logger_proxy.h"


namespace log4sp {

inline sync_logger_proxy::~sync_logger_proxy() {
    release_error_forward();
}

inline void sync_logger_proxy::add_sink(spdlog::sink_ptr sink) {
    sinks_.push_back(sink);
}

inline void sync_logger_proxy::remove_sink(spdlog::sink_ptr sink) {
    sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), sink), sinks_.end());
}

inline void sync_logger_proxy::sink_it_(const spdlog::details::log_msg &msg) {
    for (auto &sink : sinks_) {
        if (sink->should_log(msg.level)) {
            try {
                sink->log(msg);
            } catch (const std::exception &ex) {
                error_handler(msg.source, name(), ex.what());
            } catch (...) {
                SPDLOG_CRITICAL("Internal Error! Extension Log4sp encountered an unknown exception. Please contact the developer.");
                throw;
            }
        }
    }

    if (should_flush_(msg)) {
        flush_();
    }
}

inline void sync_logger_proxy::flush_() {
    for (auto &sink : sinks_) {
        try {
            sink->flush();
        } catch (const std::exception &ex) {
            error_handler(spdlog::source_loc{}, name(), ex.what());
        } catch (...) {
            SPDLOG_CRITICAL("Internal Error! Extension Log4sp encountered an unknown exception. Please contact the developer.");
            throw;
        }
    }
}


}       // namespace log4sp
#endif  // _LOG4SP_PROXY_SYNC_LOGGER_PROXY_INL_H_
