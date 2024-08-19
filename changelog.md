### 2024/08/19 - v1.3.0
- feat: support custom configuration
    - thread pool
        - queue size
        - thread count
    - default logger
        - name
        - type (sync or async)
        - log level
        - log message pattern
        - flush level
        - backtrace number

### 2024/08/09 - v1.2.2
- perf: change the default logger to asynchronous

### 2024/08/09 - v1.2.1
- fix: construct async logger directly may not work

### 2024/08/07 - v1.2.0
- feat: support asynchronous logging
    - Add the logger parameter async to configure the asynchronous logger
    - Add the logger parameter policy to configure the processing policy when the asynchronous queue overflows
    - Add the common enum AsyncOverflowPolicy to describe the processing policy when the asynchronous queue overflows
    - Deprecate the logger parameter mt

- refactor: benchmark plugin

### 2024/08/07 - v1.1.2
- docs: fix typos

### 2024/08/05 - v1.1.1
- fix: potential crashes caused by illegal parameters
    - Fix daily file sink crash when invalid rotation time in ctor
    - Fix rotating file sink crash when accepting invalid maxFileSize or maxFiles

### 2024/08/01 - v1.1.0
- feat: add server command should_log, flush, get_flush_lvl, set_flush_lvl, should_bt

### 2024/08/01 - v1.0.1
- fix: native sink constructor not found
    - Fix daily file sink constructor native not found
    - Fix rotating file sink constructor native not found

### 2024/07/29 - v1.0.0
- Initial release
