#include "spdlog/spdlog.h"

#include "log4sp/adapter/logger_handler.h"

#include "log4sp/proxy/async_logger_proxy.h"


namespace log4sp {

async_logger_proxy::~async_logger_proxy() {
    std::lock_guard<std::mutex> lock(error_mutex_);
    if (error_forward_ != nullptr) {
        forwards->ReleaseForward(error_forward_);
        error_forward_ = nullptr;
    }
}

void async_logger_proxy::add_sink(spdlog::sink_ptr sink) {
    std::static_pointer_cast<spdlog::sinks::dist_sink_mt>(sinks_.front())->add_sink(sink);
}

void async_logger_proxy::remove_sink(spdlog::sink_ptr sink) {
    std::static_pointer_cast<spdlog::sinks::dist_sink_mt>(sinks_.front())->remove_sink(sink);
}

void async_logger_proxy::set_error_forward(IChangeableForward *forward) {
    std::lock_guard<std::mutex> lock(error_mutex_);
    if (error_forward_ != nullptr) {
        forwards->ReleaseForward(error_forward_);
    }
    error_forward_ = forward;
}

void async_logger_proxy::error_handler(spdlog::source_loc loc, const std::string &msg) {
    std::lock_guard<std::mutex> lock(error_mutex_);
    logger_proxy::error_handler(loc, msg);
}

void async_logger_proxy::sink_it_(const spdlog::details::log_msg &msg) {
    try {
        if (auto pool_ptr = thread_pool_.lock()) {
            pool_ptr->post_log(shared_from_this(), msg, overflow_policy_);
        } else {
            throw std::runtime_error("Extension Error! async log: thread pool doesn't exist anymore.");
        }
    } catch (const std::exception &ex) {
        error_handler(msg.source, ex.what());
    } catch (...) {
        smutils->LogError(myself, "Extension Error! async log: Unknown exception caught.");
        throw;
    }
}

void async_logger_proxy::flush_() {
    try {
        if (auto pool_ptr = thread_pool_.lock()) {
            pool_ptr->post_flush(shared_from_this(), overflow_policy_);
        } else {
            throw std::runtime_error("Extension Error! async flush: thread pool doesn't exist anymore");
        }
    } catch (const std::exception &ex) {
        error_handler(spdlog::source_loc{}, ex.what());
    } catch (...) {
        smutils->LogError(myself, "Extension Error! async flush: Unknown exception caught.");
        throw;
    }
}

void async_logger_proxy::backend_sink_it_(const spdlog::details::log_msg &msg) {
    for (auto &sink : sinks_) {
        if (sink->should_log(msg.level)) {
            try {
                sink->log(msg);
            } catch (const std::exception &ex) {
                error_handler(msg.source, ex.what());
            } catch (...) {
                smutils->LogError(myself, "Extension Error! async backend log: Unknown exception caught.");
                throw;
            }
        }
    }

    if (should_flush_(msg)) {
        backend_flush_();
    }
}

void async_logger_proxy::backend_flush_() {
    for (auto &sink : sinks_) {
        try {
            sink->flush();
        } catch (const std::exception &ex) {
            error_handler(spdlog::source_loc{}, ex.what());
        } catch (...) {
            smutils->LogError(myself, "Extension Error! async backend flush: Unknown exception caught.");
            throw;
        }
    }
}


}       // namespace log4sp
