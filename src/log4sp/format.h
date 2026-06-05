#pragma once

#include "extension.h"

#include "log4sp/common.h"


namespace Log4sp {

[[nodiscard]]
std::string FormatToString(SourcePawn::IPluginContext *ctx, const cell_t *params, const unsigned int param);

[[nodiscard]]
spdlog::memory_buf_t FormatToBuffer(SourcePawn::IPluginContext *ctx, const char *format, const cell_t *params, unsigned int *param);


}   // namespace Log4sp
