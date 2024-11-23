#ifndef _LOG4SP_CLIENT_CONSOLE_SINK_H_
#define _LOG4SP_CLIENT_CONSOLE_SINK_H_

#include "spdlog/details/synchronous_factory.h"
#include "spdlog/sinks/base_sink.h"

#include "extension.h"


namespace log4sp {
namespace sinks {

template<typename Mutex>
class client_console_sink : public spdlog::sinks::base_sink<Mutex>
{
public:
    ~client_console_sink();

    bool set_player_filter(IPluginFunction *filter);

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override;

    void flush_() override;

    cell_t player_filter_(const int client, const char *name, const spdlog::level::level_enum lvl, const char *msg);

    IChangeableForward *player_filter_forward_;
};

using client_console_sink_mt = client_console_sink<std::mutex>;
using client_console_sink_st = client_console_sink<spdlog::details::null_mutex>;

} // namespace sinks

//
// factory functions
//
template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<spdlog::logger> client_console_logger_mt()
{
    return Factory::template create<log4sp::sinks::client_console_sink_mt>();
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<spdlog::logger> client_console_logger_st()
{
    return Factory::template create<log4sp::sinks::client_console_sink_st>();
}

} // namespace log4sp

#include "client_console_sink-inl.h"

#endif // _LOG4SP_CLIENT_CONSOLE_SINK_H_
