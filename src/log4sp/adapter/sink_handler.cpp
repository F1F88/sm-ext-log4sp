#include <cassert>

#include "log4sp/adapter/sink_handler.h"


namespace Log4sp {

[[nodiscard]]
SinkHandler &SinkHandler::Instance() noexcept
{
    static SinkHandler instance;
    return instance;
}

void SinkHandler::Initialize()
{
    Instance().Initialize_();
}

void SinkHandler::Destroy() noexcept
{
    Instance().Destroy_();
}


[[nodiscard]]
SourceMod::HandleType_t SinkHandler::HandleType() const noexcept
{
    return m_HandleType;
}

[[nodiscard]]
SourceMod::Handle_t SinkHandler::CreateHandle(SinkPtr object, const SourceMod::HandleSecurity *security, const SourceMod::HandleAccess *access, SourceMod::HandleError *error) noexcept
{
    assert(m_HandleType);

    SourceMod::Handle_t handle = handlesys->CreateHandleEx(m_HandleType, object.get(), security, access, error);
    if (!handle)
        return BAD_HANDLE;

    assert(m_Handles.find(object.get()) == m_Handles.end());
    assert(m_Sinks.find(object.get()) == m_Sinks.end());

    m_Handles[object.get()] = handle;
    m_Sinks[object.get()] = object;

    return handle;
}

[[nodiscard]]
spdlog::sink_ptr SinkHandler::ReadHandle(SourceMod::Handle_t handle, SourceMod::HandleSecurity *security, SourceMod::HandleError *error) const noexcept
{
    assert(m_HandleType);

    Sink *object;
    SourceMod::HandleError err = handlesys->ReadHandle(handle, m_HandleType, security, (void **)&object);
    if (err != SourceMod::HandleError_None)
    {
        if (error)
            *error = err;
        return nullptr;
    }

    assert(m_Sinks.find(object) != m_Sinks.end());
    return m_Sinks.find(object)->second;
}

[[nodiscard]]
spdlog::sinks::sink *SinkHandler::ReadHandleRaw(SourceMod::Handle_t handle, SourceMod::HandleSecurity *security, SourceMod::HandleError *error) const noexcept
{
    assert(m_HandleType);

    Sink *object;
    SourceMod::HandleError err = handlesys->ReadHandle(handle, m_HandleType, security, (void **)&object);
    if (err != SourceMod::HandleError_None)
    {
        if (error)
            *error = err;
        return nullptr;
    }

    assert(m_Sinks.find(object) != m_Sinks.end());
    return object;
}

void SinkHandler::OnHandleDestroy(SourceMod::HandleType_t type, void *object)
{
    auto sink_obj = static_cast<Sink*>(object);

    assert(m_Handles.find(sink_obj) != m_Handles.end());
    assert(m_Sinks.find(sink_obj) != m_Sinks.end());

    m_Handles.erase(sink_obj);
    m_Sinks.erase(sink_obj);
}


void SinkHandler::Initialize_()
{
    using spdlog::fmt_lib::format;

    SourceMod::HandleAccess access;
    SourceMod::HandleError error;

    // Init plugin create Sinks access
    // 插件创建的 Sink Handle 可以被任意插件释放
    handlesys->InitAccessDefaults(nullptr, &access);
    access.access[SourceMod::HandleAccess_Delete] = 0;

    m_HandleType = handlesys->CreateType("Sink", this, 0, nullptr, &access, myself->GetIdentity(), &error);
    if (!m_HandleType)
        ThrowLog4spEx(format("Failed to creates a Sink Handle type (error code: {})", static_cast<int>(error)));
}

void SinkHandler::Destroy_() noexcept
{
    if (m_HandleType)
    {
        handlesys->RemoveType(m_HandleType, myself->GetIdentity());
        m_HandleType = NO_HANDLE_TYPE;
    }
}


}       // namespace Log4sp
