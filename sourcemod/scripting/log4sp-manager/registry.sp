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
    CreateNative("LoggerRegistry.RegisterLogger",           Native_Registry_RegisterLogger);
    CreateNative("LoggerRegistry.RegisterOrReplaceLogger",  Native_Registry_RegisterOrReplaceLogger);
    CreateNative("LoggerRegistry.InitializeLogger",         Native_Registry_InitializeLogger);
    CreateNative("LoggerRegistry.Get",                      Native_Registry_Get);
    CreateNative("LoggerRegistry.GetGlobal",                Native_Registry_GetGlobal);
    CreateNative("LoggerRegistry.GetGlobalRaw",             Native_Registry_GetGlobalRaw);
    CreateNative("LoggerRegistry.SetGlobalLogger",          Native_Registry_SetGlobalLogger);
    CreateNative("LoggerRegistry.SetLevel",                 Native_Registry_SetLevel);
    CreateNative("LoggerRegistry.SetPattern",               Native_Registry_SetPattern);
    CreateNative("LoggerRegistry.SetFlushLevel",            Native_Registry_SetFlushLevel);
    CreateNative("LoggerRegistry.SetFlushEvery",            Native_Registry_SetFlushEvery);
    CreateNative("LoggerRegistry.GetFlusher",               Native_Registry_GetFlusher);
    CreateNative("LoggerRegistry.SetErrorHandler",          Native_Registry_SetErrorHandler);
    CreateNative("LoggerRegistry.ApplyAll",                 Native_Registry_ApplyAll);
    CreateNative("LoggerRegistry.FlushAll",                 Native_Registry_FlushAll);
    CreateNative("LoggerRegistry.Drop",                     Native_Registry_Drop);
    CreateNative("LoggerRegistry.DropAll",                  Native_Registry_DropAll);
    CreateNative("LoggerRegistry.SetAutomaticRegistration", Native_Registry_SetAutomaticRegistration);
    CreateNative("LoggerRegistry.SetLevels",                Native_Registry_SetLevels);
    CreateNative("LoggerRegistry.ApplyLoggerEnvLevels",     Native_Registry_ApplyLoggerEnvLevels);
    CreateNative("LoggerRegistry.Instance",                 Native_Registry_Instance);
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
    char name[] = "";
    ServerConsoleSink sink = new ServerConsoleSink();
    m_hGlobalLogger = new Logger(name);
    m_hGlobalLogger.AddSink(sink);
    SinkCloseAndDelete(sink);

    // Register global logger
    // raw global handle        -->
    //                              ==>  Global logger
    // registry clone handle    -->
    m_hLoggers.SetValue(name, m_hGlobalLogger.Clone());
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
                LoggerRegistry.Instance().Drop(key);
                LogMessage("[Patch] Due to the unload of plugin \"%s\", the registry drop the logger \"%s\".", filename, key);
            }
        }
    }
    delete snapshot;
#endif
}

void RegistryErrorHandlerPatch(Handle plugin)
{
    if (plugin == m_hErrorPlugin)
    {
        m_hErrorPlugin = null;
        m_fnErrorFunc = INVALID_FUNCTION;
    }
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

    // Cloning and registering the clone's handle
    m_hLoggers.SetValue(name, logger.Clone());
    return 0;
}

static any Native_Registry_RegisterOrReplaceLogger(Handle plugin, int numParams)
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

    // Drop if exists
    if (m_hLoggers.ContainsKey(name))
        LoggerRegistry.Instance().Drop(name);

    // Cloning and registering the clone's handle
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
        LoggerRegistry.Instance().RegisterLogger(logger);
    return 0;
}

static any Native_Registry_Get(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    if (IsNativeParamNullString(2))
    {
        char filename[PLATFORM_MAX_PATH];
        GetPluginFilename(plugin, filename, sizeof(filename));

        Logger logger;
        return m_hLoggers.GetValue(filename, logger) ? logger : null;
    }
    else
    {
        int len;
        GetNativeStringLength(2, len);
        char[] name = new char[len + 1];
        GetNativeString(2, name, len + 1);

        Logger logger;
        return m_hLoggers.GetValue(name, logger) ? logger : null;
    }
}

