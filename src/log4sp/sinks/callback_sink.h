#pragma once

#include "extension.h"

#include "log4sp/sinks/base_sink.h"


namespace log4sp {
namespace sinks {

class callback_sink final : public base_sink {
public:
    callback_sink(IPluginFunction *log_function = nullptr,
                  IPluginFunction *log_post_function = nullptr,
                  IPluginFunction *flush_function = nullptr);
    ~callback_sink() override;

    void set_log_callback(IPluginFunction *log_function);
    void set_log_post_callback(IPluginFunction *log_post_function);
    void set_flush_callback(IPluginFunction *flush_function);

private:
    IChangeableForward *log_callback_{nullptr};
    IChangeableForward *log_post_callback_{nullptr};
    IChangeableForward *flush_callback_{nullptr};

    void sink_it_(const details::log_msg &log_msg) override;
    void flush_() override;

    void release_forwards_();
};


}       // namespace sinks
}       // namespace log4sp
