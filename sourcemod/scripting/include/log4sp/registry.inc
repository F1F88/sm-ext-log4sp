#if defined _log4sp_manager_register_included
    #endinput
#endif
#define _log4sp_manager_register_included

#include <log4sp/logger>
#include <log4sp/sinks/sink>


/**
 * Callback function for apply a user defined function on all Logger Handles.
 *
 * @param logger    Logger Handle.
 * @param data      Data value passed to callback.
 */
typeset ApplyAllFunction
{
    function void (Logger logger);
    function void (Logger logger, any data);
};


methodmap Registry < Handle
{
    /**
     * Clone the logger Handle and register it in the registry.
     *
     * @note Registered loggers must be freed via Registry.Drop().
     *
     * @param logger    Logger Handle to be registered.
     * @error           Invalid Logger Handle, or Logger with name already exists.
     */
    public native void RegisterLogger(Logger logger);

    /**
     * Initialize the logger Handle's PatternFormatter, Log level, Flush level, ErrorHandler
     * according to the global settings, then clone the logger Handle and register it in the
     * registry.
     *
     * @note Registered loggers must be freed via Registry.Drop().
     *
     * @param logger    Logger Handle to be registered.
     * @error           Invalid Logger Handle, or Logger with name already exists.
     */
    public native void InitializeLogger(Logger logger);

    /**
     * Gets a logger Handle by logger name.
     *
     * @param name      The name of the logger.
     * @return          Logger Handle or INVALID_HANDLE if a logger with such name doesn't exist.
     */
    public native Logger Get(const char[] name);

    /**
     * Gets the global logger Handle from the registry.
     *
     * @note The global logger is created when the Log4sp Manager plugin is loaded;
     *       it is nameless and has a ServerConsoleSink.
     *
     * @return          Logger Handle or INVALID_HANDLE when the global logger is dropped.
     */
    public native Logger GetGlobal();

    /**
     * Gets the global logger Handle.
     *
     * @return          Logger Handle or INVALID_HANDLE when the global logger is dropped.
     */
    public native Logger GetGlobalRaw();

    /**
     * Clone the logger handle and set it as the global logger.
     *
     * @note The old global logger Handle will be drop and freed first.
     *
     * @param logger    Logger Handle to be registered.
     * @error           Invalid Logger Handle.
     */
    public native void SetGlobalLogger(Logger logger);

    /**
     * Set the global log level and update all loggers in the registry.
     *
     * @param lvl       Log level enum.
     */
    public native void SetLevel(LogLevel lvl);

    /**
     * Set the global formatting pattern and update all loggers in the registry.
     *
     * @note pattern flags: https://github.com/gabime/spdlog/wiki/Custom-formatting#pattern-flags
     *
     * @param pattern   Formatting pattern.
     * @param type      Pattern time.
     */
    public native void SetPattern(const char[] pattern, PatternTimeType type);

    /**
     * Set the global flush level and update all loggers in the registry.
     *
     * @param lvl       Log level enum.
     */
    public native void SetFlushLevel(LogLevel lvl);

    /**
     * Periodically flush the contents of all loggers in the registry.
     *
     * @param interval  Duration (in seconds) of each refresh interval.
     */
    public native void SetFlushEvery(float interval);

    public native Timer GetFlusher();

    /**
     * Set the global handler function for logging errors and update all loggers in the registry.
     *
     * @note The internal implementation contains warnings and may be removed in the future.
     *
     * @param plugin    Handle of the plugin that contains the function.
     *                  If null, defaults to the calling plugin.
     * @param func      Callback function to call when an error occurs.
     * @error           Invalid or corrupt plugin handle, or invalid function.
     */
    public native void SetErrorHandler(Handle plugin, ErrorHandler func);

    /**
     * Apply a user defined function on all loggers.
     *
     * @param plugin    Handle of the plugin that contains the function.
     *                  If null, defaults to the calling plugin.
     * @param func      Callback function that apply a user defined function on all loggers.
     * @param data      Extra data value to pass to the callback.
     * @error           Invalid apply all function.
     */
    public native void ApplyAll(Handle plugin, ApplyAllFunction func, any data=0);

    /**
     * Manual flush the contents of all loggers in the registry.
     */
    public native void FlushAll();

    /**
     * Unregister and close the cloned logger from the registry.
     *
     * @param name      The name of the logger.
     */
    public native void Drop(const char[] name);

    /**
     * Unregister and close all cloned loggers from the registry.
     */
    public native void DropAll();

    /**
     * Sets whether the Logger is automatically registered to the registry when it is initialized.
     *
     * @param automaticRegistration     True to enable auto-registration.
     */
    public native void SetAutomaticRegistration(bool automaticRegistration);

    /**
     * InitializeLogger, ApplyLoggerEnvLevels First set the log level according to this setting.
     * If none exists, then set the log level according to the global log level setting.
     *
     * @note This native clones and saves the levels handle.
     *
     * @param levels        The key is the logger name, and the value is the log level.
     *                      If null, do not modify.
     * @param globalLevel   Global log level.
     */
    public native void SetLevels(StringMap levels, LogLevel globalLevel);

    /**
     * Set the log level based on the environment or use the global log level setting.
     *
     * @param logger    Logger Handle to be set log level.
     * @error           Invalid Logger Handle.
     */
    public native void ApplyLoggerEnvLevels(Logger logger);

    public static native Registry Instance();

    public static bool LibraryExists() {
        return LibraryExists("log4sp_manager");
    }
}




/* DO NOT EDIT BELOW THIS LINE */
public SharedPlugin __pl_log4sp_manager =
{
    name = "log4sp_manager",
    file = "log4sp_manager.smx",
#if defined REQUIRE_PLUGIN
    required = 1,
#else
    required = 0,
#endif
};

#if !defined REQUIRE_PLUGIN
public void __pl_log4sp_manager_SetNTVOptional()
{
    MarkNativeAsOptional("Registry.RegisterLogger");
    MarkNativeAsOptional("Registry.InitializeLogger");
    MarkNativeAsOptional("Registry.Get");
    MarkNativeAsOptional("Registry.GetGlobal");
    MarkNativeAsOptional("Registry.GetGlobalRaw");
    MarkNativeAsOptional("Registry.SetGlobalLogger");
    MarkNativeAsOptional("Registry.SetLevel");
    MarkNativeAsOptional("Registry.SetPattern");
    MarkNativeAsOptional("Registry.SetFlushLevel");
    MarkNativeAsOptional("Registry.SetFlushEvery");
    MarkNativeAsOptional("Registry.GetFlusher");
    MarkNativeAsOptional("Registry.SetErrorHandler");
    MarkNativeAsOptional("Registry.ApplyAll");
    MarkNativeAsOptional("Registry.FlushAll");
    MarkNativeAsOptional("Registry.Drop");
    MarkNativeAsOptional("Registry.DropAll");
    MarkNativeAsOptional("Registry.SetAutomaticRegistration");
    MarkNativeAsOptional("Registry.SetLevels");
    MarkNativeAsOptional("Registry.ApplyLoggerEnvLevels");
    MarkNativeAsOptional("Registry.Instance");
}
#endif
