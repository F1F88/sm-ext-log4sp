#ifndef _LOG4SP_PROXY_SYNC_LOGGER_PROXY_H_
#define _LOG4SP_PROXY_SYNC_LOGGER_PROXY_H_

#include "extension.h"

#include "log4sp/proxy/logger_proxy.h"


namespace log4sp {

class sync_logger_proxy : public spdlog::logger,
                          public logger_proxy {
public:
    explicit sync_logger_proxy(std::string name)
        : spdlog::logger(std::move(name)) {}

    template <typename It>
    sync_logger_proxy(std::string name, It begin, It end)
        : spdlog::logger(std::move(name), begin, end) {}

    sync_logger_proxy(std::string name, spdlog::sink_ptr single_sink)
        : spdlog::logger(std::move(name), std::move(single_sink)) {}

    sync_logger_proxy(std::string name, spdlog::sinks_init_list sinks)
        : spdlog::logger(std::move(name), std::move(sinks)) {}

    ~sync_logger_proxy() override;

    void add_sink(spdlog::sink_ptr sink) override;

    void remove_sink(spdlog::sink_ptr sink) override;

protected:
    // 重写以在捕获到异常时自定义处理方案
    void sink_it_(const spdlog::details::log_msg &msg) override;
    void flush_() override;
};


}       // namespace log4sp
#include "log4sp/proxy/sync_logger_proxy-inl.h"
#endif  // _LOG4SP_PROXY_SYNC_LOGGER_PROXY_H_
