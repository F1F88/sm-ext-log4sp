#include <cassert>
#include <string>

#include "spdlog/sinks/stdout_sinks.h"

#include "log4sp/adapter/logger_handler.h"


namespace Log4sp {


[[nodiscard]]
LoggerHandler &LoggerHandler::Instance() noexcept
{
    static LoggerHandler instance;
    return instance;
}

void LoggerHandler::Initialize()
{
    Instance().Initialize_();
}

void LoggerHandler::Destroy() noexcept
{
    Instance().Destroy_();
}


[[nodiscard]]
SourceMod::HandleType_t LoggerHandler::HandleType() const noexcept
{
    return m_HandleType;
}

[[nodiscard]]
SourceMod::Handle_t LoggerHandler::CreateHandle(std::shared_ptr<Logger> object, const SourceMod::HandleSecurity *security, const SourceMod::HandleAccess *access, SourceMod::HandleError *error) noexcept
{
    assert(m_HandleType);

    SourceMod::Handle_t handle = handlesys->CreateHandleEx(m_HandleType, object.get(), security, access, error);
    if (!handle)
        return BAD_HANDLE;

    assert(m_Handles.find(object->Name()) == m_Handles.end());
    assert(m_Loggers.find(object->Name()) == m_Loggers.end());

    m_Handles[object->Name()] = handle;
    m_Loggers[object->Name()] = object;

    return handle;
}

[[nodiscard]]
std::shared_ptr<Logger> LoggerHandler::ReadHandle(const SourceMod::Handle_t handle, const SourceMod::HandleSecurity *security, SourceMod::HandleError *error) const noexcept
{
    assert(m_HandleType);

    Logger *object;
    SourceMod::HandleError err = handlesys->ReadHandle(handle, m_HandleType, security, (void **)&object);
    if (err != SourceMod::HandleError_None)
    {
        if (error)
            *error = err;
        return nullptr;
    }

    assert(m_Loggers.find(object->Name()) != m_Loggers.end());
    return m_Loggers.find(object->Name())->second;
}

[[nodiscard]]
Logger *LoggerHandler::ReadHandleRaw(const SourceMod::Handle_t handle, const SourceMod::HandleSecurity *security, SourceMod::HandleError *error) const noexcept
{
    assert(m_HandleType);

    Logger *object;
    SourceMod::HandleError err = handlesys->ReadHandle(handle, m_HandleType, security, (void **)&object);
    if (err != SourceMod::HandleError_None)
    {
        if (error)
            *error = err;
        return nullptr;
    }

    assert(m_Loggers.find(object->Name()) != m_Loggers.end());
    return object;
}

[[nodiscard]]
SourceMod::Handle_t LoggerHandler::FindHandle(const std::string &name) const noexcept
{
    auto found = m_Handles.find(name);
    return found == m_Handles.end() ? BAD_HANDLE : found->second;
}

[[nodiscard]]
std::shared_ptr<Logger> LoggerHandler::FindLogger(const std::string &name) const noexcept
{
    auto found = m_Loggers.find(name);
    return found == m_Loggers.end() ? nullptr : found->second;
}

void LoggerHandler::ApplyAll(const std::function<void(const SourceMod::Handle_t)> &func)
{
    for (auto &h : m_Handles)
        func(h.second);
}

void LoggerHandler::ApplyAll(const std::function<void(std::shared_ptr<Logger>)> &func)
{
    for (auto &l : m_Loggers)
        func(l.second);
}

void LoggerHandler::OnHandleDestroy(SourceMod::HandleType_t type, void *object)
{
    auto logger = static_cast<Logger*>(object);

    assert(m_Handles.find(logger->Name()) != m_Handles.end());
    assert(m_Loggers.find(logger->Name()) != m_Loggers.end());

    m_Handles.erase(logger->Name());
    m_Loggers.erase(logger->Name());
}


void LoggerHandler::Initialize_()
{
    using spdlog::fmt_lib::format;
    using spdlog::sinks::stdout_sink_st;

    SourceMod::HandleAccess access;
    SourceMod::HandleError error;

    // Init plugin create Logger access
    // 插件创建的 Logger Handle 可以被任意插件释放
    handlesys->InitAccessDefaults(nullptr, &access);
    access.access[SourceMod::HandleAccess_Delete] = 0;

    m_HandleType = handlesys->CreateType("Logger", this, 0, nullptr, &access, myself->GetIdentity(), &error);
    if (!m_HandleType)
        ThrowLog4spEx(format("Failed to creates a Logger Handle type (error code: {})", static_cast<int>(error)));

    // Init Global Logger access
    // 拓展创建的全局 Logger Handle 不可以被插件释放, 生命周期由拓展管控
    handlesys->InitAccessDefaults(nullptr, &access);
    access.access[SourceMod::HandleAccess_Delete] |= HANDLE_RESTRICT_IDENTITY;
    SourceMod::HandleSecurity security(myself->GetIdentity(), myself->GetIdentity());

    try
    {
        auto sink = std::make_shared<stdout_sink_st>();
        auto logger = std::make_shared<Logger>(SMEXT_CONF_LOGTAG, sink);
        auto handle = LoggerHandler::Instance().CreateHandle(logger, &security, &access, &error);
        if (!handle)
            ThrowLog4spEx(format("error code: {}", static_cast<int>(error)));
    }
    catch (const std::exception &ex)
    {
        ThrowLog4spEx(format("Failed to creates a Global Logger Handle (reason: {})", ex.what()));
    }

}

void LoggerHandler::Destroy_() noexcept
{
    if (m_HandleType)
    {
        handlesys->RemoveType(m_HandleType, myself->GetIdentity());
        m_HandleType = NO_HANDLE_TYPE;
    }
}


}       // namespace Log4sp
