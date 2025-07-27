
#pragma once

#include <optional>


namespace log4sp {

class logger;   // forward declaration

class command {
public:
    enum class status : std::uint8_t {
        ok,
        usage_error,
        param_error,
        execute_error
    };
    using result = std::pair<command::status, std::optional<std::string>>;

    virtual ~command() = default;

    /**
     * @brief 执行命令
     *
     * @param args      参数列表 (不包含 command 自身, 以及前缀指令)
     * @return          命令执行结果
     */
    virtual result execute(const std::vector<std::string> &args) noexcept = 0;

    static result ok()                             { return result{command::status::ok, std::nullopt}; }
    static result usage_error(std::string err)     { return result{command::status::usage_error, std::move(err)}; }
    static result param_error(std::string err)     { return result{command::status::param_error, std::move(err)}; }
    static result execute_error(std::string err)   { return result{command::status::execute_error, std::move(err)}; }
};


class list_command final : public command {
public:
    result execute(const std::vector<std::string> &args) noexcept override;
};


class apply_all_command final : public command {
public:
    result execute(const std::vector<std::string> &args) noexcept override;

private:
    static const std::unordered_map<std::string_view, std::unique_ptr<command>> functions_;
};


class get_lvl_command final : public command {
public:
    result execute(const std::vector<std::string> &args) noexcept override;
};


class set_lvl_command final : public command {
public:
    result execute(const std::vector<std::string> &args) noexcept override;
};


class set_pattern_command final : public command {
public:
    result execute(const std::vector<std::string> &args) noexcept override;
};


class should_log_command final : public command {
public:
    result execute(const std::vector<std::string> &args) noexcept override;
};


class log_command final : public command {
public:
    result execute(const std::vector<std::string> &args) noexcept override;
};


class flush_command final : public command {
public:
    result execute(const std::vector<std::string> &args) noexcept override;
};


class get_flush_lvl_command final : public command {
public:
    result execute(const std::vector<std::string> &args) noexcept override;
};


class set_flush_lvl_command final : public command {
public:
    result execute(const std::vector<std::string> &args) noexcept override;
};


class version_command final : public command {
public:
    result execute(const std::vector<std::string> &args) noexcept override;
};


}       // namespace log4sp
