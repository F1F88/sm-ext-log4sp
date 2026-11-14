#pragma once

#include "log4sp/logger.h"
#include "log4sp/adapter/handle_type_dispatch_adapter.h"


namespace Log4sp {

inline constexpr const char LOGGER_HANDLE_NAME[] = "Logger";

class LoggerHandler final :
    public IHandleTypeDispatchAdapter<Logger*, LOGGER_HANDLE_NAME> {};

}       // namespace Log4sp
