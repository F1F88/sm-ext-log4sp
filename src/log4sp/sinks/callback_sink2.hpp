#pragma once

#include <tuple>

#include "spdlog/sinks/base_sink.h"
#include "spdlog/sinks/null_sink.h"

#include "extension.h"

#include "log4sp/common.h"
#include "log4sp/adapter/sink_hanlder.h"


namespace log4sp {
namespace sinks {

/**
 * spdlog 1.x 的 callback_sink 仅支持单回调 (在 log -> sink_it 时)
 * 且回调函数初始化完毕后无法修改，因此重新实现一个增强版
 * 初始化后仍支持修改似乎并不是一个特别好的特性，但目前没有遇到阻碍，暂时保留
 */
class callback_sink2 final : public spdlog::sinks::base_sink<spdlog::details::null_mutex> {
public:
    using log_msg  = spdlog::details::log_msg;
    using pl_func  = SourcePawn::IPluginFunction;
    using handle_t = SourceMod::Handle_t;

    callback_sink2(pl_func *log_fn,
                   pl_func *log_post_fn,
                   pl_func *flush_fn,
                   pl_func *destory_fn,
                   handle_t plugin_handle,
                   cell_t data) noexcept
        : destory_function_(destory_fn), plugin_handle_(plugin_handle), data_(data) {
        if (log_fn) {
            // CallbackSink2Result (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime, any data, char error[256]);
            FWDS_CREATE_EX(nullptr, ET_Single, 9, nullptr,
                           SourceMod::ParamType::Param_String,  // name
                           SourceMod::ParamType::Param_Cell,    // lvl
                           SourceMod::ParamType::Param_String,  // msg
                           SourceMod::ParamType::Param_String,  // file
                           SourceMod::ParamType::Param_Cell,    // line
                           SourceMod::ParamType::Param_String,  // func
                           SourceMod::ParamType::Param_Cell,    // logTime
                           SourceMod::ParamType::Param_Cell,    // data
                           SourceMod::ParamType::Param_String); // error
            FWD_ADD_FUNCTION(log_fn);
            log_callback_ = forward;
        }

        if (log_post_fn) {
            // CallbackSink2Result (const char[] msg, any data, char error[256]);
            FWDS_CREATE_EX(nullptr, ET_Single, 3, nullptr,
                           SourceMod::ParamType::Param_String,  // msg
                           SourceMod::ParamType::Param_Cell,    // data
                           SourceMod::ParamType::Param_String); // error
            FWD_ADD_FUNCTION(log_post_fn);
            log_post_callback_ = forward;
        }

        if (flush_fn) {
            // CallbackSink2Result (any data, char error[256]);
            FWDS_CREATE_EX(nullptr, ET_Single, 2, nullptr,
                           SourceMod::ParamType::Param_Cell,    // data
                           SourceMod::ParamType::Param_String); // error
            FWD_ADD_FUNCTION(flush_fn);
            flush_callback_ = forward;
        }
    }

    ~callback_sink2() noexcept override {
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

        if (destory_function_) {
            on_destory_();
        }
    }

    cell_t get_data() const noexcept        { return data_; }
    void   set_data(cell_t data) noexcept   { data_ = data; }

private:
    void sink_it_(const log_msg &log_msg) override {
        using spdlog::throw_spdlog_ex;

        if (log_callback_) {
            auto forward = log_callback_;
            auto logTime = std::chrono::duration_cast<std::chrono::seconds>(log_msg.time.time_since_epoch());
            std::array<char, 256> error;
            cell_t result = 0;

            FWD_PUSH_STRING(log_msg.logger_name.data());        // name
            FWD_PUSH_CELL(log_msg.level);                       // lvl
            FWD_PUSH_STRING(log_msg.payload.data());            // msg
            FWD_PUSH_STRING(log_msg.source.filename);           // file
            FWD_PUSH_CELL(log_msg.source.line);                 // line
            FWD_PUSH_STRING(log_msg.source.funcname);           // func
            FWD_PUSH_CELL(static_cast<cell_t>(logTime.count()));// logTime
            FWD_PUSH_CELL(data_);                               // data
            FWD_PUSH_STRING_EX(error.data(), error.size(), SM_PARAM_STRING_COPY | SM_PARAM_STRING_UTF8, SM_PARAM_COPYBACK); // error
            FWD_EXECUTE(&result);

            if (!!result)
                throw_spdlog_ex(error.data());
        }

        if (log_post_callback_) {
            auto forward = log_post_callback_;
            std::string formatted = to_pattern(log_msg);
            std::array<char, 256> error;
            cell_t result = 0;
            FWD_PUSH_STRING(formatted.c_str());                 // msg
            FWD_PUSH_CELL(data_);                               // data
            FWD_PUSH_STRING_EX(error.data(), error.size(), SM_PARAM_STRING_COPY | SM_PARAM_STRING_UTF8, SM_PARAM_COPYBACK); // error
            FWD_EXECUTE(&result);

            if (!!result)
                throw_spdlog_ex(error.data());
        }
    }

    void flush_() override {
        using spdlog::throw_spdlog_ex;
        if (flush_callback_) {
            auto forward = flush_callback_;
            std::array<char, 256> error;
            cell_t result = 0;

            FWD_PUSH_CELL(data_);                               // data
            FWD_PUSH_STRING_EX(error.data(), error.size(), SM_PARAM_STRING_COPY | SM_PARAM_STRING_UTF8, SM_PARAM_COPYBACK); // error
            FWD_EXECUTE(&result);

            if (!!result)
                throw_spdlog_ex(error.data());
        }
    }

    // Ref: sm_AddFrameAction
    void on_destory_() const noexcept {
        assert(destory_function_);

        // void (any data);
        FWDS_CREATE_EX(nullptr, ET_Ignore, 1, nullptr, SourceMod::ParamType::Param_Cell);
        FWD_ADD_FUNCTION(destory_function_);

        using frame_data_t = std::tuple<handle_t, SourceMod::IChangeableForward*, cell_t>;
        frame_data_t *data = new frame_data_t(plugin_handle_, forward, data_);

        // 放到下一帧执行, 以避免在回调中使用 Sink Natives 造成的崩溃
        smutils->AddFrameAction(
            [](void* raw) {
                std::unique_ptr<frame_data_t> frame_data(reinterpret_cast<frame_data_t*>(raw));
                auto plugin_handle  = std::get<0>(*frame_data);
                auto forward        = std::get<1>(*frame_data);
                auto data           = std::get<2>(*frame_data);

                auto plugin = plsys->PluginFromHandle(plugin_handle, nullptr);
                if (!plugin) {
                    forwards->ReleaseForward(forward);
                    return;
                }

                // void (any data);
                FWD_PUSH_CELL(data);                            // data
                FWD_EXECUTE();
                forwards->ReleaseForward(forward);
            },
            (void*)data);
    }

    SourceMod::IChangeableForward *log_callback_{nullptr};
    SourceMod::IChangeableForward *log_post_callback_{nullptr};
    SourceMod::IChangeableForward *flush_callback_{nullptr};
    pl_func *destory_function_{nullptr};
    handle_t plugin_handle_{BAD_HANDLE};
    cell_t data_{0};
};


}       // namespace sinks
}       // namespace log4sp
