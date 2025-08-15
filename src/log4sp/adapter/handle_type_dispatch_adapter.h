#pragma once

#include "extension.h"


namespace log4sp {
/**
 * 适配 SourceMod IHandleTypeDispatch
 *
 * 主要目标是适配 SourceMod 存储 object 的原始指针 -> 存储 object 的智能指针, 其次还封装了一些辅助函数以便于读写 Handle.
 *
 * 实现类需要指定 object 原始指针的类型, 以及 key_type 的类型, 并实现 get_key 方法.
 *
 *  通常 get_key 方法可以直接返回 object 的原始指针作为 key
 *  特殊需求如每个 logger 都有唯一的名称, 也可以用名称作为 key
 *
 *  实现类应该是一个单例类, 并在初始化时创建 Handle type, 销毁时释放 Handle type
 *  模板类只简单封装了 create_handle_type_ 和 remove_handle_type_
 */
template <typename Key, typename Object>
class handle_type_dispatch_adapter : public SourceMod::IHandleTypeDispatch {
    static_assert(std::is_default_constructible_v<std::hash<Key>> &&
                  std::is_copy_constructible_v<Key> &&
                  std::is_move_constructible_v<Key>,
                  "Key must be hashable and comparable");

public:
    using handle_access     = SourceMod::HandleAccess;
    using handle_error      = SourceMod::HandleError;
    using handle_security   = SourceMod::HandleSecurity;
    using handle_t          = SourceMod::Handle_t;
    using handle_type_t     = SourceMod::HandleType_t;
    using identity_token_t  = SourceMod::IdentityToken_t;
    using type_access       = SourceMod::TypeAccess;
    using key_t             = Key;
    using object_raw_t      = Object*;
    using object_ptr_t      = std::shared_ptr<Object>;

    /**
     * @brief Creates a new handle.
     *
     * @param object    Object to bind to the handle.
     * @param owner     Owner of the new Handle (may be NULL).
     * @param ident     Identity for type access if needed (may be NULL).
     * @param error     Optional pointer to store an error code on failure (undefined on success).
     * @return          A new Handle_t, or BAD_HANDLE on failure.
     */
    [[nodiscard]]
    auto create_handle(object_ptr_t object,
                       identity_token_t *owner,
                       identity_token_t *ident,
                       handle_error *error) noexcept -> handle_t;

    /**
     * @brief Creates a new handle.
     *
     * @param object    Object to bind to the handle.
     * @param security  Security pointer; pOwner is written as the owner,
     *                  pIdent is used as the parent identity for authorization.
     * @param access    Access right descriptor for the Handle; NULL for type defaults.
     * @param error     Optional pointer to store an error code on failure (undefined on success).
     * @return          A new Handle_t, or BAD_HANDLE on failure.
     */
    [[nodiscard]]
    auto create_handle(object_ptr_t object,
                       const handle_security *security,
                       const handle_access *access,
                       handle_error *error) noexcept -> handle_t;
    /**
     * @brief Frees the memory associated with a handle and calls any destructors.
     * NOTE: This function will decrement the internal reference counter.  It will
     * only perform any further action if the counter hits 0.
     *
     * @param handle    Handle_t identifier to destroy.
     * @param security  Security information struct (may be NULL).
     * @return          A HandleError error code.
     */
    auto free_handle(handle_t handle, const handle_security *security) noexcept -> handle_error;

    /**
     * @brief Retrieves the contents of a handle.
     *
     * @param handle    Handle_t from which to retrieve contents.
     * @param security  Security information struct (may be NULL).
     * @param error     HandleError error code.
     * @return          Object pointer, or nullptr on failure.
     */
    [[nodiscard]]
    auto read_handle(handle_t handle,
                     const handle_security *security,
                     handle_error *error) const noexcept -> object_ptr_t;

    /**
     * @brief Retrieves the contents of a handle.
     *
     * @param handle    Handle_t from which to retrieve contents.
     * @param security  Security information struct (may be NULL).
     * @param error     HandleError error code.
     * @return          Object pointer, or nullptr on failure.
     */
    [[nodiscard]]
    auto read_handle_raw(handle_t handle,
                         const handle_security *security,
                         handle_error *error) const noexcept -> object_raw_t;

    /**
     * @brief Get key by object.
     * @note  Derived classes must implement this.
     *
     * @param key       Object raw pointer.
     * @return          A Handle_t, or BAD_HANDLE on failure.
     */
    [[nodiscard]]
    virtual const key_t &get_key(object_raw_t &raw) const noexcept =0;

