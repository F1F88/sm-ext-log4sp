#pragma once

#include "spdlog/sinks/base_sink.h"

#include "extension.h"


namespace log4sp {
namespace sinks {

/**
 * spdlog 1.x 的 callback_sink 仅支持单回调 (在 log -> sink_it 时)
 * 且回调函数初始化完毕后无法修改，因此重新实现一个增强版
 * 初始化后仍支持修改似乎并不是一个特别好的特性，但目前没有遇到阻碍，暂时保留
 */
class callback_sink final : public spdlog::sinks::base_sink<spdlog::details::null_mutex> {
public:
    using log_msg = spdlog::details::log_msg;

    callback_sink(IPluginFunction *log_function = nullptr,
                  IPluginFunction *log_post_function = nullptr,
                  IPluginFunction *flush_function = nullptr) noexcept;
    ~callback_sink() noexcept override;

    void set_log_callback(IPluginFunction *log_function) noexcept;
    void set_log_post_callback(IPluginFunction *log_post_function) noexcept;
    void set_flush_callback(IPluginFunction *flush_function) noexcept;

private:
    SourceMod::IChangeableForward *log_callback_{nullptr};
    SourceMod::IChangeableForward *log_post_callback_{nullptr};
    SourceMod::IChangeableForward *flush_callback_{nullptr};

    void sink_it_(const log_msg &log_msg) noexcept override;
    void flush_() noexcept override;

    void release_forwards_() noexcept;
};


}       // namespace sinks
}       // namespace log4sp
