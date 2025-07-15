#include "log4sp/common.h"

namespace log4sp {

using spdlog::filename_t;

[[noreturn]] void throw_log4sp_ex(const std::string &msg, int last_errno) {
    spdlog::throw_spdlog_ex(msg, last_errno);
}

[[noreturn]] void throw_log4sp_ex(std::string msg) {
    spdlog::throw_spdlog_ex(std::move(msg));
}


}       // namespace log4sp
