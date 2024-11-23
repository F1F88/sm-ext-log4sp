#ifndef _LOG4SP_CLIENT_CONSOLE_SINK_INL_H_
#define _LOG4SP_CLIENT_CONSOLE_SINK_INL_H_

#include <log4sp/client_console_sink.h>

namespace log4sp {
namespace sinks {

template <typename Mutex>
client_console_sink<Mutex>::~client_console_sink()
{
    if (player_filter_forward_ != nullptr)
    {
        forwards->ReleaseForward(player_filter_forward_);
    }
}

template <typename Mutex>
inline bool client_console_sink<Mutex>::set_player_filter(IPluginFunction *filter)
{
    if (filter == nullptr)
    {
        return false;
    }

    if (player_filter_forward_ != nullptr)
    {
        forwards->ReleaseForward(player_filter_forward_); // 清空 forward function
    }

    player_filter_forward_ = forwards->CreateForwardEx(NULL, ET_Ignore, 4, NULL, Param_Cell, Param_String, Param_Cell, Param_String);
    if (player_filter_forward_ == nullptr)
    {
        return false;
    }

    return player_filter_forward_->AddFunction(filter);
}

template <typename Mutex>
inline void client_console_sink<Mutex>::sink_it_(const spdlog::details::log_msg &msg)
{
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

    auto name = msg.logger_name.data();
    auto lvl = msg.level;

    std::string message(formatted.data(), formatted.size());
    auto logMessage = message.c_str();

    const int maxClients = playerhelpers->GetMaxClients();
    for (int client = 1; client <= maxClients; ++client)
    {
        auto player = playerhelpers->GetGamePlayer(client);
        if (!player->IsInGame() || player->IsFakeClient())
        {
            continue; // PrintToConsole 里也做了同样的判断, 这是为了减少 call filter forward 次数
        }

        cell_t result = player_filter_(client, name, lvl, logMessage);
        if (result == Pl_Stop)
        {
            return;
        }

        if (result == Pl_Handled || result == Pl_Changed)
        {
            continue;
        }

        player->PrintToConsole(logMessage);
    }
}

template <typename Mutex>
inline void client_console_sink<Mutex>::flush_()
{
    // No need to do anything...
}

template <typename Mutex>
inline cell_t client_console_sink<Mutex>::player_filter_(const int client, const char *name, const spdlog::level::level_enum lvl, const char *msg)
{
    if (player_filter_forward_ == nullptr)
    {
        return Pl_Continue;
    }

    cell_t result = 0;
    player_filter_forward_->PushCell(client);
    player_filter_forward_->PushString(name);
    player_filter_forward_->PushCell(lvl);
    player_filter_forward_->PushString(msg);
    player_filter_forward_->Execute(&result);
    return result;
}

} // namespace sinks
} // namespace log4sp


#endif // _LOG4SP_CLIENT_CONSOLE_SINK_INL_H_
