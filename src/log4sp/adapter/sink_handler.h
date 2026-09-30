#pragma once

#include "spdlog/sinks/sink.h"
#include "log4sp/adapter/handle_type_dispatch_adapter.h"


namespace Log4sp {

inline constexpr const char SINK_HANDLE_NAME[] = "Sink";

class SinkHandler final :
    public IHandleTypeDispatchAdapter<spdlog::sinks::sink*, SINK_HANDLE_NAME> {};

}       // namespace Log4sp
