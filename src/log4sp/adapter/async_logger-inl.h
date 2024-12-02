#ifndef _LOG4SP_ADAPTER_ASYNC_LOGGER_INL_H_
#define _LOG4SP_ADAPTER_ASYNC_LOGGER_INL_H_

#include "spdlog/sinks/dist_sink.h"

#include "log4sp/utils.h"
#include "log4sp/logger_register.h"
#include "log4sp/adapter/async_logger.h"


namespace log4sp {

inline std::shared_ptr<async_logger> async_logger::create(std::shared_ptr<spdlog::async_logger> logger,
                                                          IPluginContext *ctx) {
    HandleSecurity security = {nullptr, myself->GetIdentity()};
    HandleError error;

    std::shared_ptr<async_logger> logger_adapter = create(logger, &security, nullptr, &error);
    if (logger_adapter == nullptr) {
        ctx->ReportError("SM Error! Allocation of asynchronous logger handle failed. (err=%d)", error);
        return nullptr;
    }

    return logger_adapter;
}

inline std::shared_ptr<async_logger> async_logger::create(std::shared_ptr<spdlog::async_logger> logger,
                                                          const HandleSecurity *security,
                                                          const HandleAccess *access,
                                                          HandleError *error) {
    // 1. 创建适配器
    // auto logger_adapter = std::make_shared<async_logger>(logger);
    auto logger_adapter = std::shared_ptr<async_logger>(new async_logger(logger));

    // 2. 为适配器创建 handle
    Handle_t handle = handlesys->CreateHandleEx(g_LoggerHandleType, logger_adapter.get(), security, access, error);
    if (handle == BAD_HANDLE) {
        return nullptr;
    }

    // 3. 自定义 (适配) sinks - AddSink / DropSink
    // ! 必须保证第一个 sink 是 dist_sink_mt（否则 add_sink/remove_sink 时会造成崩溃）
    auto dist_sink = std::dynamic_pointer_cast<spdlog::sinks::dist_sink_mt>(logger->sinks().front());
    if (dist_sink == nullptr) {
        dist_sink = std::make_shared<spdlog::sinks::dist_sink_mt>();
        for (auto sink : logger->sinks()) {
            dist_sink->add_sink(sink);
        }

        logger->sinks().clear();
        logger->sinks().push_back(dist_sink);
    }

    // 4. 自定义 (适配) error handler
    logger->set_error_handler([logger_adapter](const std::string &msg) {
        logger_adapter->error_handler(msg);
    });

    // 5. 保存 handle
    logger_adapter->handle_ = handle;

    // 6. 注册适配器到 logger register
    logger_register::instance().register_logger(logger_adapter);
    return logger_adapter;
}

inline void async_logger::add_sink(spdlog::sink_ptr sink) {
    auto dist_sink = std::static_pointer_cast<spdlog::sinks::dist_sink_mt>(raw_->sinks().front());
    dist_sink->add_sink(sink);
}

inline void async_logger::remove_sink(spdlog::sink_ptr sink) {
    auto dist_sink = std::static_pointer_cast<spdlog::sinks::dist_sink_mt>(raw_->sinks().front());
    dist_sink->remove_sink(sink);
}

inline void async_logger::set_error_forward(IChangeableForward *forward) {
    std::lock_guard<std::mutex> lock(error_handler_mutex_);
    if (error_forward_ != nullptr) {
        forwards->ReleaseForward(error_forward_);
    }
    error_forward_ = forward;
}

inline void async_logger::error_handler(const std::string &msg) {
    std::lock_guard<std::mutex> lock(error_handler_mutex_);
    if (error_forward_ != nullptr) {
        error_forward_->PushCell(handle_);
        error_forward_->PushString(msg.c_str());
        error_forward_->Execute();
    } else {
        using std::chrono::system_clock;
        static std::chrono::system_clock::time_point last_report_time;
        static size_t err_counter = 0;
        auto now = system_clock::now();
        err_counter++;
        if (now - last_report_time < std::chrono::seconds(1)) {
            return;
        }
        last_report_time = now;
        auto tm_time = spdlog::details::os::localtime(system_clock::to_time_t(now));
        char date_buf[64];
        std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", &tm_time);
#if defined(USING_R) && defined(R_R_H)  // if in R environment
        REprintf("[*** LOG ERROR #%04zu ***] [%s] [%s] %s\n", err_counter, date_buf, name().c_str(),
                 msg.c_str());
#else
        std::fprintf(stderr, "[*** LOG ERROR #%04zu ***] [%s] [%s] %s\n", err_counter, date_buf,
                     raw_->name().c_str(), msg.c_str());
#endif
    }
}


}       // namespace log4sp
#endif  // _LOG4SP_ADAPTER_ASYNC_LOGGER_INL_H_
