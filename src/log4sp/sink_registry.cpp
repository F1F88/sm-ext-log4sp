#include <log4sp/sink_registry.h>

namespace log4sp {

void SinkHandleRegistry::registerSink(Handle_t handle, SinkHandleInfo info)
{
    sinks_[handle] = info;
}

void SinkHandleRegistry::registerSink(Handle_t handle, HandleType_t type, spdlog::sink_ptr sink)
{
    registerSink(handle, {type, sink});
}

bool SinkHandleRegistry::hasKey(Handle_t handle)
{
    return sinks_.find(handle) != sinks_.end();
}

SinkHandleInfo *SinkHandleRegistry::get(Handle_t handle)
{
    auto it = sinks_.find(handle);
    if (it == sinks_.end())
    {
        return nullptr;
    }
    return &it->second;
}

spdlog::sink_ptr SinkHandleRegistry::getSink(Handle_t handle)
{
    auto it = sinks_.find(handle);
    if (it == sinks_.end())
    {
        return nullptr;
    }
    return it->second.sink;
}

HandleType_t SinkHandleRegistry::getHandleType(Handle_t handle)
{
    auto it = sinks_.find(handle);
    if (it == sinks_.end())
    {
        return 0;
    }
    return it->second.type;
}

bool SinkHandleRegistry::drop(spdlog::sinks::sink *sink)
{
    auto it = sinks_.begin();
    while (it != sinks_.end())
    {
        if (it->second.sink.get() == sink)
        {
            sinks_.erase(it);
            return true;
        }
        ++it;
    }
    return false;
}

bool SinkHandleRegistry::drop(Handle_t handle)
{
    return sinks_.erase(handle) != 0;
}

void SinkHandleRegistry::dropAll()
{
    return sinks_.clear();
}

void SinkHandleRegistry::shutdown()
{
    dropAll();
}

SinkHandleRegistry &SinkHandleRegistry::instance()
{
    static SinkHandleRegistry singleInstance;
    return singleInstance;
}

}   // namespace log4sp
