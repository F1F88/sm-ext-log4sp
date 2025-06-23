#if defined _log_for_sourcepawn_sinks_tcp_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_tcp_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/logger>
#include <log4sp/sinks/sink>


enum struct TCPSinkConfig
{
    char host[256];                     // Target host name or IP address.
    int  port;                          // Target port.
    bool lazyConnect;                   // If true, connect on first log call instead of on construction.
}


/**
 * Simple tcp client sink.
 * Connects to remote address and send the formatted log.
 * Will attempt to reconnect if connection drops.
 */
methodmap TCPSink < Sink
{
    /**
     * Simple tcp client sink.
     * Connects to remote address and send the formatted log.
     * Will attempt to reconnect if connection drops.
     *
     * @param host          Target host name or IP address.
     * @param port          Target port.
     * @param lazyConnect   If true, connect on first log call instead of on construction.
     * @return              TCPSink handle.
     * @error               Failed to connect to TCP host/port.
     */
    public native TCPSink(const char[] host, int port, bool lazyConnect = false);

    /**
     * Create a logger handle that sends formatted log via tcp.
     *
     * @param host          Target host name or IP address.
     * @param port          Target port.
     * @param lazyConnect   If true, connect on first log call instead of on construction.
     * @return              Logger handle.
     * @error               Logger name already exits, or invalid target address.
     */
    public static native Logger CreateLogger(const char[] name, const char[] host, int port, bool lazyConnect = false);
}
