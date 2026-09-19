#pragma semicolon 1
#pragma newdecls required

#undef REQUIRE_PLUGIN
#include <log4sp/registry>
#define REQUIRE_PLUGIN

#include <log4sp/sinks/server_console_sink>


static Handle           m_pThis;
static StringMap        m_hLoggers;                     // <nameString, Logger>
static StringMap        m_hLogLevels;                   // <nameString, LogLevel>
static DataPack         m_hPatternData;
static PatternTimeType  m_ePatternTimeType = PatternTimeType_Local;
static LogLevel         m_eGlobalLogLevel = LogLevel_Info;
static LogLevel         m_eGlobalFlushLevel = LogLevel_Off;
static Handle           m_hFlushTimer;
static Handle           m_hErrorPlugin;
static ErrorHandler     m_fnErrorFunc = INVALID_FUNCTION;
static Logger           m_hGlobalLogger;
static bool             m_bAutomaticRegistration = true;


void CreateRegistryNatives()
{
    //* 不要忘记添加新增的 Native 到 __pl_log4sp_manager_SetNTVOptional !!! *//
    CreateNative("Log4spRegistry.RegisterLogger",           Native_Registry_RegisterLogger);
    CreateNative("Log4spRegistry.InitializeLogger",         Native_Registry_InitializeLogger);
    CreateNative("Log4spRegistry.Get",                      Native_Registry_Get);
    CreateNative("Log4spRegistry.GetGlobal",                Native_Registry_GetGlobal);
    CreateNative("Log4spRegistry.GetGlobalRaw",             Native_Registry_GetGlobalRaw);
    CreateNative("Log4spRegistry.SetGlobalLogger",          Native_Registry_SetGlobalLogger);
    CreateNative("Log4spRegistry.SetLevel",                 Native_Registry_SetLevel);
    CreateNative("Log4spRegistry.SetPattern",               Native_Registry_SetPattern);
    CreateNative("Log4spRegistry.SetFlushLevel",            Native_Registry_SetFlushLevel);
    CreateNative("Log4spRegistry.SetFlushEvery",            Native_Registry_SetFlushEvery);
    CreateNative("Log4spRegistry.GetFlusher",               Native_Registry_GetFlusher);
    CreateNative("Log4spRegistry.SetErrorHandler",          Native_Registry_SetErrorHandler);
    CreateNative("Log4spRegistry.ApplyAll",                 Native_Registry_ApplyAll);
    CreateNative("Log4spRegistry.FlushAll",                 Native_Registry_FlushAll);
    CreateNative("Log4spRegistry.Drop",                     Native_Registry_Drop);
    CreateNative("Log4spRegistry.DropAll",                  Native_Registry_DropAll);
    CreateNative("Log4spRegistry.SetAutomaticRegistration", Native_Registry_SetAutomaticRegistration);
    CreateNative("Log4spRegistry.SetLevels",                Native_Registry_SetLevels);
    CreateNative("Log4spRegistry.ApplyLoggerEnvLevels",     Native_Registry_ApplyLoggerEnvLevels);
    CreateNative("Log4spRegistry.Instance",                 Native_Registry_Instance);
}

void InitializeRegistryMembers()
{
    char filename[PLATFORM_MAX_PATH];
    GetPluginFilename(INVALID_HANDLE, filename, sizeof(filename));

    m_pThis = FindPluginByFile(filename);
    m_hLoggers = new StringMap();
    m_hLogLevels = new StringMap();
    m_hPatternData = new DataPack();

    // Create global logger
    ServerConsoleSink sink = new ServerConsoleSink();
    m_hGlobalLogger = new Logger("");
    m_hGlobalLogger.AddSink(sink);
    SinkCleanupAndDelete(sink);

    // Register global logger
    Log4spRegistry.Instance().RegisterLogger(m_hGlobalLogger);
    m_hLoggers.SetValue("", m_hGlobalLogger.Clone());
}

// HACK: Remove when the Handle in Header Only mode can be completely cloned and closed
void RegistryHandleOwnershipPatch(Handle plugin)
{
#if !defined LOG4SP_HEADER_ONLY
    #pragma unused plugin
#else
    if (plugin == m_pThis)
        return;

    char filename[PLATFORM_MAX_PATH];
    GetPluginFilename(plugin, filename, sizeof(filename));

    StringMapSnapshot snapshot = m_hLoggers.Snapshot();
    for (int i = 0; i < snapshot.Length; ++i)
    {
        int keyLen = snapshot.KeyBufferSize(i);
        char[] key = new char[keyLen + 1];
        snapshot.GetKey(i, key, keyLen + 1);

        Logger logger;
        if (m_hLoggers.GetValue(key, logger))
        {
            char owner[PLATFORM_MAX_PATH];
            logger.__GetOwner(owner, sizeof(owner));

            if (StrEqual(filename, owner))
            {
                Log4spRegistry.Instance().Drop(key);
                LogMessage("[Patch] Due to the unload of plugin \"%s\", the registry drop the logger \"%s\".", filename, key);
            }
        }
    }
    delete snapshot;

    if (plugin == m_hErrorPlugin)
    {
        m_hErrorPlugin = null;
        m_fnErrorFunc = INVALID_FUNCTION;
    }
#endif
}