static any Native_Registry_GetGlobal(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    if (!m_hGlobalLogger.IsValid())
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
    if (m_hGlobalLogger.IsValid())
    {
        if (logger.Equals(m_hGlobalLogger))
            return 0;

        // Get logger name
        int size1 = logger.GetNameLength() + 1;
        char[] name1 = new char[size1];
        logger.GetName(name1, size1);

        // Get global logger name
        int size2 = m_hGlobalLogger.GetNameLength() + 1;
        char[] name2 = new char[size2];
        m_hGlobalLogger.GetName(name2, size2);

        // Throw if exists
        if (m_hLoggers.ContainsKey(name1) && !StrEqual(name1, name2))
            ThrowNativeError(SP_ERROR_PARAM, "Logger with name \"%s\" already exists.", name1);

        LoggerRegistry.Instance().Drop(name2);
        LoggerCloseAndDelete(m_hGlobalLogger);
    }

    // Set to global
    m_hGlobalLogger = view_as<Logger>(logger.Clone());

    // Register new global logger
    LoggerRegistry.Instance().RegisterLogger(m_hGlobalLogger);
    return 0;
}

static any Native_Registry_SetLevel(Handle plugin, int numParams)
{
    Handle pThis = GetNativeCell(1);
    if (!IsValidRegistryHandle(pThis))
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Registry Handle %x.", pThis);

    LogLevel lvl = GetNativeCell(2);

    // apply to loggers
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

    int size;
    {
        int err = GetNativeStringLength(2, size);
        if (err != SP_ERROR_NONE)
            ThrowNativeError(SP_ERROR_PARAM, "Invalid pattern (error %d)", err);
    }

    char[] pattern = new char[size + 1];
    {
        int err = GetNativeString(2, pattern, size + 1);
        if (err != SP_ERROR_NONE)
            ThrowNativeError(SP_ERROR_PARAM, "Invalid pattern (error %d)", err);
    }

    PatternTimeType type = GetNativeCell(3);

    // Save to global
    m_hPatternData.Reset(true);
    m_hPatternData.WriteCell(size);
    m_hPatternData.WriteString(pattern);
    m_ePatternTimeType = type;

    // apply to loggers
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

    // apply to loggers
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
        delete m_hFlushTimer;

    if (interval > 0.0)
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
    if (!errorPlugin)
        errorPlugin = plugin;

    // HACK: Replace GetNativeFunction() with GetNativeCell() to suppress compiler warnings.
    // warning 237: coercing functions to and from primitives is unsupported and will be removed in the future
    // https://github.com/alliedmodders/sourcepawn/pull/644
    // https://github.com/alliedmodders/sourcepawn/issues/671
    ErrorHandler errorFunc = GetNativeCell(3);
    if (errorFunc == INVALID_FUNCTION)
        ThrowNativeError(SP_ERROR_PARAM, "Invalid Error Handler.");

    // apply to loggers
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
    any data = GetNativeCell(4);

    PrivateForward fwd = new PrivateForward(ET_Ignore, Param_Cell, Param_Cell);
    if (!fwd.AddFunction(applyAllPlugin, applyAllFunc))
    {
        delete fwd;
        ThrowNativeError(SP_ERROR_PARAM, "Invalid ApplyAll Function.");
    }

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
            int error = Call_Finish();
            if (error != SP_ERROR_NONE)
            {
                delete snapshot;
                delete fwd;
                ThrowNativeError(SP_ERROR_PARAM, "Failed to execute ApplyAll forward (error %d)", error);
            }
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

    if (IsNativeParamNullString(2))
    {
        char filename[PLATFORM_MAX_PATH];
        GetPluginFilename(plugin, filename, sizeof(filename));

        Logger logger;
        if (!m_hLoggers.GetValue(filename, logger))
            return 0;

        // Close the raw global logger
        if (m_hGlobalLogger.IsValid() && logger.Equals(m_hGlobalLogger))
            LoggerCloseAndDelete(m_hGlobalLogger);

        // Close and unregister the clone's handle.
        m_hLoggers.Remove(filename);
        LoggerCloseAndDelete(logger);
    }
    else
    {
        int len;
        GetNativeStringLength(2, len);
        char[] name = new char[len + 1];
        GetNativeString(2, name, len + 1);

        Logger logger;
        if (!m_hLoggers.GetValue(name, logger))
            return 0;

        // Close the raw global logger
        if (m_hGlobalLogger.IsValid() && logger.Equals(m_hGlobalLogger))
            LoggerCloseAndDelete(m_hGlobalLogger);

        // Close and unregister the clone's handle.
        m_hLoggers.Remove(name);
        LoggerCloseAndDelete(logger);
    }
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

        LoggerRegistry.Instance().Drop(key);
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
    LogLevel lvl = GetNativeCell(3);

    // apply to loggers
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
            LogLevel level;
            if (levels.GetValue(key, level))
                logger.SetLevel(level);
            else
                logger.SetLevel(lvl);
        }
        // else: SM BUG?
    }
    delete snapshot;

    delete m_hLogLevels;
    m_hLogLevels = view_as<StringMap>(CloneHandle(levels));
    m_eGlobalLogLevel = lvl;
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
    if (m_hLogLevels && m_hLogLevels.GetValue(name, lvl))
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
