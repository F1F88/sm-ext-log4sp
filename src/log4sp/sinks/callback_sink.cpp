#include "log4sp/sinks/callback_sink.h"


namespace log4sp {
namespace sinks {

callback_sink::callback_sink(IPluginFunction *log_function)
    : formatter_{spdlog::details::make_unique<spdlog::pattern_formatter>()} {
    set_log_callback(log_function);
}

callback_sink::callback_sink(IPluginFunction *log_function, IPluginFunction *flush_function)
    : callback_sink(log_function) {

    try {
        set_flush_callback(flush_function);
    } catch (...) {
        forwards->ReleaseForward(log_callback_);
        throw;
    }
}

callback_sink::~callback_sink() {
    forwards->ReleaseForward(log_callback_);
    if (flush_callback_ != nullptr) {
        forwards->ReleaseForward(flush_callback_);
    }
}

void callback_sink::set_log_callback(IPluginFunction *log_function) {
    assert(log_function != nullptr);

    auto cb = forwards->CreateForwardEx(nullptr, ET_Ignore, 8, nullptr,
                                        Param_String, Param_Cell, Param_String,
                                        Param_String, Param_Cell, Param_String,
                                        Param_Array, Param_Array);
    if (cb == nullptr) {
        throw std::runtime_error{"SM error! Could not create callback sink log forward."};
    }

    if (!cb->AddFunction(log_function)) {
        forwards->ReleaseForward(cb);
        throw std::runtime_error{"SM error! Could not add callback sink log function."};
    }

    if (log_callback_ != nullptr) {
        forwards->ReleaseForward(log_callback_);
    }
    log_callback_ = cb;
}

void callback_sink::set_flush_callback(IPluginFunction *flush_function) {
    assert(flush_function != nullptr);

    auto cb = forwards->CreateForwardEx(nullptr, ET_Ignore, 0, nullptr);
    if (cb == nullptr) {
        throw std::runtime_error{"SM error! Could not create callback sink flush forward."};
    }

    if (!cb->AddFunction(flush_function)) {
        forwards->ReleaseForward(cb);
        throw std::runtime_error{"SM error! Could not add callback sink flush function."};
    }

    if (flush_callback_ != nullptr) {
        forwards->ReleaseForward(flush_callback_);
    }
    flush_callback_ = cb;
}

void callback_sink::log(const spdlog::details::log_msg &msg) {
    int64_t sec = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::seconds>(msg.time.time_since_epoch()).count());
    int64_t ns  = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(msg.time.time_since_epoch()).count());

    // See SMCore::GetTime
    cell_t pSec[]{static_cast<cell_t>(sec & 0xFFFFFFFF), static_cast<cell_t>((sec >> 32) & 0xFFFFFFFF)};
    cell_t pNs[]{static_cast<cell_t>(ns & 0xFFFFFFFF), static_cast<cell_t>((ns >> 32) & 0xFFFFFFFF)};

    log_callback_->PushString(msg.logger_name.data());
    log_callback_->PushCell(msg.level);
    log_callback_->PushString(msg.payload.data());

    log_callback_->PushString(msg.source.filename);
    log_callback_->PushCell(msg.source.line);
    log_callback_->PushString(msg.source.funcname);

    log_callback_->PushArray(pSec, sizeof(pSec));
    log_callback_->PushArray(pNs, sizeof(pNs));
    log_callback_->Execute();
}

void callback_sink::flush() {
    if (flush_callback_ != nullptr) {
        flush_callback_->Execute();
    }
}


}       // namespace sinks
}       // namespace log4sp
