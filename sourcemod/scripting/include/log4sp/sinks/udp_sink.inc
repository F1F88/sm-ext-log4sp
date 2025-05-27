#if defined _log_for_sourcepawn_sinks_udp_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_udp_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/logger>
#include <log4sp/sinks/sink>


/**
 * Simple udp client sink.
 * Sends formatted log via udp.
 */
methodmap UDPSink < Sink
{
    /**
     * Simple udp client sink.
     * Sends formatted log via udp.
     *
     * @param host          Target host name or IP address.
     * @param port          Target port.
     * @return              UDPSink handle.
     * @error               Invalid target address.
     */
    public native UDPSink(const char[] host, int port);

    /**
     * Create a logger handle that sends formatted log via udp.
     *
     * @param host          Target host name or IP address.
     * @param port          Target port.
     * @return              Logger handle.
     * @error               Logger name already exits, or invalid target address.
     */
    public static native Logger CreateLogger(const char[] name, const char[] host, int port);
}
