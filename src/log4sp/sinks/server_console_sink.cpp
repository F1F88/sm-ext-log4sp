#include <cerrno>

#include "spdlog/details/os.h"

// clang-format off
#ifdef _WIN32
    // under windows using fwrite to non-binary stream results in \r\r\n (see issue #1675)
    // so instead we use ::FileWrite
    #include "spdlog/details/windows_include.h"

    #ifndef _USING_V110_SDK71_  // fileapi.h doesn't exist in winxp
        #include <fileapi.h>    // WriteFile (..)
    #endif

    #include <io.h>     // _get_osfhandle(..)
    #include <stdio.h>  // _fileno(..)
#endif                  // _WIN32

#include "log4sp/sinks/server_console_sink.h"


// clang-format on
namespace log4sp {
namespace sinks {

stdout_sink_base::stdout_sink_base(FILE *file)
    : file_(file) {
#ifdef _WIN32
    // get windows handle from the FILE* object

    handle_ = reinterpret_cast<HANDLE>(::_get_osfhandle(::_fileno(file_)));

    // don't throw to support cases where no console is attached,
    // and let the log method to do nothing if (handle_ == INVALID_HANDLE_VALUE).
    // throw only if non stdout/stderr target is requested (probably regular file and not console).
    if (handle_ == INVALID_HANDLE_VALUE && file != stdout && file != stderr) {
        spdlog::memory_buf_t outbuf;
        spdlog::fmt_lib::format_system_error(outbuf, errno, "spdlog::stdout_sink_base: _get_osfhandle() failed");
        throw spdlog::fmt_lib::to_string(outbuf);
    }
#endif  // _WIN32
}

void stdout_sink_base::sink_it_(const details::log_msg &msg) {
#ifdef _WIN32
    if (handle_ == INVALID_HANDLE_VALUE) {
        return;
    }

    spdlog::memory_buf_t formatted;
    base_sink::formatter_->format(msg, formatted);
    auto size = static_cast<DWORD>(formatted.size());
    DWORD bytes_written = 0;
    bool ok = ::WriteFile(handle_, formatted.data(), size, &bytes_written, nullptr) != 0;
    if (!ok) {
        auto msg{spdlog::fmt_lib::format("stdout_sink_base: WriteFile() failed. GetLastError(): {}", std::to_string(::GetLastError()))};
        spdlog::memory_buf_t outbuf;
        spdlog::fmt_lib::format_system_error(outbuf, errno, msg.c_str());
        throw spdlog::fmt_lib::to_string(outbuf);
    }
#else
    spdlog::memory_buf_t formatted;
    base_sink::formatter_->format(msg, formatted);
    details::os::fwrite_bytes(formatted.data(), formatted.size(), file_);
#endif                // _WIN32
    ::fflush(file_);  // flush every line to terminal
}

void stdout_sink_base::flush_() {
    fflush(file_);
}


}   // namespace sinks
}   // namespace log4sp
