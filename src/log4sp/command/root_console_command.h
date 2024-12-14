
#ifndef _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_H_
#define _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_H_

#include "extension.h"


namespace log4sp {

class logger_proxy;

class command {
public:
    virtual void execute(const ICommandArgs *args) = 0;

protected:
    std::shared_ptr<logger_proxy> arg_to_logger(const ICommandArgs *args, int num);

    spdlog::level::level_enum arg_to_level(const ICommandArgs *args, int num);
};


class get_lvl_command final : public command {
public:
    void execute(const ICommandArgs *args) override;
};


class set_lvl_command final : public command {
public:
    void execute(const ICommandArgs *args) override;
};


class set_pattern_command final : public command {
public:
    void execute(const ICommandArgs *args) override;
};


class should_log_command final : public command {
public:
    void execute(const ICommandArgs *args) override;
};


class log_command final : public command {
public:
    void execute(const ICommandArgs *args) override;
};


class flush_command final : public command {
public:
    void execute(const ICommandArgs *args) override;
};


class get_flush_lvl_command final : public command {
public:
    void execute(const ICommandArgs *args) override;
};


class set_flush_lvl_command final : public command {
public:
    void execute(const ICommandArgs *args) override;
};


class should_bt_command final : public command {
public:
    void execute(const ICommandArgs *args) override;
};


class enable_bt_command final : public command {
public:
    void execute(const ICommandArgs *args) override;
};


class disable_bt_command final : public command {
public:
    void execute(const ICommandArgs *args) override;
};


class dump_bt_command final : public command {
public:
    void execute(const ICommandArgs *args) override;
};


}       // namespace log4sp
#include "log4sp/command/root_console_command-inl.h"
#endif  // _LOG4SP_COMMAND_ROOT_CONSOLE_COMMAND_H_
