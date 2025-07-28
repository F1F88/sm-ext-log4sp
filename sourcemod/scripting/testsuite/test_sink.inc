#if defined _log4sp_sinks_test_sink_included
 #endinput
#endif
#define _log4sp_sinks_test_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <adt_array>
#include <adt_trie>
#include <profiler>
#include <log4sp/sinks/callback_sink2>

#define TEST_SINK_DATA_KEY_LOG_COUNT        "logCnt"
#define TEST_SINK_DATA_KEY_LOG_DELAY        "logDelay"
#define TEST_SINK_DATA_KEY_FLUSH_COUNT      "flushCnt"
#define TEST_SINK_DATA_KEY_FLUSH_DELAY      "flushDelay"
#define TEST_SINK_DATA_KEY_LOG_EXCEPTION    "logException"
#define TEST_SINK_DATA_KEY_FLUSH_EXCEPTION  "flushException"
#define TEST_SINK_DATA_KEY_LOG_MESSAGES     "logMessages"
#define TEST_SINK_DATA_KEY_LOG_LINES        "logLines"
#define TEST_SINK_DATA_KEY_SINK             "sink"
#define TEST_SINK_LOG_LINE_LEN              2048


enum struct sLogMessage
{
    char name[64];
    LogLevel lvl;
    char msg[2048];
    char file[PLATFORM_MAX_PATH];
    int line;
    char func[256];
    int logTime;

    void Clone(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime) {
        strcopy(this.name, sizeof(sLogMessage::name), name);
        this.lvl = lvl;
        strcopy(this.msg, sizeof(sLogMessage::msg), msg);
        strcopy(this.file, sizeof(sLogMessage::file), file);
        this.line = line;
        strcopy(this.func, sizeof(sLogMessage::func), func);
        this.logTime = logTime;
    }
}


methodmap TestSink < Sink
{
    public TestSink() {
        CallbackSink2 sink = new CallbackSink2(__Inner_OnLogCallback,
                                               __Inner_OnLogPostCallback,
                                               __Inner_OnFlushCallback,
                                               __Inner_OnDestoryCallback,
                                               new InnerTestSinkDataHelper());
        return view_as<TestSink>(sink);
    }

    public int GetLogCount() {
        return this.Data.GetLogCount();
    }

    public int GetFlushCount() {
        return this.Data.GetFlushCount();
    }

    public ArrayList DrainMsgs() {
        ArrayList logMessages = this.Data.GetLogMessages();
        if (logMessages.Length == 0)
            AssertTrue("TestSink::LogMessages is empty", false);

        ArrayList copy = logMessages.Clone();
        logMessages.Clear();
        return copy;
    }

    public sLogMessage DrainLastMsg() {
        ArrayList logMessages = this.Data.GetLogMessages();
        if (logMessages.Length == 0)
            AssertTrue("TestSink::LogMessages is empty", false);

        int index = logMessages.Length - 1;
        sLogMessage logMsg;
        logMessages.GetArray(index, logMsg);
        logMessages.Erase(index);
        return logMsg;
    }

    public ArrayList DrainLines() {
        ArrayList lines = this.Data.GetLogLines();
        if (lines.Length == 0)
            AssertTrue("TestSink::LogLines is empty", false);

        ArrayList copy = lines.Clone();
        lines.Clear();
        return copy;
    }

    public char[] DrainLastLine() {
        ArrayList lines = this.Data.GetLogLines();
        if (lines.Length == 0)
            AssertTrue("TestSink::LogLines is empty", false);

        char line[TEST_SINK_LOG_LINE_LEN];
        lines.GetString(lines.Length - 1, line, sizeof(line));
        lines.Erase(lines.Length - 1);
        return line;
    }

    public void SetLogDelay(int milliseconds) {
        this.Data.SetLogDelay(milliseconds);
    }

    public void SetFlushDelay(int milliseconds) {
        this.Data.SetFlushDelay(milliseconds);
    }

    public void SetLogException(const char[] description) {
        this.Data.SetLogException(description);
    }

    public void ClearLogException() {
        this.Data.ClearLogException();
    }

    public void SetFlushException(const char[] description) {
        this.Data.SetFlushException(description);
    }

    public void ClearFlushException() {
        this.Data.ClearFlushException();
    }

    property InnerTestSinkDataHelper Data {
        public get() {
            return view_as<InnerTestSinkDataHelper>(view_as<CallbackSink2>(this).GetData());
        }
    }
}