static any Native_Registry_RegisterLogger(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    Logger logger = GetNativeCell(2);
    if (!logger.IsValid())
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Logger Handle %x.", logger);

    // Get logger name
    int size = logger.GetNameLength() + 1;
    char[] name = new char[size];
    logger.GetName(name, size);

    // Throw if exists
    if (m_hLoggers.ContainsKey(name))
        ThrowNativeError(SP_ERROR_PARAM, "Logger with name \"%s\" already exists.", name);

    // Register and increment the reference count
    m_hLoggers.SetValue(name, logger.Clone());
    return 0;
}

/**
 * Initialize and register a logger,
 * formatter, log level and flush level will be set according the global settings.
 * Useful for initializing manually created loggers with the global settings.
 */
static any Native_Registry_InitializeLogger(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    Logger logger = GetNativeCell(2);
    if (!logger.IsValid())
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Logger Handle %x.", logger);

    m_hPatternData.Reset();
    if (m_hPatternData.IsReadable())
    {
        // Set new pattern
        int size = m_hPatternData.ReadCell() + 1;
        char[] pattern = new char[size];
        m_hPatternData.ReadString(pattern, size);
        logger.SetPattern(pattern, m_ePatternTimeType);
    }

    // Get logger name
    int size = logger.GetNameLength() + 1;
    char[] name = new char[size];
    logger.GetName(name, size);

    // Set new level according to previously configured level or default level
    LogLevel lvl;
    if (m_hLogLevels && m_hLogLevels.GetValue(name, lvl))
        logger.SetLevel(lvl);
    else
        logger.SetLevel(m_eGlobalLogLevel);

    // Set new flush level according to previously configured flush level
    logger.SetFlushLevel(m_eGlobalFlushLevel);

    // Set new error function according to previously configured error function
    if (m_fnErrorFunc != INVALID_FUNCTION)
        logger.SetErrorHandler(m_hErrorPlugin, m_fnErrorFunc);

    // Register if need
    if (m_bAutomaticRegistration)
        Log4spRegistry.Instance().RegisterLogger(logger);
    return 0;
}

static any Native_Registry_Get(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    int len;
    GetNativeStringLength(2, len);
    char[] name = new char[len + 1];
    GetNativeString(2, name, len + 1);

    Logger logger;
    return m_hLoggers.GetValue(name, logger) ? logger : null;
}

static any Native_Registry_GetGlobal(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    if (!m_hGlobalLogger)
        return INVALID_HANDLE;

    // Get logger name
    int size = m_hGlobalLogger.GetNameLength() + 1;
    char[] name = new char[size];
    m_hGlobalLogger.GetName(name, size);

    Logger logger;
    return m_hLoggers.GetValue(name, logger) ? logger : null;
}

// Return raw ptr to the global logger.
// To be used directly by the spdlog default api (e.g. spdlog::info)
// This make the default API faster, but cannot be used concurrently with set_default_logger().
// e.g do not call set_default_logger() from one thread while calling spdlog::info() from another.
static any Native_Registry_GetGlobalRaw(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    return m_hGlobalLogger;
}

// Set global logger.
// Global logger is stored in m_hGlobalLogger (for faster retrieval) and in the m_hLoggers map.
static any Native_Registry_SetGlobalLogger(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    Logger logger = GetNativeCell(2);
    if (!logger.IsValid())
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Logger Handle %x.", logger);

    // Close and Drop global logger
    if (m_hGlobalLogger)
    {
        int size = m_hGlobalLogger.GetNameLength() + 1;
        char[] name = new char[size];
        m_hGlobalLogger.GetName(name, size);

        Log4spRegistry.Instance().Drop(name);
        LoggerCleanupAndDelete(m_hGlobalLogger);
    }

    // Set to global
    m_hGlobalLogger = view_as<Logger>(logger.Clone());

    // Register new global logger
    Log4spRegistry.Instance().RegisterLogger(m_hGlobalLogger);
    return 0;
}

