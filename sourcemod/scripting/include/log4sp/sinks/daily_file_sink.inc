#if defined _log4sp_sinks_daily_file_sink_included
    #endinput
#endif
#define _log4sp_sinks_daily_file_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>

#if defined LOG4SP_HEADER_ONLY
#include <log4sp/details/file_helper>
#include <log4sp/details/long>
#endif      // LOG4SP_HEADER_ONLY


/**
 * Callback function called when calculating the daily log file name.
 *
 * @param filename  Daily log file name.
 * @param maxlen    Maximum length of the filename.
 * @param timestamp Timestamp in seconds.
 */
typedef DailyFileCalculator = function void (char[] filename, int maxlen, const char[] timestamp);


/**
 * Generator of daily log file names with strftime format.
 */
public void DailyFileFormatCalculator(char[] filename, int maxlen, const char[] timestamp) {
    // FormatTime buffer cannot be the same as the format
    char buffer[256];
    strcopy(buffer, sizeof(buffer), filename);
    FormatTime(filename, maxlen, buffer, StringToInt(timestamp)); // FIXME: Possible Year 2038 Problem
}


/**
 * Creates new log files daily at a specified time, helping organize logs by date
 * for the easier tracking and to the analysis sink files.
 *
 * @note Note that old log files from previous executions will not be deleted by
 *       this class, rotation and deletion is only applied while the program is
 *       running.
 */