methodmap InnerTestSinkDataHelper < Handle
{
    public InnerTestSinkDataHelper() {
        StringMap data = new StringMap();
        data.SetValue(TEST_SINK_DATA_KEY_LOG_COUNT,         0);
        data.SetValue(TEST_SINK_DATA_KEY_FLUSH_COUNT,       0);
        data.SetValue(TEST_SINK_DATA_KEY_LOG_DELAY,         0);
        data.SetValue(TEST_SINK_DATA_KEY_FLUSH_DELAY,       0);
        // data.SetString(TEST_SINK_DATA_KEY_LOG_EXCEPTION,    "");
        // data.SetString(TEST_SINK_DATA_KEY_FLUSH_EXCEPTION,  "");
        data.SetValue(TEST_SINK_DATA_KEY_LOG_MESSAGES,      new ArrayList(ByteCountToCells(sizeof(sLogMessage))));
        data.SetValue(TEST_SINK_DATA_KEY_LOG_LINES,         new ArrayList(ByteCountToCells(TEST_SINK_LOG_LINE_LEN)));
        return view_as<InnerTestSinkDataHelper>(data);
    }

    public int GetLogCount() {
        return __Inner_StringMapGetValueEx(this.Data, TEST_SINK_DATA_KEY_LOG_COUNT);
    }

    public void IncreaseLogCount() {
        this.Data.SetValue(TEST_SINK_DATA_KEY_LOG_COUNT, (this.GetLogCount() + 1));
    }

    public int GetFlushCount() {
        return __Inner_StringMapGetValueEx(this.Data, TEST_SINK_DATA_KEY_FLUSH_COUNT);
    }

    public void IncreaseFlushCount() {
        this.Data.SetValue(TEST_SINK_DATA_KEY_FLUSH_COUNT, (this.GetFlushCount() + 1));
    }

    public int GetLogDelay() {
        return __Inner_StringMapGetValueEx(this.Data, TEST_SINK_DATA_KEY_LOG_DELAY);
    }

    public void SetLogDelay(int value) {
        this.Data.SetValue(TEST_SINK_DATA_KEY_LOG_DELAY, value);
    }

    public int GetFlushDelay() {
        return __Inner_StringMapGetValueEx(this.Data, TEST_SINK_DATA_KEY_FLUSH_DELAY);
    }

    public void SetFlushDelay(int value) {
        this.Data.SetValue(TEST_SINK_DATA_KEY_FLUSH_DELAY, value);
    }

    public void SetLogException(const char[] description) {
        this.Data.SetString(TEST_SINK_DATA_KEY_LOG_EXCEPTION, description);
    }

    public void ClearLogException() {
        this.Data.Remove(TEST_SINK_DATA_KEY_LOG_EXCEPTION);
    }

    public bool ShouldLogException() {
        return this.Data.ContainsKey(TEST_SINK_DATA_KEY_LOG_EXCEPTION);
    }

    public char[] GetLogExceptionDescription() {
        char description[256];
        this.Data.GetString(TEST_SINK_DATA_KEY_LOG_EXCEPTION, description, sizeof(description));
        return description;
    }

    public void SetFlushException(const char[] description) {
        this.Data.SetString(TEST_SINK_DATA_KEY_FLUSH_EXCEPTION, description);
    }

    public void ClearFlushException() {
        this.Data.Remove(TEST_SINK_DATA_KEY_FLUSH_EXCEPTION);
    }

    public bool ShouldFlushException() {
        return this.Data.ContainsKey(TEST_SINK_DATA_KEY_FLUSH_EXCEPTION);
    }

    public char[] GetFlushExceptionDescription() {
        char description[256];
        this.Data.GetString(TEST_SINK_DATA_KEY_FLUSH_EXCEPTION, description, sizeof(description));
        return description;
    }

    public ArrayList/*<sLogMessage>*/ GetLogMessages() {
        return __Inner_StringMapGetValueEx(this.Data, TEST_SINK_DATA_KEY_LOG_MESSAGES);
    }

    public void AddLogMessage(sLogMessage logMessage) {
        this.GetLogMessages().PushArray(logMessage);
    }

    public ArrayList/*<char[2048]>*/ GetLogLines() {
        return __Inner_StringMapGetValueEx(this.Data, TEST_SINK_DATA_KEY_LOG_LINES);
    }

    public void AddLogLine(char[] line) {
        this.GetLogLines().PushString(line);
    }

    public void Close() {
        delete this.GetLogMessages();
        delete this.GetLogLines();
        delete this.Data;
    }

    property StringMap Data {
        public get() {
            return view_as<StringMap>(this);
        }
    }
}


