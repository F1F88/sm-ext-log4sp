#ifndef _LOG4SP_PROXY_ASYNC_LOGGER_PROXY_H_
#define _LOG4SP_PROXY_ASYNC_LOGGER_PROXY_H_

#include "spdlog/async_logger.h"
#include "spdlog/sinks/dist_sink.h"

#include "log4sp/proxy/logger_proxy.h"


namespace log4sp {

/**
 * 为了实现这个类，我们修改了 spdlog::async_logger 源码
 */
class async_logger_proxy final : public spdlog::async_logger,
                                 public logger_proxy {
    friend class spdlog::details::thread_pool;

public:
    async_logger_proxy(std::string name,
                       std::shared_ptr<spdlog::sinks::dist_sink_mt> sink,
                       std::weak_ptr<spdlog::details::thread_pool> tp,
                       spdlog::async_overflow_policy policy = spdlog::async_overflow_policy::block)
        : spdlog::async_logger(std::move(name), std::move(sink), std::move(tp), policy) {}

    ~async_logger_proxy() override;

    void add_sink(spdlog::sink_ptr sink) override;

    void remove_sink(spdlog::sink_ptr sink) override;

    void set_error_forward(IChangeableForward *forward) override;

    void error_handler(spdlog::source_loc loc, const std::string &name, const std::string &msg) override;

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override;
    void flush_() override;
    void backend_sink_it_(const spdlog::details::log_msg &incoming_log_msg);
    void backend_flush_();

private:
    std::mutex error_mutex_;
};


}       // namespace log4sp
#include "log4sp/proxy/async_logger_proxy-inl.h"
#endif  // _LOG4SP_PROXY_ASYNC_LOGGER_PROXY_H_
