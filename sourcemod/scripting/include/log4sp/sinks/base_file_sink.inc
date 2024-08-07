#if defined _log_for_sourcepawn_sinks_base_file_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_base_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


methodmap BaseFileSinkST < Sink
{
    /**
     * Trivial file sink with single file as target.
     *
     * @param file      The file path where the log messages will be written.
     * @param truncate  If truncate != false , the created file will be truncated.
     * @return          Handle for a single-threaded base file sink.
     * @error           Create sink handle failed.
     */
    public native BaseFileSinkST(const char[] file, bool truncate = false);

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer    Buffer to store file name.
     * @param maxlen    Maximum size of string buffer.
     * @error           Invalid sink handle.
     */
    public native void GetFilename(char[] buffer, int maxlen);
}

methodmap BaseFileSinkMT < Sink
{
    /**
     * Trivial file sink with single file as target.
     *
     * @param file      The file path where the log messages will be written.
     * @param truncate  If truncate != false , the created file will be truncated.
     * @return          Handle for a multi-threaded base file sink.
     * @error           Create sink handle failed.
     */
    public native BaseFileSinkMT(const char[] file, bool truncate = false);

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer    Buffer to store file name.
     * @param maxlen    Maximum size of string buffer.
     * @error           Invalid sink handle.
     */
    public native void GetFilename(char[] buffer, int maxlen);
}
