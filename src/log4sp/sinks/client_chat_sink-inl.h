#ifndef _LOG4SP_SINKS_CLIENT_CHAT_SINK_INL_H_
#define _LOG4SP_SINKS_CLIENT_CHAT_SINK_INL_H_

#include "log4sp/sinks/client_chat_sink.h"

namespace log4sp {
namespace sinks {

template <typename Mutex>
client_chat_sink<Mutex>::~client_chat_sink()
{
    if (player_filter_forward_ != nullptr)
    {
        forwards->ReleaseForward(player_filter_forward_);
    }
}

template <typename Mutex>
inline bool client_chat_sink<Mutex>::set_player_filter(IPluginFunction *filter)
{
    if (filter == nullptr)
    {
        return false;
    }

    if (player_filter_forward_ != nullptr)
    {
        forwards->ReleaseForward(player_filter_forward_);
    }

    player_filter_forward_ = forwards->CreateForwardEx(nullptr, ET_Ignore, 4, nullptr, Param_Cell, Param_String, Param_Cell, Param_String);
    if (player_filter_forward_ == nullptr)
    {
        return false;
    }

    return player_filter_forward_->AddFunction(filter);
}

template <typename Mutex>
inline void client_chat_sink<Mutex>::sink_it_(const spdlog::details::log_msg &msg)
{
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
    auto message = fmt::to_string(formatted).c_str();

    auto name = msg.logger_name.data();
    auto lvl = static_cast<cell_t>(msg.level);
    auto payload = msg.payload.data();

    auto max_clients = playerhelpers->GetMaxClients();
    for (int client = 1; client <= max_clients; ++client)
    {
        auto player = playerhelpers->GetGamePlayer(client);
        if (!player->IsInGame() || player->IsFakeClient())
        {
            continue;
        }

        if (player_filter_forward_ != nullptr)
        {
            player_filter_forward_->PushCell(client);
            player_filter_forward_->PushString(name);
            player_filter_forward_->PushCell(lvl);
            player_filter_forward_->PushString(payload);

            cell_t result = Pl_Continue;
            player_filter_forward_->Execute(&result);

            if (result == Pl_Handled || result == Pl_Changed)
            {
                continue;
            }

            if (result == Pl_Stop)
            {
                return;
            }
        }

        gamehelpers->TextMsg(client, TEXTMSG_DEST_CHAT, message);
    }
}


}       // namespace sinks
}       // namespace log4sp
#endif  // _LOG4SP_SINKS_CLIENT_CHAT_SINK_INL_H_
