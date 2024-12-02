#ifndef _LOG4SP_LOGGER_REGISTER_H_
#define _LOG4SP_LOGGER_REGISTER_H_

#include <unordered_map>

#include "extension.h"

#include "log4sp/adapter/base_logger.h"


namespace log4sp {

/**
 * 参考 "spdlog/details/registry.h"
 * 但速度应该更快，因为单线程不需要锁
 */
class logger_register final {
public:
    static logger_register &instance() {
        static logger_register singleInstance;
        return singleInstance;
    }

    void register_logger(std::shared_ptr<base_logger> logger_adapter) {
        loggers_[logger_adapter->raw()->name()] = std::move(logger_adapter);
    }

    std::shared_ptr<base_logger> get(const std::string &name) {
        auto found = loggers_.find(name);
        return found == loggers_.end() ? nullptr : found->second;
    }

    /**
     * 应该只在 LoggerHandler::OnHandleDestroy 中使用
     */
    void drop(const std::string &name) {
        loggers_.erase(name);
    }

    /**
     * 应该只在 Log4sp::SDK_OnUnload 中使用
     */
    void drop_all() {
        loggers_.clear();
    }

    /**
     * 应该只在 Log4sp::SDK_OnUnload 中使用
     * drop_all() 的包装器
     */
    void shutdown() {
        drop_all();
    }

private:
    logger_register() {}
    ~logger_register() {}

    std::unordered_map<std::string, std::shared_ptr<base_logger>> loggers_;
};


}       // namespace log4sp
#endif  // _LOG4SP_LOGGER_REGISTER_H_
