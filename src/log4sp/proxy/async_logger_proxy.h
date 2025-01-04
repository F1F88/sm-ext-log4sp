#ifndef _LOG4SP_PROXY_ASYNC_LOGGER_PROXY_H_
#define _LOG4SP_PROXY_ASYNC_LOGGER_PROXY_H_

#include <mutex>

#include "spdlog/async_logger.h"
#include "spdlog/sinks/dist_sink.h"
#include "spdlog/details/backend_worker.h"
#include "spdlog/details/thread_pool.h"

#include "log4sp/proxy/logger_proxy.h"


namespace log4sp {

/**
 * 为了实现这个类，我们修改了 spdlog::async_logger 源码
 */
class async_logger_proxy final : public logger_proxy,
                                 public spdlog::details::backend_worker {
public:
    async_logger_proxy(std::string name,
                       std::shared_ptr<spdlog::sinks::dist_sink_mt> sink,
                       std::weak_ptr<spdlog::details::thread_pool> tp,
                       spdlog::async_overflow_policy policy = spdlog::async_overflow_policy::block)
        : logger_proxy(std::move(name), std::move(sink)),
          thread_pool_(std::move(tp)),
          overflow_policy_(policy) {}

    ~async_logger_proxy() override;

    void add_sink(spdlog::sink_ptr sink) override;

    void remove_sink(spdlog::sink_ptr sink) override;

    void set_error_forward(IChangeableForward *forward) override;

    void error_handler(spdlog::source_loc loc, const std::string &msg) override;

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override;
    void flush_() override;
    void backend_sink_it_(const spdlog::details::log_msg &incoming_log_msg) override;
    void backend_flush_() override;

private:
    std::weak_ptr<spdlog::details::thread_pool> thread_pool_;
    spdlog::async_overflow_policy overflow_policy_;
    std::mutex error_mutex_;
};


}       // namespace log4sp
#endif  // _LOG4SP_PROXY_ASYNC_LOGGER_PROXY_H_
