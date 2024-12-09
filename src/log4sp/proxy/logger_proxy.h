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
 *   1. 通过继承 logger_proxy 接口，重写接口解决并发安全问题
 *   2. 通过继承 spdlog::logger 类，重写 sink_it_ 和 flush_ 捕获到异常时的处理方案（调用 logger_proxy 接口的 error_handle 方法）
 *   3. 通过继承添加 error_handler()
 *   4. 修改源码，将 async_logger 设为了非 final 类
 */
class logger_proxy {
public:
    logger_proxy() : error_forward_(nullptr) {}
    ~logger_proxy() {
        release_error_forward();
    }

    virtual void add_sink(spdlog::sink_ptr sink) = 0;

    virtual void remove_sink(spdlog::sink_ptr sink) = 0;

    virtual void set_error_forward(IChangeableForward *forward) {
        if (error_forward_ != nullptr) {
            forwards->ReleaseForward(error_forward_);
        }
        error_forward_ = forward;
    }

    virtual void release_error_forward() {
        if (error_forward_ != nullptr) {
            forwards->ReleaseForward(error_forward_);
            error_forward_ = nullptr;
        }
    }

    virtual void error_handler(spdlog::source_loc loc, const std::string &name, const std::string &msg) {
        if (error_forward_ != nullptr) {
            SPDLOG_TRACE("loc: Line {}, {}::{}", loc.line, loc.filename, loc.funcname);
            SPDLOG_TRACE("name: {}", name);

            error_forward_->PushString(name.c_str());
            error_forward_->PushString(msg.c_str());
            error_forward_->Execute();
        } else {
            static size_t err_counter = 0;
            spdlog::log(loc, spdlog::level::err, "[{}] [{}] {}", ++err_counter, name, msg);
        }
    }

protected:
    IChangeableForward *error_forward_;     // 通过 set_error_forward() 赋值
};


}       // namespace log4sp
#endif  // _LOG4SP_PROXY_LOGGER_PROXY_H_
