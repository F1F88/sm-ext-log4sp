#ifndef _LOG4SP_PROXY_LOGGER_PROXY_H_
#define _LOG4SP_PROXY_LOGGER_PROXY_H_

#include "spdlog/logger.h"

#include "extension.h"


namespace log4sp {

/**
 * 原版 spdlog::logger 存在的问题
 *   1. sinks 和 err_handler_ 是线程不安全的
 *   2. err_handler_ 不是虚函数，不能使用继承和多态
 *   3. err_handler_ 的权限是 protected，不能被外部调用
 *   4. async_logger 被 final 修饰，无法继承，重写可能还涉及线程池部分
 *
 * 方案：
 *   1. 通过继承添加 add_sink 和 remove_sink 方法解决
 *   2. 通过继承重写 sink_it_ 和 flush_ 捕获到异常时的处理方案（）
 *   3. 通过继承添加 error_handler()
 *   4. 修改源码，将 async_logger 设为了非 final 类
 */
class logger_proxy : public spdlog::logger {
public:
    explicit logger_proxy(std::string name)
        : spdlog::logger(std::move(name)) {}

    template <typename It>
    logger_proxy(std::string name, It begin, It end)
        : spdlog::logger(std::move(name), begin, end) {}

    logger_proxy(std::string name, spdlog::sink_ptr single_sink)
        : spdlog::logger(std::move(name), std::move(single_sink)) {}

    logger_proxy(std::string name, spdlog::sinks_init_list sinks)
        : spdlog::logger(std::move(name), std::move(sinks)) {}

    ~logger_proxy() override;

    virtual void add_sink(spdlog::sink_ptr sink);

    virtual void remove_sink(spdlog::sink_ptr sink);

    virtual void set_error_forward(IChangeableForward *forward);

    // sink_it_ 和 flush_ 遇到异常时将调用这个以替代 spdlog::logger 的 err_handler_
    virtual void error_handler(spdlog::source_loc loc, const std::string &msg);

protected:
    IChangeableForward *error_forward_{nullptr};

    // 重写以在捕获到异常时自定义处理方案
    void sink_it_(const spdlog::details::log_msg &msg) override;
    void flush_() override;
};


}       // namespace log4sp
#endif  // _LOG4SP_PROXY_LOGGER_PROXY_H_