static bool __Inner_OnLogCallback(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime, InnerTestSinkDataHelper data, char error[256])
{
    // 模拟延迟
    __Inner_Sleep(data.GetLogDelay());

    // 模拟异常
    if (data.ShouldLogException())
    {
        strcopy(error, sizeof(error), data.GetLogExceptionDescription());
        return true;
    }

    // Update LogCounter
    data.IncreaseLogCount();

    // Add LogMessage
    sLogMessage logMessage;
    logMessage.Clone(name, lvl, msg, file, line, func, logTime);
    data.AddLogMessage(logMessage);

    return false;
}

static void __Inner_OnLogPostCallback(const char[] line, InnerTestSinkDataHelper data)
{
    // Add Line
    char buffer[TEST_SINK_LOG_LINE_LEN];
    int len = strcopy(buffer, sizeof(buffer), line);

    if (buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';
    if (buffer[len - 2] == '\r')
        buffer[len - 2] = '\0';

    data.AddLogLine(buffer);
}

static bool __Inner_OnFlushCallback(InnerTestSinkDataHelper data, char error[256])
{
    // 模拟延迟
    __Inner_Sleep(data.GetFlushDelay());

    // 模拟异常
    if (data.ShouldFlushException())
    {
        strcopy(error, sizeof(error), data.GetFlushExceptionDescription());
        return true;
    }

    // Update FlushCounter
    data.IncreaseFlushCount();

    return false;
}

static void __Inner_OnDestoryCallback(InnerTestSinkDataHelper data)
{
    data.Close();
}


stock any __Inner_StringMapGetValueEx(StringMap map, const char[] key)
{
    any value;
    map.GetValue(key, value);
    return value;
}

stock void __Inner_Sleep(int milliseconds)
{
    int endTickCount = GetSysTickCount() + milliseconds;
    // PrintToServer("Start | sys tick = %d | time %d | engine time = %f |", GetSysTickCount(), GetTime(), GetEngineTime());
    while (GetSysTickCount() < endTickCount) {}
    // PrintToServer("End   | sys tick = %d | time %d | engine time = %f |", GetSysTickCount(), GetTime(), GetEngineTime());
}


#undef TEST_SINK_DATA_KEY_LOG_COUNT
#undef TEST_SINK_DATA_KEY_LOG_DELAY
#undef TEST_SINK_DATA_KEY_FLUSH_COUNT
#undef TEST_SINK_DATA_KEY_FLUSH_DELAY
#undef TEST_SINK_DATA_KEY_LOG_EXCEPTION
#undef TEST_SINK_DATA_KEY_FLUSH_EXCEPTION
#undef TEST_SINK_DATA_KEY_LOG_MESSAGES
#undef TEST_SINK_DATA_KEY_LOG_LINES
#undef TEST_SINK_DATA_KEY_SINK
#undef TEST_SINK_LOG_LINE_LEN
