#if defined _log4sp_sinks_rotating_file_sink_included
    #endinput
#endif
#define _log4sp_sinks_rotating_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>

#if defined LOG4SP_HEADER_ONLY
#include <log4sp/details/file_helper>
#endif      // LOG4SP_HEADER_ONLY


/**
 * Automatically rotates log files based on size limits, preventing files from
 * growing too large and ensuring better file management.
 *
 * Rotate files:
 *  log.txt   -> log.1.txt
 *  log.1.txt -> log.2.txt
 *  log.2.txt -> log.3.txt
 *  log.3.txt -> delete
 */
methodmap RotatingFileSink < Sink
{
#if !defined LOG4SP_HEADER_ONLY
    /**
     * Rotating file sink based on size.
     *
     * @note RotatingFileSink handles must be freed via delete or CloseHandle().
     *
     * @param baseFilename  The file path where the log messages will be written.
     * @param maxSize       The maximum size of the log file in bytes before it gets rotated.
     * @param maxFiles      The maximum number of rotated log files to keep.
     * @param rotateOnOpen  If true, the log file will be rotated when opened.
     * @param openPlugin    Handle of the plugin that contains the file open function.
     *                      If null, defaults to the calling plugin.
     * @param openFunc      Function to call before the sink open the file.
     * @param closePlugin   Handle of the plugin that contains the file close function.
     *                      If null, defaults to the calling plugin.
     * @param closeFunc     Function to call after the sink close the file.
     * @return              A new RotatingFileSink Handle.
     * @error               Param maxFile < 0, Param maxFiles > 200000.
     */
    public native RotatingFileSink(const char[] baseFilename,
                                   const int maxSize,
                                   const int maxFiles,
                                   bool rotateOnOpen=false,
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
     * Rotate log files on demand.
     *
     * @param error         Error message buffer in case of rotate failure.
     * @param maxlen        Maximum length of the error.
     * @return              True on success, false on error.
     */
    public native bool RotateNow(char[] error="", int maxlen=0);

    /**
     * Gets the maximum size of the log file in bytes before it gets rotated.
     *
     * @return              The maximum size of the log file.
     */
    public native int GetMaxSize();

    /**
     * Sets the maximum size of the log file in bytes before it gets rotated.
     *
     * @param maxSize       The maximum size of the log file.
     * @param error         Error message buffer in case of sets failure.
     * @param maxlen        Maximum length of the error.
     * @return              True on success, false on error.
     */
    public native bool SetMaxSize(const int maxSize, char[] error="", int maxlen=0);

    /**
     * Gets the maximum number of rotated log files to keep.
     *
     * @return              The maximum number of rotated log files.
     */
    public native int GetMaxFiles();

    /**
     * Sets the maximum number of rotated log files to keep.
     *
     * @param maxFiles      The maximum number of rotated log files.
     * @param error         Error message buffer in case of sets failure.
     * @param maxlen        Maximum length of the error.
     * @return              True on success, false on error.
     */
    public native bool SetMaxFiles(const int maxFiles, char[] error="", int maxlen=0);

    /**
     * Returns whether or not a rotating file sink handle is valid.
     *
     * @return          True if valid, false otherwise.
     */
    public native bool IsValid();

    /**
     * Calc filename according to index and file extension if exists.
     * e.g. calcFilename("logs/mylog.txt, 3) => "logs/mylog.3.txt".
     *
     * @param buffer        Buffer to store file name.
     * @param maxlen        Maximum length of the buffer.
     * @param file          Original file path.
     * @param index         File index.
     * @return              Number of bytes written.
     */
    public static native int CalcFilename(char[] buffer, int maxlen, const char[] file, int index);

    /**
     * Returns the maximum number of rotating log files allowed.
     *
     * @return              Maximum allowed file count (200000).
     */
    public static int MaxFiles() {
        return 200000;
    }

#else

    /**
     * Rotating file sink based on size.
     *
     * @note RotatingFileSink handles must be freed via Sink.Close().
     * @note Multi-plugin shared Handles must be freed before unload the plugin.
     *
     * @param baseFilename  The file path where the log messages will be written.
     * @param maxSize       The maximum size of the log file in bytes before it gets rotated.
     * @param maxFiles      The maximum number of rotated log files to keep.
     * @param rotateOnOpen  If true, the log file will be rotated when opened.
     * @param openPlugin    Handle of the plugin that contains the file open function.
     *                      If null, defaults to the calling plugin.
     * @param openFunc      Function to call before the sink open the file.
     * @param closePlugin   Handle of the plugin that contains the file close function.
     *                      If null, defaults to the calling plugin.
     * @param closeFunc     Function to call after the sink close the file.
     * @return              A new RotatingFileSink Handle.
     * @error               Param maxFile < 0, Param maxFiles > 200000.
     */
    public RotatingFileSink(const char[] baseFilename,
                            const int maxSize,
                            const int maxFiles,
                            bool rotateOnOpen=false,
                            Handle openPlugin=null,  FileOpenFunction openFunc=INVALID_FUNCTION,
                            Handle closePlugin=null, FileCloseFunction closeFunc=INVALID_FUNCTION) {
        if (maxSize <= 0)
            ThrowError("RotatingFileSink constructor param maxFile cannot be less than or equal to zero.");
        if (maxFiles < 0)
            ThrowError("RotatingFileSink constructor param maxFiles cannot be less than zero.");
        if (maxFiles > RotatingFileSink.MaxFiles())
            ThrowError("RotatingFileSink constructor param maxFiles cannot exceed %d.", RotatingFileSink.MaxFiles());

        RotatingFileSink sink = view_as<RotatingFileSink>(Sink.__Sink("RotatingFileSink",
                                                                      null, __RotatingFileSink_OnLog,
                                                                      null, __RotatingFileSink_OnFlush,
                                                                      null, __RotatingFileSink_OnClose));
        sink.__SetBaseFilename(baseFilename);
        sink.__SetMaxSize(maxSize);
        sink.__SetMaxFiles(maxFiles);
        // sink.__SetCurrentSize();
        sink.__SetFileHelper(new FileHelper(openPlugin, openFunc, closePlugin, closeFunc));

        char filename[PLATFORM_MAX_PATH];
        RotatingFileSink.CalcFilename(filename, sizeof(filename), baseFilename, 0);

        char error[LOG4SP_HEADER_ONLY_MAX_ERR_LENGTH];
        if (!sink.__GetFileHelper().Open(filename, _, error, sizeof(error))) {
            sink.Close();
            ThrowError("%s", error);
        }

        int curSize = sink.__GetFileHelper().Size(error, sizeof(error));
        if (curSize == -1) {
            sink.Close();
            ThrowError("%s", error);
        }
        sink.__SetCurrentSize(curSize);

        if (rotateOnOpen && curSize > 0) {
            if (!sink.__Rotate(error, sizeof(error))) {
                sink.Close();
                ThrowError("%s", error);
            }
            sink.__SetCurrentSize(0);
        }
        return sink;
    }

    public int GetFilename(char[] buffer, int maxlen) {
        if (!this.IsValid())
            ThrowError("Invalid RotatingFileSink Handle %x.", this);
        return this.__GetFileHelper().GetFilename(buffer, maxlen);
    }

    public bool RotateNow(char[] error="", int maxlen=0) {
        if (!this.IsValid())
            ThrowError("Invalid RotatingFileSink Handle %x.", this);
        return this.__Rotate(error, maxlen);
    }

    public int GetMaxSize() {
        return this.__GetMaxSize();
    }

    public bool SetMaxSize(const int maxSize, char[] error="", int maxlen=0) {
        if (maxSize <= 0) {
            strcopy(error, maxlen, "Param maxSize cannot be less than or equal to 0.");
            return false;
        }
        this.__SetMaxSize(maxSize);
        return true;
    }

    public int GetMaxFiles() {
        return this.__GetMaxFiles();
    }

    public bool SetMaxFiles(const int maxFiles, char[] error="", int maxlen=0) {
        if (maxFiles <= 0) {
            strcopy(error, maxlen, "Param maxFiles cannot be less than or equal to 0.");
            return false;
        }
        if (maxFiles > RotatingFileSink.MaxFiles()) {
            FormatEx(error, maxlen, "Param maxFiles cannot exceed %d.", RotatingFileSink.MaxFiles());
            return false;
        }
        this.__SetMaxFiles(maxFiles);
        return true;
    }

    public bool IsValid() {
        if (!view_as<Sink>(this).IsValid())
            return false;

        if (!this.__GetFileHelper().IsValid())
            return false;

#if !defined LOG4SP_HEADER_ONLY_NO_VALID_HANDLE
        if (!this.__Contains("m_szBaseFilename") || !this.__Contains("m_uMaxSize")
         || !this.__Contains("m_uMaxFiles")      || !this.__Contains("m_uCurSize"))
            return false;
#endif      // LOG4SP_HEADER_ONLY_NO_VALID_HANDLE

        char classname[32];
        this.__GetClassname(classname, sizeof(classname));
        return StrEqual(classname, "RotatingFileSink");
    }

    public static int CalcFilename(char[] buffer, int maxlen, const char[] filename, int index) {
        if (index == 0)
            return strcopy(buffer, maxlen, filename);

        char path[PLATFORM_MAX_PATH], ext[PLATFORM_MAX_PATH];
        FileHelper.SplitByExtension(path, sizeof(path), ext, sizeof(ext), filename);
        return FormatEx(buffer, maxlen, "%s.%d%s", path, index, ext);
    }

    public static int MaxFiles() {
        return 200000;
    }


    // *** Do not use the function below this line! *** //
    public int __GetBaseFilename(char[] filename, int maxlen) { return this.__GetString("m_szBaseFilename", filename, maxlen); }
    public void __SetBaseFilename(const char[] filename)      { this.__SetString("m_szBaseFilename", filename); }

    public int __GetMaxSize()                           { return this.__GetValue("m_uMaxSize"); }
    public void __SetMaxSize(int maxSize)               { this.__SetValue("m_uMaxSize", maxSize); }

    public int __GetMaxFiles()                          { return this.__GetValue("m_uMaxFiles"); }
    public void __SetMaxFiles(int maxFiles)             { this.__SetValue("m_uMaxFiles", maxFiles); }

    public int __GetCurrentSize()                       { return this.__GetValue("m_uCurSize"); }
    public void __SetCurrentSize(int curSize)           { this.__SetValue("m_uCurSize", curSize); }

    // Ownership and lifecycle are the same as this, closed in OnClose.
    public FileHelper __GetFileHelper()                 { return this.__GetValue("m_hFileHelper"); }
    public void __SetFileHelper(FileHelper fileHelper)  { this.__SetValue("m_hFileHelper", fileHelper); }

    // Rotate files:
    // log.txt -> log.1.txt
    // log.1.txt -> log.2.txt
    // log.2.txt -> log.3.txt
    // log.3.txt -> delete
    public bool __Rotate(char[] error, int maxlen) {
        const int RENAME_INTERVAL = 100;
        FileHelper fileHelper = this.__GetFileHelper();
        fileHelper.CloseFile();

        for (int i = this.__GetMaxFiles(); i > 0; --i) {
            char baseFilename[PLATFORM_MAX_PATH], source[PLATFORM_MAX_PATH];
            this.__GetBaseFilename(baseFilename, sizeof(baseFilename));
            RotatingFileSink.CalcFilename(source, sizeof(source), baseFilename, i - 1);
            if (!FileExists(source))
                continue;

            char target[PLATFORM_MAX_PATH];
            RotatingFileSink.CalcFilename(target, sizeof(target), baseFilename, i);
            if (!this.__RenameFile(source, target)) {
                // if failed try again after a small delay.
                // this is a workaround to a windows issue, where very high rotation
                // rates can cause the rename to fail with permission denied (because of antivirus?).
                SafeSleepForMilliseconds(RENAME_INTERVAL);
                if (!this.__RenameFile(source, target)) {
                    // truncate the log file anyway to prevent it to grow beyond its limit!
                    if (!fileHelper.Reopen(true, error, maxlen))
                        return false;

                    this.__SetCurrentSize(0);
                    FormatEx(error, maxlen, "Failed renaming %s to %s.", source, target);
                    return false;
                }
            }
        }
        return fileHelper.Reopen(true, error, maxlen);
    }

    // delete the target if exists, and rename the src file to target
    // return true on success, false otherwise.
    public bool __RenameFile(const char[] source, const char[] target) {
        // try to delete the target file in case it already exists.
        DeleteFile(target);
        return RenameFile(target, source);
    }
#endif      // LOG4SP_HEADER_ONLY
}


#if defined LOG4SP_HEADER_ONLY
public bool __RotatingFileSink_OnLog(RotatingFileSink sink, char[] error, int maxlen, const char[] logTime, LOG4SP_ES_CONST SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
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
            else if (loc.filename[i] == '\\' || loc.filename[i] == '/')
                sepOffset = i + 1;
        }
        FormatEx(source, sizeof(source), "[%s::%d] ", loc.filename[sepOffset], loc.line);
    }

    char level[6];
    LogLevelToName(level, sizeof(level), lvl);

    char buffer[LOG4SP_HEADER_ONLY_MAX_MSG_LENGTH];
    len = FormatEx(buffer, sizeof(buffer), "[%s.%s] [%s] [%s] %s%s\n", date, millisecond, name, level, source, msg);
    buffer[len - 1] = '\n'; // buffer[len] of FormatEx must be '\0'.

    // rotate if the new estimated file size exceeds max size.
    // rotate only if the real size > 0 to better deal with full disk (see issue #2261).
    // we only check the real size when new_size > max_size_ because it is relatively expensive.
    FileHelper fileHelper = sink.__GetFileHelper();
    int newSize = sink.__GetCurrentSize() + len;
    if (newSize > sink.__GetMaxSize())
    {
        if (!fileHelper.Flush(error, maxlen))
            return false;

        int size = fileHelper.Size(error, maxlen);
        if (size == -1)
            return false;

        if (size > 0)
        {
            if (!sink.__Rotate(error, maxlen))
                return false;
            newSize = len;
        }
    }

    if (!fileHelper.Write(buffer, error, maxlen))
        return false;

    sink.__SetCurrentSize(newSize);
    return true;
}

public bool __RotatingFileSink_OnFlush(RotatingFileSink sink, char[] error, int maxlen)
{
    return sink.__GetFileHelper().Flush(error, maxlen);
}

public void __RotatingFileSink_OnClose(RotatingFileSink sink)
{
    sink.__GetFileHelper().Close();
    sink.__SetFileHelper(null);
}
#endif      // LOG4SP_HEADER_ONLY
