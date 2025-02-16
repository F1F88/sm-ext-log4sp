#if defined _log_for_sourcepawn_test_sinks_test_sink_included
 #endinput
#endif
#define _log_for_sourcepawn_test_sinks_test_sink_included

#pragma newdecls required
#pragma semicolon 1

#include <adt_array>
#include <testing>
#include <log4sp/logger>
#include <log4sp/sinks/callback_sink>

#include "test_utils"


static TestSink __sink = null;

static int __logCounter = 0;
static int __logPostCounter = 0;
static int __flushCounter = 0;
static char __throw_error[2048] = {'\0', ...};
static ArrayList __logMsgs = null;
static ArrayList __logLines = null;


methodmap TestSink < CallbackSink {
    public int GetLogCount() {
        return __logCounter;
    }

    public int GetLogPostCount() {
        return __logPostCounter;
    }

    public int GeFlushCount() {
        return __flushCounter;
    }

    public void ThrowOnNext(const char[] msg) {
        strcopy(__throw_error, sizeof(__throw_error), msg);
    }

    public void ClearThrow() {
        __throw_error[0] = '\0';
    }

    public ArrayList GetMsgs() {
        return __logMsgs;
    }

    public ArrayList GetLines() {
        return __logLines;
    }

    public static TestSink Initialize() {
        __logMsgs = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
        __logLines = new ArrayList(ByteCountToCells(TEST_MAX_MSG_LENGTH));
        __sink = view_as<TestSink>(new CallbackSink(__TestSink_OnLog, __TestSink_OnLogPost, __TestSink_OnFlush));
        return __sink;
    }

    public static void Destroy() {
        if (__sink) {
            delete __sink;
        }

        __logCounter = 0;
        __logPostCounter = 0;
        __flushCounter = 0;

        for (int i = 0; i < sizeof(__throw_error); ++i) {
            __throw_error[i] = '\0';
        }

        if (__logMsgs) {
            delete __logMsgs;
        }

        if (__logLines) {
            delete __logLines;
        }
    }

    public static void AssertMsgs(const char[] text, ArrayList msgs, ArrayList expected) {
        if (msgs.Length != expected.Length) {
            AssertEq("msgs.Length", msgs.Length, expected.Length);
        }

        char lBuffer[100][TEST_MAX_MSG_LENGTH];
        char eBuffer[100][TEST_MAX_MSG_LENGTH];

        for (int i = 0; i < msgs.Length; ++i) {
            msgs.GetString(i, lBuffer[i], sizeof(lBuffer[]));
            expected.GetString(i, eBuffer[i], sizeof(eBuffer[]));
        }

        AssertStrArrayEq(text, lBuffer, eBuffer, msgs.Length);
    }

    // public static void AssertMsgsRegex(const char[] text, ArrayList msgs, ArrayList expected) {
    //     if (msgs.Length != expected.Length) {
    //         AssertEq("msgs.Length", msgs.Length, expected.Length);
    //     }

    //     char buffer[256];
    //     char lBuffer[TEST_MAX_MSG_LENGTH];
    //     char eBuffer[TEST_MAX_MSG_LENGTH];

    //     for (int i = 0; i < msgs.Length; ++i) {
    //         msgs.GetString(i, lBuffer, sizeof(lBuffer));
    //         expected.GetString(i, eBuffer, sizeof(eBuffer));

    //         if (StrRegexEq(lBuffer, eBuffer) <= 0)
    //         {
    //             FormatEx(buffer, sizeof(buffer), "%s [%d]", text, i);
    //             AssertTrue(buffer, false);
    //         }
    //     }

    //     AssertTrue(text, true);
    // }

    public static void AssertLines(const char[] text, ArrayList lines, ArrayList expected) {
        if (lines.Length != expected.Length) {
            AssertEq("lines.Length", lines.Length, expected.Length);
        }

        char lBuffer[100][TEST_MAX_MSG_LENGTH];
        char eBuffer[100][TEST_MAX_MSG_LENGTH];

        for (int i = 0; i < lines.Length; ++i) {
            lines.GetString(i, lBuffer[i], sizeof(lBuffer[]));
            expected.GetString(i, eBuffer[i], sizeof(eBuffer[]));
        }

        AssertStrArrayEq(text, lBuffer, eBuffer, lines.Length);
    }

    public static void AssertLinesRegex(const char[] text, ArrayList lines, ArrayList expected) {
        if (lines.Length != expected.Length) {
            AssertEq("lines.Length", lines.Length, expected.Length);
        }

        char buffer[256];
        char lBuffer[TEST_MAX_MSG_LENGTH];
        char eBuffer[TEST_MAX_MSG_LENGTH];

        for (int i = 0; i < lines.Length; ++i) {
            lines.GetString(i, lBuffer, sizeof(lBuffer));
            expected.GetString(i, eBuffer, sizeof(eBuffer));

            if (StrRegexEq(lBuffer, eBuffer) <= 0)
            {
                FormatEx(buffer, sizeof(buffer), "%s [%d]", text, i);
                AssertTrue(buffer, false);
            }
        }

        AssertTrue(text, true);
    }
}


static void __TestSink_OnLog(const char[] name, LogLevel lvl, const char[] msg, const char[] file, int line, const char[] func, int logTime) {
    if (__throw_error[0]) {
        ThrowError(__throw_error);
    }

    // Logger.Get(LOG4SP_GLOBAL_LOGGER_NAME).Debug(msg);

    __logMsgs.PushString(msg);
    __logCounter++;
}

static void __TestSink_OnLogPost(const char[] msg) {
    __logLines.PushString(msg);
    __logPostCounter++;
}

static void __TestSink_OnFlush() {
    if (__throw_error[0]) {
        ThrowError(__throw_error);
    }

    __flushCounter++;
}
