#pragma once

#include "extension.h"

#include "log4sp/common.h"


namespace log4sp {

[[nodiscard]] std::string format_to_string(SourcePawn::IPluginContext *ctx, const cell_t *params, const unsigned int param);

[[nodiscard]] spdlog::memory_buf_t format_to_buffer(SourcePawn::IPluginContext *ctx, const char *format, const cell_t *params, unsigned int *param);


}   // namespace log4sp