static any Native_Registry_SetLevel(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    LogLevel lvl = GetNativeCell(2);

    StringMapSnapshot snapshot = m_hLoggers.Snapshot();
    for (int i = 0; i < snapshot.Length; ++i)
    {
        int keyLen = snapshot.KeyBufferSize(i);
        char[] key = new char[keyLen + 1];
        snapshot.GetKey(i, key, keyLen + 1);

        Logger logger;
        if (m_hLoggers.GetValue(key, logger))
            logger.SetLevel(lvl);
    }
    delete snapshot;

    // Set to global
    m_eGlobalLogLevel = lvl;
    return 0;
}

/**
 * Set global format string.
 * Each sink in each logger will have a pattern set to this string.
 */
// Set global formatter. Each sink in each logger will get a clone of this object
static any Native_Registry_SetPattern(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    int len;
    GetNativeStringLength(2, len);
    char[] pattern = new char[len + 1];
    GetNativeString(2, pattern, len + 1);

    PatternTimeType type = GetNativeCell(3);

    // Save to global
    m_hPatternData.Reset(true);
    m_hPatternData.WriteCell(len);
    m_hPatternData.WriteString(pattern);
    m_ePatternTimeType = type;

    StringMapSnapshot snapshot = m_hLoggers.Snapshot();
    for (int i = 0; i < snapshot.Length; ++i)
    {
        int keyLen = snapshot.KeyBufferSize(i);
        char[] key = new char[keyLen + 1];
        snapshot.GetKey(i, key, keyLen + 1);

        Logger logger;
        if (m_hLoggers.GetValue(key, logger))
            logger.SetPattern(pattern, type);
    }
    delete snapshot;
    return 0;
}

static any Native_Registry_SetFlushLevel(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    LogLevel lvl = GetNativeCell(2);

    StringMapSnapshot snapshot = m_hLoggers.Snapshot();
    for (int i = 0; i < snapshot.Length; ++i)
    {
        int keyLen = snapshot.KeyBufferSize(i);
        char[] key = new char[keyLen + 1];
        snapshot.GetKey(i, key, keyLen + 1);

        Logger logger;
        if (m_hLoggers.GetValue(key, logger))
            logger.SetFlushLevel(lvl);
    }
    delete snapshot;

    // Set to global
    m_eGlobalFlushLevel = lvl;
    return 0;

}

static any Native_Registry_SetFlushEvery(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    float interval = GetNativeCell(2);

    // Kill if exists
    if (m_hFlushTimer)
    {
        KillTimer(m_hFlushTimer);
        m_hFlushTimer = null;
    }

    // Set to global
    m_hFlushTimer = CreateTimer(interval, Timer_FlushEvery, _, TIMER_REPEAT);
    return 0;
}

static any Native_Registry_GetFlusher(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    return m_hFlushTimer;
}

static any Native_Registry_SetErrorHandler(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    Handle errorPlugin = GetNativeCell(2);

    // HACK: Replace GetNativeFunction() with GetNativeCell() to suppress compiler warnings.
    // warning 237: coercing functions to and from primitives is unsupported and will be removed in the future
    // https://github.com/alliedmodders/sourcepawn/pull/644
    // https://github.com/alliedmodders/sourcepawn/issues/671
    ErrorHandler errorFunc = GetNativeCell(3);
    if (errorFunc == INVALID_FUNCTION)
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    StringMapSnapshot snapshot = m_hLoggers.Snapshot();
    for (int i = 0; i < snapshot.Length; ++i)
    {
        int keyLen = snapshot.KeyBufferSize(i);
        char[] key = new char[keyLen + 1];
        snapshot.GetKey(i, key, keyLen + 1);

        Logger logger;
        if (m_hLoggers.GetValue(key, logger))
            logger.SetErrorHandler(errorPlugin, errorFunc);
    }
    delete snapshot;

    // Set to global
    m_hErrorPlugin = errorPlugin;
    m_fnErrorFunc = errorFunc;
    return 0;
}

static any Native_Registry_ApplyAll(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    Handle applyAllPlugin = GetNativeCell(2);
    if (!applyAllPlugin)
        applyAllPlugin = plugin;

    Function applyAllFunc = GetNativeFunction(3);
    any data = GetNativeCell(3);

    PrivateForward fwd = new PrivateForward(ET_Ignore, Param_Cell, Param_Cell);
    if (!fwd.AddFunction(applyAllPlugin, applyAllFunc))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid ApplyAll Function.");

    StringMapSnapshot snapshot = m_hLoggers.Snapshot();
    for (int i = 0; i < snapshot.Length; ++i)
    {
        int keyLen = snapshot.KeyBufferSize(i);
        char[] key = new char[keyLen + 1];
        snapshot.GetKey(i, key, keyLen + 1);

        Logger logger;
        if (m_hLoggers.GetValue(key, logger))
        {
            Call_StartForward(fwd);
            Call_PushCell(logger);
            Call_PushCell(data);
            Call_Finish();
        }
    }
    delete snapshot;
    delete fwd;
    return 0;
}

