#ifndef _LOG4SP_SINK_REGISTRY_H_
#define _LOG4SP_SINK_REGISTRY_H_

#include <unordered_map>

#include "extension.h"


namespace log4sp {

struct SinkHandleInfo
{
    HandleType_t type;
    spdlog::sink_ptr sink;
};

/**
 * Note - 为什么需要这个类:
 * 1. sink native 手动创建的 sink 需要支持添加到 logger 里, 这使得 sink 的生命周期难以管理
 *  a. 使用原始指针创建 sink
 *      用户 create sink 时, 可以直接将原始指针传递给 handlesys->CreateHandleEx();
 *      由于不受智能指针影响, native 结束后 sink 也不会被释放;
 *      用户 delete sink 时, 直接删除原始指针 - 会造成 logger 指向的 sink 是空指针的问题;
 *      用户 delete sink 时, 不删除原始指针 - 会造成只创建但没有添加到 logger 的 sink 内存泄露.
 *  b. 使用智能指针创建 sink
 *      用户 create sink 时, 需要将通过 智能指针.get() 获取原始指针再传递给 handlesys->CreateHandleEx();
 *      由于传递的是原始指针, native 结束后智能指针引用计数归 0, 所以 sink 会被释放, 造成空指针问题.
 *  c. 使用智能指针 + SinkRegistry
 *      用户 create sink 时, 需要将通过 智能指针.get() 获取原始指针再传递给 handlesys->CreateHandleEx();
 *      由于创建 sink handle 成功后会被注册到 SinkRegistry, native 结束后引用计数为 1, 所以 sink 不会释放;
 *      用户 delete sink 时, 删除 SinkRegistry 引用, 是否释放取决于智能指针（没有 logger 引用这个 sink 时应该会被立即释放）
 * 2. HandleType_t 子类型受限 ( HANDLESYS_MAX_SUBTYPES 0xF  |  HANDLESYS_MAX_TYPES (1<<9) )
 *  a. 使用继承结构
 *      会限制后续拓展更多类型的 sink;
 *      logger addSink 简单, 只需将 handle 按基类 sink HandleType 读取, 就能得到 spdlog::sinks::sink *;
 *      基类的 sink native 读取 handle 简单, 只需将 handle 按基类 sink HandleType 读取, 就能得到 spdlog::sinks::sink *;
 *      派生类 sink methodmap 简洁, 只需要实现派生类拓展的 native 即可.
 *  b. 不使用继承结构
 *      后续可以拓展更多类型的 sink;
 *      logger addSink 困难, 因为不知道 handle 的 HandleType，需要每种 sink 专属一个 add;
 *      基类的 sink native 读取 handle 困难, 因为不知道 handle 的 HandleType, 每种 sink 都需要实现基类方法;
 *      派生类 sink methodmap 复杂, 每个派生类都需要实现基类的 native, 以便在读取 handle 时传递正确的 HandleType.
 *  c. 不使用继承结构 + SinkRegistry
 *      后续可以拓展更多类型的 sink;
 *      logger addSink 略微繁琐, 因为需要从 SinkRegistry 中查询 HandleType;
 *      基类的 sink native 读取 handle 略微繁琐, 因为需要从 SinkRegistry 中查询 HandleType;
 *      派生类 sink methodmap 简洁, 只需要实现派生类拓展的 native 即可.
 * 3. 添加一个新 sink 类型流程
 *  a. 在 src/extension.h 中 extern sink HandleType 以及 Natives 数组
 *  b. 在 src/extension.cpp 中创建/销毁 sink HandleType 以及注册 Natives 数组
 *  c. 在 src/natives 中实现基类 sink native 不包含的功能, 如创建对象，以及实现 Natives 数组
 *  d. 在 AMBuilder 中添加新 sink
 *  e. 在 sourcemod/scripting 中添加 sink include
 *
 * This has an impact on adding other sink types in the future.
 * ref:
 *  https://wiki.alliedmods.net/Handle_API_(SourceMod)#HandleType_t
 *  https://discord.com/channels/335290997317697536/335290997317697536/1255520855916806214
 *  https://discord.com/channels/335290997317697536/335290997317697536/1258060854205747241
 */
class SinkHandleRegistry
{
public:
    void registerSink(Handle_t handle, SinkHandleInfo info);

    void registerSink(Handle_t handle, HandleType_t type, spdlog::sink_ptr sink);

    bool hasKey(Handle_t handle);

    SinkHandleInfo *get(Handle_t handle);

    spdlog::sink_ptr getSink(Handle_t handle);

    HandleType_t getHandleType(Handle_t handle);

    /**
     * O(1)
     */
    bool drop(Handle_t handle);

    /**
     * O(n) - 需要遍历整个 map
     */
    bool drop(spdlog::sinks::sink *sink);

    void dropAll();

    void shutdown();

    static SinkHandleRegistry &instance();

private:
    std::unordered_map<Handle_t, SinkHandleInfo> sinks_;
};

} // namespace log4sp

#endif  // _LOG4SP_SINK_REGISTRY_H_