    [[nodiscard]]
    auto handle_type() const -> handle_type_t { return handle_type_; }

    handle_type_dispatch_adapter(const handle_type_dispatch_adapter &) = delete;
    handle_type_dispatch_adapter &operator=(const handle_type_dispatch_adapter &) = delete;
    handle_type_dispatch_adapter(const handle_type_dispatch_adapter &&) = delete;

protected:
    handle_type_dispatch_adapter() = default;
    virtual ~handle_type_dispatch_adapter() = default;

    auto create_handle_type_(const char *name,
                             handle_type_t parent,
                             const type_access *typeAccess,
                             const handle_access *hndlAccess,
                             identity_token_t *ident) noexcept -> handle_error;
    void remove_handle_type_() noexcept;

    void OnHandleDestroy(handle_type_t type, void *object) noexcept override;

    handle_type_t handle_type_{NO_HANDLE_TYPE};
    std::unordered_map<key_t, std::pair<handle_t, object_ptr_t>> datas_;
};


// public
template <class Key, class Object>
inline auto handle_type_dispatch_adapter<Key, Object>::create_handle(
    object_ptr_t obj,
    identity_token_t *owner,
    identity_token_t *ident,
    handle_error *error) noexcept -> handle_t
{
    auto raw = obj.get();
    auto handle = handlesys->CreateHandle(handle_type_, raw, owner, ident, error);
    if (!handle) {
        return BAD_HANDLE;
    }

    auto key = get_key(raw);
    datas_[key] = {handle, obj};
    return handle;
}

template <class Key, class Object>
inline auto handle_type_dispatch_adapter<Key, Object>::create_handle(
    object_ptr_t obj,
    const handle_security *security,
    const handle_access *access,
    handle_error *error) noexcept -> handle_t
{
    auto raw = obj.get();
    auto handle = handlesys->CreateHandleEx(handle_type_, raw, security, access, error);
    if (!handle) {
        return BAD_HANDLE;
    }

    auto key = get_key(raw);
    datas_[key] = {handle, obj};
    return handle;
}

template <class Key, class Object>
inline auto handle_type_dispatch_adapter<Key, Object>::free_handle(
    handle_t handle,
    const handle_security *security) noexcept -> handle_error
{
    auto error = handlesys->FreeHandle(handle, security);
    assert(error == handle_error::HandleError_None);
    return error;
}

template <class Key, class Object>
inline auto handle_type_dispatch_adapter<Key, Object>::read_handle(
    handle_t handle,
    const handle_security *security,
    handle_error *error) const noexcept -> object_ptr_t
{
    auto raw = read_handle_raw(handle, security, error);
    if (!raw) {
        return nullptr;
    }

    auto key = get_key(raw);
    assert(datas_.find(key) != datas_.end());
    return datas_.find(key)->second.second;
}

template <class Key, class Object>
inline auto handle_type_dispatch_adapter<Key, Object>::read_handle_raw(
    handle_t handle,
    const handle_security *security,
    handle_error *error) const noexcept -> object_raw_t
{
    object_raw_t raw;
    handle_error err = handlesys->ReadHandle(handle, handle_type_, security, (void **)&raw);
    if (err != handle_error::HandleError_None) {
        if (error) {
            *error = err;
        }
        return nullptr;
    }

    assert(datas_.find(get_key(raw)) != datas_.end());
    return raw;
}


// protected
template <class Key, class Object>
inline auto handle_type_dispatch_adapter<Key, Object>::create_handle_type_(
    const char *name,
    handle_type_t parent,
    const type_access *typeAccess,
    const handle_access *hndlAccess,
    identity_token_t *ident) noexcept -> handle_error
{
    using spdlog::fmt_lib::format;

    handle_error error = handle_error::HandleError_None;
    handle_type_ = handlesys->CreateType(name, this, parent, typeAccess, hndlAccess, ident, &error);
    return error;
}

template <class Key, class Object>
inline void handle_type_dispatch_adapter<Key, Object>::remove_handle_type_() noexcept
{
    if (handle_type_) {
        handlesys->RemoveType(handle_type_, myself->GetIdentity());
        handle_type_ = NO_HANDLE_TYPE;
    }
    assert(datas_.size() == 0);
}

template <class Key, class Object>
inline void handle_type_dispatch_adapter<Key, Object>::OnHandleDestroy(handle_type_t type, void *object) noexcept
{
    auto raw = static_cast<object_raw_t>(object);
    auto key = get_key(raw);

    assert(datas_.find(key) != datas_.end());

    datas_.erase(key);
}


}       // namespace log4sp