static any Native_Registry_FlushAll(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    StringMapSnapshot snapshot = m_hLoggers.Snapshot();
    for (int i = 0; i < snapshot.Length; ++i)
    {
        int keyLen = snapshot.KeyBufferSize(i);
        char[] key = new char[keyLen + 1];
        snapshot.GetKey(i, key, keyLen + 1);

        Logger logger;
        if (m_hLoggers.GetValue(key, logger))
            logger.Flush();
    }
    delete snapshot;
    return 0;
}

static any Native_Registry_Drop(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    int len;
    GetNativeStringLength(2, len);
    char[] name = new char[len + 1];
    GetNativeString(2, name, len + 1);

    Logger logger;
    if (!m_hLoggers.GetValue(name, logger))
        return 0;

    // Drop and decrement the reference count
    if (m_hGlobalLogger.IsValid() && logger.Equals(m_hGlobalLogger))
        LoggerCleanupAndDelete(m_hGlobalLogger);

    m_hLoggers.Remove(name);
    LoggerCleanupAndDelete(logger);
    return 0;
}

static any Native_Registry_DropAll(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    StringMapSnapshot snapshot = m_hLoggers.Snapshot();
    for (int i = 0; i < snapshot.Length; ++i)
    {
        int keyLen = snapshot.KeyBufferSize(i);
        char[] key = new char[keyLen + 1];
        snapshot.GetKey(i, key, keyLen + 1);

        Log4spRegistry.Instance().Drop(key);
    }
    delete snapshot;
    return 0;
}

static any Native_Registry_SetAutomaticRegistration(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    m_bAutomaticRegistration = GetNativeCell(2);
    return 0;
}

// set levels for all existing/future loggers. global_level can be null if should not set.
static any Native_Registry_SetLevels(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    // <nameString, levelEnum>
    StringMap levels = GetNativeCell(2);
    LogLevel globalLevel = GetNativeCell(3);

    if (levels)
    {
        StringMapSnapshot snapshot = m_hLoggers.Snapshot();
        for (int i = 0; i < snapshot.Length; ++i)
        {
            // key: logger name
            int keyLen = snapshot.KeyBufferSize(i);
            char[] key = new char[keyLen + 1];
            snapshot.GetKey(i, key, keyLen + 1);

            Logger logger;
            if (m_hLoggers.GetValue(key, logger))
            {
                LogLevel lvl;
                if (levels.GetValue(key, lvl))
                    logger.SetLevel(lvl);
                else
                    logger.SetLevel(globalLevel);
            }
            // else: SM BUG?
        }
        delete snapshot;
    }

    delete m_hLogLevels;
    m_hLogLevels = view_as<StringMap>(CloneHandle(levels));
    m_eGlobalLogLevel = globalLevel;
    return 0;
}

static any Native_Registry_ApplyLoggerEnvLevels(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    Logger logger = GetNativeCell(2);
    if (!logger.IsValid())
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Logger Handle %x.", logger);

    // Get logger name
    int size = logger.GetNameLength() + 1;
    char[] name = new char[size];
    logger.GetName(name, size);

    // Set new level according to previously configured level or default level
    LogLevel lvl;
    if (m_hLogLevels.GetValue(name, lvl))
        logger.SetLevel(lvl);
    else
        logger.SetLevel(m_eGlobalLogLevel);
    return 0;
}

static any Native_Registry_Instance(Handle plugin, int numParams)
{
    // 似乎没意义, 但不额外占用内存, 不可关闭, 不可克隆, 全局唯一, 且生命周期吻合
    return m_pThis;
}


static bool IsValidRegistryHandle(Handle handle)
{
    return m_pThis == handle;
}

static Action Timer_FlushEvery(Handle timer)
{
    StringMapSnapshot snapshot = m_hLoggers.Snapshot();
    for (int i = 0; i < snapshot.Length; ++i)
    {
        int keyLen = snapshot.KeyBufferSize(i);
        char[] key = new char[keyLen + 1];
        snapshot.GetKey(i, key, keyLen + 1);

        Logger logger;
        if (m_hLoggers.GetValue(key, logger))
            logger.Flush();
    }
    delete snapshot;
    return Plugin_Continue;
}
