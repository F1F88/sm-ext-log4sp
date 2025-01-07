#if defined _log_for_sourcepawn_sinks_base_file_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_base_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


methodmap BaseFileSink < Sink
{
    /**
     * Trivial file sink with single file as target.
     *
     * @param file          The file path where the log messages will be written.
     * @param truncate      If true, the created file will be truncated.
     * @param multiThread   If true, the created sink will be multi threaded and thread safe.
     * @return              BaseFileSink handle.
     */
    public native BaseFileSink(const char[] file, bool truncate = false, bool multiThread = false);

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer    Buffer to store file name.
     * @param maxlen    Maximum length of the buffer.
     * @return          Number of characters written to the buffer, not including the null terminator.
     */
    public native int GetFilename(char[] buffer, int maxlen);

    public native void Truncate();
}
