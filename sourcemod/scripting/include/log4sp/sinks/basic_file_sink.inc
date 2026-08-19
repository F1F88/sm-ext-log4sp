#if defined _log4sp_sinks_basic_file_sink_included
    #endinput
#endif
#define _log4sp_sinks_basic_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>

#if defined LOG4SP_HEADER_ONLY
#include <log4sp/details/file_helper>
#endif      // LOG4SP_HEADER_ONLY


/**
 * Writes log messages directly to files with efficient performance, making it
 * ideal for persistent storage and long-term log tracking in applications.
 */
methodmap BasicFileSink < Sink
{
#if !defined LOG4SP_HEADER_ONLY
    /**
     * Trivial file sink with single file as target.
     *
     * @note BasicFileSink handles must be freed via delete or CloseHandle().
     *
     * @param filename      The file path where the log messages will be written.
     * @param truncate      If true, the created file will be truncated.
     * @param openPlugin    Handle of the plugin that contains the file open function.
     *                      If null, defaults to the calling plugin.
     * @param openFunc      Function to call before the sink open the file.
     * @param closePlugin   Handle of the plugin that contains the file close function.
     *                      If null, defaults to the calling plugin.
     * @param closeFunc     Function to call after the sink close the file.
     * @return              A new BasicFileSink Handle.
     */
    public native BasicFileSink(const char[] filename,
                                bool truncate=false,
                                Handle openPlugin=null,  FileOpenFunction openFunc=INVALID_FUNCTION,
                                Handle closePlugin=null, FileCloseFunction closeFunc=INVALID_FUNCTION);

    /**
     * Get the current filename being used by the file sink.
     *
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum length of the buffer.
     * @return              Number of bytes written.
     */
    public native int GetFilename(char[] buffer, int maxlen);

    /**
     * Truncate log files on demand.
     *
     * @param error         Error message buffer in case of truncate failure.
     * @param maxlen        Maximum length of the error.
     * @return              True on success, false on error.
     */
    public native bool Truncate(char[] error="", int maxlen=0);

    /**
     * Returns whether or not a basic file sink handle is valid.
     *
     * @return          True if valid, false otherwise.
     */
    public native bool IsValid();

#else

    /**
     * Trivial file sink with single file as target.
     *
     * @note BasicFileSink handles must be freed via Sink.Close().
     * @note Multi-plugin shared Handles must be freed before unload the plugin.
     *
     * @param file          The file path where the log messages will be written.
     * @param truncate      If true, the file will be truncated.
     * @param openPlugin    Handle of the plugin that contains the file open function.
     *                      If null, defaults to the calling plugin.
     * @param openFunc      Function to call before the sink open the file.
     * @param closePlugin   Handle of the plugin that contains the file close function.
     *                      If null, defaults to the calling plugin.
     * @param closeFunc     Function to call after the sink close the file.
     * @return              A new BasicFileSink Handle.
     */
    public BasicFileSink(const char[] filename,
                         bool truncate=false,
                         Handle openPlugin=null,  FileOpenFunction openFunc=INVALID_FUNCTION,
                         Handle closePlugin=null, FileCloseFunction closeFunc=INVALID_FUNCTION) {
        BasicFileSink sink = view_as<BasicFileSink>(Sink.__Sink("BasicFileSink",
                                                                null, __BasicFileSink_OnLog,
                                                                null, __BasicFileSink_OnFlush,
                                                                null, __BasicFileSink_OnClose));
        sink.__SetFileHelper(new FileHelper(openPlugin, openFunc, closePlugin, closeFunc));

        char error[LOG4SP_HEADER_ONLY_MAX_ERR_LENGTH];
        if (!sink.__GetFileHelper().Open(filename, truncate, error, sizeof(error))) {
            sink.Close();
            ThrowError("%s", error);
        }
        return sink;
    }

    public int GetFilename(char[] buffer, int maxlen) {
        if (!this.IsValid())
            ThrowError("Invalid BasicFileSink Handle %x.", this);
        return this.__GetFileHelper().GetFilename(buffer, maxlen);
    }

    public bool Truncate(char[] error="", int maxlen=0) {
        if (!this.IsValid())
            ThrowError("Invalid BasicFileSink Handle %x.", this);
        return this.__GetFileHelper().Reopen(true, error, maxlen);
    }

    public bool IsValid() {
        if (!view_as<Sink>(this).IsValid())
            return false;

        if (!this.__GetFileHelper().IsValid())
            return false;

        char classname[16];
        this.__GetClassname(classname, sizeof(classname));
        return StrEqual(classname, "BasicFileSink");
    }


    // *** Do not use the function below this line! *** //
    // Ownership and lifecycle are the same as this, closed in OnClose.
    public FileHelper __GetFileHelper()                 { return this.__GetValue("m_hFileHelper"); }
    public void __SetFileHelper(FileHelper fileHelper)  { this.__SetValue("m_hFileHelper", fileHelper); }
#endif      // LOG4SP_HEADER_ONLY
}


#if defined LOG4SP_HEADER_ONLY
public bool __BasicFileSink_OnLog(BasicFileSink sink, char[] error, int maxlen, const char[] logTime, LOG4SP_ES_CONST SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
{
    int len = strlen(logTime);
    char seconds[21], millisecond[4];
    strcopy(seconds, len - 8, logTime);
    strcopy(millisecond, sizeof(millisecond), logTime[len - 9]);

    char date[20]; // 2026-01-01 12:34:56
    FormatTime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", StringToInt(seconds)); // FIXME: Possible Year 2038 Problem

    char source[256];
    if (!loc.IsEmpty())
    {
        int sepOffset = 0;
        for (int i = 0; i < sizeof(SourceLoc::filename); ++i)
        {
            if (!loc.filename[i])
                break;
            if (loc.filename[i] == '\\' || loc.filename[i] == '/')
                sepOffset = i + 1;
        }
        FormatEx(source, sizeof(source), "[%s::%d] ", loc.filename[sepOffset], loc.line);
    }

    char level[6];
    LogLevelToName(level, sizeof(level), lvl);

    char buffer[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
    len = FormatEx(buffer, sizeof(buffer), "[%s.%s] [%s] [%s] %s%s\n", date, millisecond, name, level, source, msg);
    buffer[len - 1] = '\n'; // buffer[len] of FormatEx must be '\0'.

    return sink.__GetFileHelper().Write(buffer, error, maxlen);
}

public bool __BasicFileSink_OnFlush(BasicFileSink sink, char[] error, int maxlen)
{
    return sink.__GetFileHelper().Flush(error, maxlen);
}

public void __BasicFileSink_OnClose(BasicFileSink sink)
{
    sink.__GetFileHelper().Close();
    sink.__SetFileHelper(null);
}
#endif      // LOG4SP_HEADER_ONLY
