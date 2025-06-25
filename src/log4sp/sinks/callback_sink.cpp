#include <cassert>
#include "log4sp/common.h"
#include "log4sp/sinks/callback_sink.h"


namespace log4sp {
namespace sinks {

using spdlog::details::log_msg;

callback_sink::callback_sink(IPluginFunction *log_function,
                             IPluginFunction *log_post_function,
                             IPluginFunction *flush_function) noexcept {
    set_log_callback(log_function);
    set_log_post_callback(log_post_function);
    set_flush_callback(flush_function);
}

callback_sink::~callback_sink() noexcept {
    release_forwards_();
}

void callback_sink::release_forwards_() noexcept {
    if (log_callback_) {
        forwards->ReleaseForward(log_callback_);
        log_callback_ = nullptr;
    }

    if (log_post_callback_) {
        forwards->ReleaseForward(log_post_callback_);
        log_post_callback_ = nullptr;
    }

    if (flush_callback_) {
        forwards->ReleaseForward(flush_callback_);
        flush_callback_ = nullptr;
    }
}

void callback_sink::set_log_callback(IPluginFunction *log_function) noexcept {
    if (log_callback_) {
        forwards->ReleaseForward(log_callback_);
        log_callback_ = nullptr;
    }

    if (log_function) {
        // void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime);
        FWDS_CREATE_EX(nullptr, ET_Ignore, 7, nullptr,
                       Param_String,    // name
                       Param_Cell,      // lvl
                       Param_String,    // msg
                       Param_String,    // file
                       Param_Cell,      // line
                       Param_String,    // func
                       Param_Cell);     // logTime
        FWD_ADD_FUNCTION(log_function);
        log_callback_ = forward;
    }
}

void callback_sink::set_log_post_callback(IPluginFunction *log_post_function) noexcept {
    if (log_post_callback_) {
        forwards->ReleaseForward(log_post_callback_);
        log_post_callback_ = nullptr;
    }

    if (log_post_function) {
        // void (const char[] msg);
        FWDS_CREATE_EX(nullptr, ET_Ignore, 1, nullptr, Param_String);
        FWD_ADD_FUNCTION(log_post_function);
        log_post_callback_ = forward;
    }
}

void callback_sink::set_flush_callback(IPluginFunction *flush_function) noexcept {
    if (flush_callback_) {
        forwards->ReleaseForward(flush_callback_);
        flush_callback_ = nullptr;
    }

    if (flush_function) {
        // void ();
        FWDS_CREATE_EX(nullptr, ET_Ignore, 0, nullptr);
        FWD_ADD_FUNCTION(flush_function);
        flush_callback_ = forward;
    }
}

void callback_sink::sink_it_(const log_msg &log_msg) noexcept {
    if (log_callback_) {
        auto forward = log_callback_;
        auto logTime = std::chrono::duration_cast<std::chrono::seconds>(log_msg.time.time_since_epoch());

        FWD_PUSH_STRING(log_msg.logger_name.data());            // name
        FWD_PUSH_CELL(log_msg.level);                           // lvl
        FWD_PUSH_STRING(log_msg.payload.data());                // msg
        FWD_PUSH_STRING(log_msg.source.filename);               // file
        FWD_PUSH_CELL(log_msg.source.line);                     // line
        FWD_PUSH_STRING(log_msg.source.funcname);               // func
        FWD_PUSH_CELL(static_cast<cell_t>(logTime.count()));    // logTime
        FWD_EXECUTE();
    }

    if (log_post_callback_) {
        auto forward = log_post_callback_;
        std::string formatted = to_pattern(log_msg);

        FWD_PUSH_STRING(formatted.c_str());
        FWD_EXECUTE();
    }
}

void callback_sink::flush_() noexcept {
    if (flush_callback_) {
        auto forward = flush_callback_;
        FWD_EXECUTE();
    }
}


}       // namespace sinks
}       // namespace log4sp
