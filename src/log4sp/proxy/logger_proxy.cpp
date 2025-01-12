#include "spdlog/spdlog.h"

#include "log4sp/adapter/logger_handler.h"

#include "log4sp/proxy/logger_proxy.h"


namespace log4sp {

logger_proxy::~logger_proxy() {
    if (error_forward_ != nullptr) {
        forwards->ReleaseForward(error_forward_);
        error_forward_ = nullptr;
    }
}

void logger_proxy::add_sink(spdlog::sink_ptr sink) {
    sinks_.push_back(sink);
}

void logger_proxy::remove_sink(spdlog::sink_ptr sink) {
    sinks_.erase(std::remove(sinks_.begin(), sinks_.end(), sink), sinks_.end());
}

void logger_proxy::set_error_forward(IChangeableForward *forward) {
    if (error_forward_ != nullptr) {
        forwards->ReleaseForward(error_forward_);
    }
    error_forward_ = forward;
}

void logger_proxy::error_handler(spdlog::source_loc loc, const std::string &msg) {
    if (error_forward_ != nullptr) {
        error_forward_->PushString(msg.c_str());
        error_forward_->Execute();
    } else {
        static size_t err_counter = 0;
        spdlog::log(loc, spdlog::level::err, "[#{}] [{}] {}", ++err_counter, name(), msg);
    }
}

void logger_proxy::sink_it_(const spdlog::details::log_msg &msg) {
    for (auto &sink : sinks_) {
        if (sink->should_log(msg.level)) {
            try {
                sink->log(msg);
            } catch (const std::exception &ex) {
                error_handler(msg.source, ex.what());
            } catch (...) {
                smutils->LogError(myself, "Extension Error! logger log: Unknown exception caught.");
                throw;
            }
        }
    }

    if (should_flush_(msg)) {
        flush_();
    }
}

void logger_proxy::flush_() {
    for (auto &sink : sinks_) {
        try {
            sink->flush();
        } catch (const std::exception &ex) {
            error_handler(spdlog::source_loc{}, ex.what());
        } catch (...) {
            smutils->LogError(myself, "Extension Error! logger flush: Unknown exception caught.");
            throw;
        }
    }
}


}       // namespace log4sp
