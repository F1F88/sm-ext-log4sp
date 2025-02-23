#if defined _log_for_sourcepawn_sinks_test_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_sinks_test_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <sourcemod>
#include <log4sp/logger>
#include <log4sp/sinks/callback_sink>

#include "test_utils"


static int __logCounter = 0;
static int __logPostCounter = 0;
static int __flushCounter = 0;
static char __lastLogMsg[TEST_MAX_MSG_LENGTH] = {'\0', ...};
static char __lastLogLine[TEST_MAX_MSG_LENGTH] = {'\0', ...};


methodmap TestSink < CallbackSink {
    public TestSink() {
        if (__logCounter || __logPostCounter || __flushCounter || __lastLogMsg[0] || __lastLogLine[0]) {
            AssertTrue("TestSink.Close() may be missing.", false);
        }

        return view_as<TestSink>(new CallbackSink(__TestSink_OnLog, __TestSink_OnLogPost, __TestSink_OnFlush));
    }

    public int GetLogCount() {
        return __logCounter;
    }

    public int GetLogPostCount() {
        return __logPostCounter;
    }

    public int GeFlushCount() {
        return __flushCounter;
    }

    public char[] GetLastLogMsg() {
        return __lastLogMsg;
    }

    public char[] GetLastLogLine() {
        return __lastLogLine;
    }

    public void Close() {
        delete this;
        __logCounter = 0;
        __logPostCounter = 0;
        __flushCounter = 0;
        __lastLogMsg[0] = '\0';
        __lastLogLine[0] = '\0';
    }
}


static void __TestSink_OnLog(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime) {
    __logCounter++;
    strcopy(__lastLogMsg, sizeof(__lastLogMsg), msg);
}

static void __TestSink_OnLogPost(const char[] msg) {
    __logPostCounter++;
    strcopy(__lastLogLine, sizeof(__lastLogLine), msg);
}

static void __TestSink_OnFlush() {
    __flushCounter++;
}