methodmap DailyFileSink < Sink
{
#if !defined LOG4SP_HEADER_ONLY
    /**
     * Create daily file sink which rotates on given time.
     *
     * @note Note that old log files from previous executions will not be deleted by this class,
     *       rotation and deletion is only applied while the program is running.
     * @note DailyFileSink handles must be freed via delete or CloseHandle().
     *
     * @param baseFilename  The file path where the log messages will be written.
     * @param hour          The hour of the day when the log file should be rotated. (0-23)
     * @param minute        The minute of the hour when the log file should be rotated. (0-59)
     * @param truncate      If true, the created file will be truncated.
     * @param maxFiles      If max_files > 0, retain only the last max_files and delete previous.
     * @param calcPlugin    Handle of the plugin that contains the calculator function.
     *                      If null, defaults to the calling plugin.
     * @param calcFunc      Function to call when calculating the daily log file name.
     * @param openPlugin    Handle of the plugin that contains the file open function.
     *                      If null, defaults to the calling plugin.
     * @param openFunc      Function to call before the sink open the file.
     * @param closePlugin   Handle of the plugin that contains the file close function.
     *                      If null, defaults to the calling plugin.
     * @param closeFunc     Function to call after the sink close the file.
     * @return              A new DailyFileSink Handle.
     * @error               Invalid rotation time in ctor, or maxFiles < 0, or maxFiles > 65535.
     */
    public native DailyFileSink(const char[] baseFilename,
                                int hour=0,
                                int minute=0,
                                bool truncate=false,
                                int maxFiles=0,
                                Handle calcPlugin=null,  DailyFileCalculator calcFunc=INVALID_FUNCTION,
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
     * Returns whether or not a daily file sink handle is valid.
     *
     * @return          True if valid, false otherwise.
     */
    public native bool IsValid();

#else

    /**
     * Create daily file sink which rotates on given time.
     *
     * @note Note that old log files from previous executions will not be deleted by this class,
     *       rotation and deletion is only applied while the program is running.
     * @note DailyFileSink handles must be freed via Sink.Close().
     * @note Multi-plugin shared Handles must be freed before unload the plugin.
     *
     * @param baseFilename  The file path where the log messages will be written.
     * @param hour          The hour of the day when the log file should be rotated. (0-23)
     * @param minute        The minute of the hour when the log file should be rotated. (0-59)
     * @param truncate      If true, the created file will be truncated.
     * @param maxFiles      If max_files > 0, retain only the last max_files and delete previous.
     * @param calcPlugin    Handle of the plugin that contains the calculator function.
     *                      If null, defaults to the calling plugin.
     * @param calcFunc      Function to call when calculating the daily log file name.
     * @param openPlugin    Handle of the plugin that contains the file open function.
     *                      If null, defaults to the calling plugin.
     * @param openFunc      Function to call before the sink open the file.
     * @param closePlugin   Handle of the plugin that contains the file close function.
     *                      If null, defaults to the calling plugin.
     * @param closeFunc     Function to call after the sink close the file.
     * @return              A new DailyFileSink Handle.
     * @error               Invalid rotation time in ctor, or maxFiles < 0, or maxFiles > 65535.
     */
    public DailyFileSink(const char[] baseFilename,
                         int hour=0,
                         int minute=0,
                         bool truncate=false,
                         int maxFiles=0,
                         Handle calcPlugin=null,  DailyFileCalculator calcFunc=INVALID_FUNCTION,
                         Handle openPlugin=null,  FileOpenFunction openFunc=INVALID_FUNCTION,
                         Handle closePlugin=null, FileCloseFunction closeFunc=INVALID_FUNCTION) {
        const int UINT16_MAX = 0xFFFF;
        if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
            ThrowError("Invalid rotation time in ctor.");
        if (maxFiles < 0 || maxFiles > UINT16_MAX)
            ThrowError("Invalid maxFiles %u. [0-%u]", maxFiles, UINT16_MAX);

        PrivateForward calcFwd;
        if (calcFunc && calcFunc != INVALID_FUNCTION) {
            // void (char[] filename, int maxlen, const char[] timestamp)
            calcFwd = new PrivateForward(ET_Ignore, Param_String, Param_Cell, Param_String);
            calcFwd.AddFunction(calcPlugin, calcFunc);
        }

        DailyFileSink sink = view_as<DailyFileSink>(Sink.__Sink("DailyFileSink",
                                                                null, __DailyFileSink_OnLog,
                                                                null, __DailyFileSink_OnFlush,
                                                                null, __DailyFileSink_OnClose));
        sink.__SetBaseFilename(baseFilename);
        sink.__SetHour(hour);
        sink.__SetMinute(minute);
        sink.__SetRotateTime(sink.__NextRotationTimestamp());
        sink.__SetFileHelper(new FileHelper(openPlugin, openFunc, closePlugin, closeFunc));
        sink.__SetTruncate(truncate);
        sink.__SetMaxFiles(maxFiles);
        sink.__SetFilenames(null);
        sink.__SetCalcFwd(calcFwd);

        int time64[2];
        GetTime(time64);

        char timeStr[21];
        Int64ToString(time64, timeStr, sizeof(timeStr));

        char filename[PLATFORM_MAX_PATH];
        sink.__CalcFilename(filename, sizeof(filename), timeStr);

        char error[LOG4SP_HEADER_ONLY_MAX_ERR_LENGTH];
        if (!sink.__GetFileHelper().Open(filename, truncate, error, sizeof(error))) {
            sink.Close();
            ThrowError("%s", error);
        }

        if (maxFiles > 0) {
            sink.__InitFilenamesQueue();
        }
        return sink;
    }

    public int GetFilename(char[] buffer, int maxlen) {
        if (!this.IsValid())
            ThrowError("Invalid DailyFileSink Handle %x.", this);
        return this.__GetFileHelper().GetFilename(buffer, maxlen);
    }

    public bool IsValid() {
        if (!view_as<Sink>(this).IsValid())
            return false;

        if (!this.__GetFileHelper().IsValid())
            return false;

#if !defined LOG4SP_HEADER_ONLY_NO_VALID_HANDLE
        if (!this.__Contains("m_szBaseFilename") || !this.__Contains("m_iHour")
         || !this.__Contains("m_iMinute")        || !this.__Contains("m_lRotateTime")
         || !this.__Contains("m_bTruncate")      || !this.__Contains("m_uMaxFiles")
         || !this.__Contains("m_hFilenames")     || !this.__Contains("m_hCalcFwd"))
            return false;
#endif      // LOG4SP_HEADER_ONLY_NO_VALID_HANDLE

        char classname[16];
        this.__GetClassname(classname, sizeof(classname));
        return StrEqual(classname, "DailyFileSink");
    }


    // *** Do not use the function below this line! *** //

    public int __GetBaseFilename(char[] baseFilename, int maxlen) { return this.__GetString("m_szBaseFilename", baseFilename, maxlen); }
    public void __SetBaseFilename(const char[] baseFilename)      { this.__SetString("m_szBaseFilename", baseFilename); }

    public int __GetHour()                              { return this.__GetValue("m_iHour"); }
    public void __SetHour(int hour)                     { this.__SetValue("m_iHour", hour); }

    public int __GetMinute()                            { return this.__GetValue("m_iMinute"); }
    public void __SetMinute(int minute)                 { this.__SetValue("m_iMinute", minute); }

    // Returns a timestamp in seconds.
    public Long __GetRotateTime() {
        Long value;
        view_as<StringMap>(this).GetArray("m_lRotateTime", value, sizeof(Long));
        return value;
    }
    public void __SetRotateTime(Long rotateTime)        { view_as<StringMap>(this).SetArray("m_lRotateTime", rotateTime, sizeof(Long)); }

    // Ownership and lifecycle are the same as this, closed in OnClose.
    public FileHelper __GetFileHelper()                 { return this.__GetValue("m_hFileHelper"); }
    public void __SetFileHelper(FileHelper fileHelper)  { this.__SetValue("m_hFileHelper", fileHelper); }

    public bool __GetTruncate()                         { return this.__GetValue("m_bTruncate"); }
    public void __SetTruncate(bool truncate)            { this.__SetValue("m_bTruncate", truncate); }

    public int __GetMaxFiles()                          { return this.__GetValue("m_uMaxFiles"); }
    public void __SetMaxFiles(int maxFiles)             { this.__SetValue("m_uMaxFiles", maxFiles); }

    // Ownership and lifecycle are the same as this, closed in OnClose.
    public ArrayList __GetFilenames()                   { return this.__GetValue("m_hFilenames"); }
    public void __SetFilenames(ArrayList filenames)     { this.__SetValue("m_hFilenames", filenames); }

    // Ownership and lifecycle are the same as this, closed in OnClose.
    public PrivateForward __GetCalcFwd()                { return this.__GetValue("m_hCalcFwd"); }
    public void __SetCalcFwd(PrivateForward calcFwd)    { this.__SetValue("m_hCalcFwd", calcFwd); }

    public void __InitFilenamesQueue() {
        int time64[2];
        GetTime(time64);

        Long now; // seconds
        now = Int64ToLong(time64);

        int maxFiles = this.__GetMaxFiles();
        char filename[PLATFORM_MAX_PATH];
        ArrayList temp = new ArrayList(ByteCountToCells(PLATFORM_MAX_PATH));
        while (temp.Length < maxFiles) {
            this.__CalcFilename(filename, sizeof(filename), now.ToStringFixed());
            if (!FileExists(filename))
                break;

            temp.PushString(filename);
            now = now.Sub(IntToLong(86400));
        }

        ArrayList filenames = new ArrayList(ByteCountToCells(PLATFORM_MAX_PATH));
        for (int i = temp.Length - 1; i >= 0; --i) {
            temp.GetString(i, filename, sizeof(filename));
            filenames.PushString(filename);
        }
        delete temp;
        this.__SetFilenames(filenames);
    }

    // Returns a timestamp in seconds.
    public Long __NextRotationTimestamp() {
        int time64[2];
        GetTime(time64);

        Long now, today, rotationTime;
        now = Int64ToLong(time64);

#if !defined SM_INT64_SUPPORTED
        // Mod operations were very slow before int64 was supported.
        char buffer[8];
        FormatTime(buffer, sizeof(buffer), "%H%M%S");
        int hour   = (buffer[0] - '0') * 10 + (buffer[1] - '0');
        int minute = (buffer[2] - '0') * 10 + (buffer[3] - '0');
        int second = (buffer[4] - '0') * 10 + (buffer[5] - '0');
        today = now.Sub(IntToLong(hour * 3600 + minute * 60 + second));
        today.ToStringFixed();
#else
        today = now.Sub(now.Mod(IntToLong(86400)));
#endif      // SM_INT64_SUPPORTED

        rotationTime = today.Add(IntToLong(this.__GetHour() * 3600 + this.__GetMinute() * 60));
        if (rotationTime.Gt(now))
            return rotationTime;
        return rotationTime.Add(IntToLong(86400));
    }

    // Delete the file N rotations ago.
    // Return error on failure to delete the old file.
    public bool __DeleteOld(char[] error, int maxlen) {
        char filename[PLATFORM_MAX_PATH];
        this.GetFilename(filename, sizeof(filename));

        ArrayList filenames = this.__GetFilenames();
        if (filenames.Length >= this.__GetMaxFiles()) {
            char oldFilename[PLATFORM_MAX_PATH];
            filenames.GetString(0, oldFilename, sizeof(oldFilename));
            filenames.Erase(0);

            if (FileExists(oldFilename)) {
                if (!DeleteFile(oldFilename)) {
                    filenames.PushString(filename);
                    FormatEx(error, maxlen, "Failed removing daily file %s.", oldFilename);
                    return false;
                }
            }
        }
        filenames.PushString(filename);
        return true;
    }

    // Unit of timestamp is seconds.
    public void __CalcFilename(char[] filename, int maxlen, const char[] timestamp) {
        this.__GetBaseFilename(filename, maxlen);
        PrivateForward calcFwd = this.__GetCalcFwd();
        if (calcFwd) {
            Call_StartForward(calcFwd);
            Call_PushStringEx(filename, maxlen, SM_PARAM_STRING_COPY|SM_PARAM_STRING_UTF8, SM_PARAM_COPYBACK); // filename
            Call_PushCell(maxlen);              // maxlen
            Call_PushString(timestamp);         // timestamp
            Call_Finish();
        } else {
            // Generator of daily log file names in format baseFilename_YYYYMMDD.ext
            char file[PLATFORM_MAX_PATH], ext[PLATFORM_MAX_PATH];
            FileHelper.SplitByExtension(file, sizeof(file), ext, sizeof(ext), filename);

            // https://github.com/alliedmodders/sourcemod/blob/e87291362bc8445b187bb2a574ad702e8f1a0696/core/logic/Logger.cpp#L327
            char buffer[PLATFORM_MAX_PATH];
            FormatEx(buffer, sizeof(buffer), "%s_%s%s", file, "%Y%m%d", ext);

            FormatTime(filename, maxlen, buffer, StringToInt(timestamp)); // FIXME: Possible Year 2038 Problem
        }
    }
#endif      // LOG4SP_HEADER_ONLY
}


#if defined LOG4SP_HEADER_ONLY
public bool __DailyFileSink_OnLog(DailyFileSink sink, char[] error, int maxlen, const char[] logTime, LOG4SP_ES_CONST SourceLoc loc, const char[] name, LogLevel lvl, const char[] msg)
{
    int len = strlen(logTime);
    char seconds[21], millisecond[4];
    strcopy(seconds, len - 8, logTime);
    strcopy(millisecond, sizeof(millisecond), logTime[len - 9]);

    char date[20]; // 2026-01-01 12:34:56
    FormatTime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", StringToInt(seconds)); // FIXME: Possible Year 2038 Problem
    // PrintToServer("__DailyFileSink_OnLog %s | %d | %s", seconds, StringToInt(seconds), date);

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

    bool shouldRotate = StringToLong(seconds).Ge(sink.__GetRotateTime());
    if (shouldRotate)
    {
        char filename[PLATFORM_MAX_PATH];
        sink.__CalcFilename(filename, sizeof(filename), seconds);

        FileHelper fileHelper = sink.__GetFileHelper();
        if (!fileHelper.Open(filename, sink.__GetTruncate(), error, maxlen))
            return false;

        sink.__SetRotateTime(sink.__NextRotationTimestamp());
    }

    if (!sink.__GetFileHelper().Write(buffer, error, maxlen))
        return false;

    // Do the cleaning only at the end because it might throw on failure.
    if (shouldRotate && sink.__GetMaxFiles() > 0)
    {
        if (!sink.__DeleteOld(error, maxlen))
            return false;
    }
    return true;
}

public bool __DailyFileSink_OnFlush(DailyFileSink sink, char[] error, int maxlen)
{
    return sink.__GetFileHelper().Flush(error, maxlen);
}

public void __DailyFileSink_OnClose(DailyFileSink sink)
{
    // It will be destroyed soon, so ignore resetting it to null.
    PrivateForward calculatorFwd = sink.__GetCalcFwd();
    if (calculatorFwd)
    {
        delete calculatorFwd;
        // sink.__SetCalcFwd(null);
    }

    sink.__GetFileHelper().Close();
    // sink.__SetFileHelper(null);

    ArrayList filenames = sink.__GetFilenames();
    if (filenames)
    {
        delete filenames;
        // sink.__SetFilenames(null);
    }
}
#endif      // LOG4SP_HEADER_ONLY
