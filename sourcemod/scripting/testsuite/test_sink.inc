#if defined _log4sp_sinks_test_sink_included
 #endinput
#endif
#define _log4sp_sinks_test_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <log4sp/sinks/sink>


typeset TestDrainCallback
{
    function void (const char[] name, LogLevel lvl, const char[] msg);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime);
    function void (const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime, any data);
}

typeset TestDrainLineCallback
{
    function void (const char[] line);
    function void (const char[] line, any data);
}

enum struct sLogMessage
{
    char name[64];
    LogLevel lvl;
    char msg[2048];
    char file[PLATFORM_MAX_PATH];
    int line;
    char func[256];
    int logTime;
}


methodmap TestSink < Sink
{
    public native TestSink();

    public native int GetLogCount();
    public native int GetFlushCount();

    public native void DrainMsgs(TestDrainCallback callback, any data = 0);
    public ArrayList DrainMsgsFast() {
        ArrayList msgs = new ArrayList(.blocksize=ByteCountToCells(sizeof(sLogMessage)));
        this.DrainMsgs(__CB_DrainMsgs, msgs);
        if (msgs.Length == 0)
            ThrowError("TestSink msgs is empty!");

        return msgs;
    }

    public native void DrainLastMsg(TestDrainCallback callback, any data = 0);
    public sLogMessage DrainLastMsgFast() {
        ArrayList msg = new ArrayList(.blocksize=ByteCountToCells(sizeof(sLogMessage)));
        this.DrainLastMsg(__CB_DrainMsgs, msg);
        if (msg.Length == 0)
            ThrowError("TestSink msg is empty!");

        sLogMessage logMsg;
        msg.GetArray(0, logMsg);
        delete msg;
        return logMsg;
    }

    public native void DrainLines(TestDrainLineCallback callback, any data = 0);
    public ArrayList DrainLinesFast() {
        ArrayList lines = new ArrayList(.blocksize=ByteCountToCells(2048));
        this.DrainLines(__CB_DrainLines, lines);
        if (lines.Length == 0)
            ThrowError("TestSink lines is empty!");

        return lines;
    }

    public native void DrainLastLine(TestDrainLineCallback callback, any data = 0);
    public char[] DrainLastLineFast() {
        ArrayList line = new ArrayList(.blocksize=ByteCountToCells(2048));
        this.DrainLastLine(__CB_DrainLines, line);
        if (line.Length == 0)
            ThrowError("TestSink line is empty!");

        char lineString[2048];
        line.GetString(0, lineString, sizeof(lineString));
        delete line;
        return lineString;
    }

    public native void SetLogDelay(int milliseconds);
    public native void SetFlushDelay(int milliseconds);

    public native void SetLogException(const char[] msg);
    public native void ClearLogException();

    public native void SetFlushException(const char[] msg);
    public native void ClearFlushException();
}


static void __CB_DrainMsgs(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime, ArrayList msgs)
{
    sLogMessage logMsg;
    strcopy(logMsg.name, sizeof(sLogMessage::name), name);
    logMsg.lvl = lvl;
    strcopy(logMsg.msg, sizeof(sLogMessage::msg), msg);
    strcopy(logMsg.file, sizeof(sLogMessage::file), file);
    logMsg.line = line;
    strcopy(logMsg.func, sizeof(sLogMessage::func), func);
    logMsg.logTime = logTime;

    msgs.PushArray(logMsg);
}

static void __CB_DrainLines(const char[] line, ArrayList lines)
{
    lines.PushString(line);
}
