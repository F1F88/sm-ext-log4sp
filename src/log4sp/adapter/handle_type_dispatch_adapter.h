#pragma once

#include "extension.h"


namespace Log4sp {

/**
 * IHandleTypeDispatchAdapter 模板设计初衷
 *  HeaderOnly 采用 StringMap 模拟类行为, 而 StringMap Handle 权限为默认值
 *  Extension 为了减少差异也采用默认权限, 因此除了 Handle Name 子类几乎相同
 *  使用模板也许会方便一些, 并且调整了一些传参使调用更简洁, 添加了一些检查以用于 DEBUG
 */
template <typename TypePointer, const char *TypeName>
class IHandleTypeDispatchAdapter : public SourceMod::IHandleTypeDispatch
{
public:
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
    auto CreateHandle(TypePointer object,
                      SourceMod::IdentityToken_t *owner,
                      SourceMod::IdentityToken_t *ident,
                      SourceMod::HandleError *error) noexcept -> SourceMod::Handle_t;

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
    auto CreateHandle(TypePointer object,
                      const SourceMod::HandleSecurity *security,
                      const SourceMod::HandleAccess *access,
                      SourceMod::HandleError *error) noexcept -> SourceMod::Handle_t;

    /**
     * @brief Clones a handle by adding to its internal reference count.
     *        Its data, type, and security permissions remain the same.
     *
     * @param handle    Handle to duplicate.  Any non-free handle target is valid.
     * @param owner     New owner of cloned handle.
     * @param security  Security information struct (may be NULL).
     * @param error     Optional pointer to store an error code on failure (undefined on success).
     * @return          A duplicated handle, or BAD_HANDLE on failure.
     */
    auto CloneHandle(SourceMod::Handle_t handle,
                     SourceMod::IdentityToken_t *owner,
                     SourceMod::HandleSecurity *security,
                     SourceMod::HandleError *error) noexcept -> SourceMod::Handle_t;

    /**
     * @brief Frees the memory associated with a handle and calls any destructors.
     * @note  This function will decrement the internal reference counter.
     *        It will only perform any further action if the counter hits 0.
     *
     * @param handle    Handle_t identifier to destroy.
     * @param security  Security information struct (may be NULL).
     * @return          A HandleError error code.
     */
    auto FreeHandle(SourceMod::Handle_t handle,
                    const SourceMod::HandleSecurity *security) noexcept -> SourceMod::HandleError;

    /**
     * @brief Retrieves the contents of a handle.
     *
     * @param handle    Handle_t from which to retrieve contents.
     * @param security  Security information struct (may be NULL).
     * @param error     HandleError error code.
     * @return          Object pointer, or nullptr on failure.
     */
    [[nodiscard]]
    auto ReadHandle(SourceMod::Handle_t handle,
                    const SourceMod::HandleSecurity *security,
                    SourceMod::HandleError *error) const noexcept -> TypePointer;

    SourceMod::HandleType_t m_HandleType{NO_HANDLE_TYPE};

    IHandleTypeDispatchAdapter(const IHandleTypeDispatchAdapter &) = delete;
    IHandleTypeDispatchAdapter(const IHandleTypeDispatchAdapter &&) = delete;
    IHandleTypeDispatchAdapter &operator=(const IHandleTypeDispatchAdapter &) = delete;

    static void Initialize()        { (void)Instance(); }
    static void Destroy() noexcept  { Instance().RemoveHandleType(); }

    [[nodiscard]]
    static IHandleTypeDispatchAdapter &Instance() noexcept {
        static IHandleTypeDispatchAdapter instance;
        return instance;
    }

    static constexpr const char *m_NAME{TypeName};

protected:
    IHandleTypeDispatchAdapter();
    virtual ~IHandleTypeDispatchAdapter() = default;

    auto CreateHandleType(const char *name,
                          SourceMod::HandleType_t parent,
                          const SourceMod::TypeAccess *typeAccess,
                          const SourceMod::HandleAccess *hndlAccess,
                          SourceMod::IdentityToken_t *ident) noexcept -> SourceMod::HandleError;
    void RemoveHandleType() noexcept;

    void OnHandleDestroy(SourceMod::HandleType_t type, void *object) noexcept override;
};


// public
template <class TypePointer, const char *TypeName>
inline auto IHandleTypeDispatchAdapter<TypePointer, TypeName>::CreateHandle(
    TypePointer object,
    SourceMod::IdentityToken_t *owner,
    SourceMod::IdentityToken_t *ident,
    SourceMod::HandleError *error) noexcept -> SourceMod::Handle_t
{
    return handlesys->CreateHandle(m_HandleType, object, owner, ident, error);
}

template <class TypePointer, const char *TypeName>
inline auto IHandleTypeDispatchAdapter<TypePointer, TypeName>::CreateHandle(
    TypePointer object,
    const SourceMod::HandleSecurity *security,
    const SourceMod::HandleAccess *access,
    SourceMod::HandleError *error) noexcept -> SourceMod::Handle_t
{
    return handlesys->CreateHandleEx(m_HandleType, object, security, access, error);
}

template <class TypePointer, const char *TypeName>
inline auto IHandleTypeDispatchAdapter<TypePointer, TypeName>::CloneHandle(
    SourceMod::Handle_t handle,
    SourceMod::IdentityToken_t *owner,
    SourceMod::HandleSecurity *security,
    SourceMod::HandleError *error) noexcept -> SourceMod::Handle_t
{
    SourceMod::Handle_t result = BAD_HANDLE;
    auto err = handlesys->CloneHandle(handle, &result, owner, security);
    if (error)
        *error = err;
    return result;
}

template <class TypePointer, const char *TypeName>
inline auto IHandleTypeDispatchAdapter<TypePointer, TypeName>::FreeHandle(
    SourceMod::Handle_t handle,
    const SourceMod::HandleSecurity *security) noexcept -> SourceMod::HandleError
{
    auto error = handlesys->FreeHandle(handle, security);
#if !defined(NDEBUG) || (defined(DEBUG) || defined(_DEBUG))
    if (error)
        smutils->LogError(myself, "Failed to free a %s Handle %x (error %d)", TypeName, handle, error);
#endif
    return error;

}

template <class TypePointer, const char *TypeName>
inline auto IHandleTypeDispatchAdapter<TypePointer, TypeName>::ReadHandle(
    SourceMod::Handle_t handle,
    const SourceMod::HandleSecurity *security,
    SourceMod::HandleError *error) const noexcept -> TypePointer
{
    TypePointer object;
    auto err = handlesys->ReadHandle(handle, m_HandleType, security, (void **)&object);
    if (err != SourceMod::HandleError::HandleError_None)
    {
        if (error)
            *error = err;
        return nullptr;
    }
    return object;
}


// protected
template <class TypePointer, const char *TypeName>
inline IHandleTypeDispatchAdapter<TypePointer, TypeName>::IHandleTypeDispatchAdapter()
{
    auto err = CreateHandleType(TypeName, NO_HANDLE_TYPE, nullptr, nullptr, myself->GetIdentity());
    if (err != SourceMod::HandleError_None)
        throw std::runtime_error("Failed to create \"" + std::string(TypeName) + "\" Handle type! error " + std::to_string(err) + ".");
}

template <class TypePointer, const char *TypeName>
inline auto IHandleTypeDispatchAdapter<TypePointer, TypeName>::CreateHandleType(
    const char *name,
    SourceMod::HandleType_t parent,
    const SourceMod::TypeAccess *typeAccess,
    const SourceMod::HandleAccess *hndlAccess,
    SourceMod::IdentityToken_t *ident) noexcept -> SourceMod::HandleError
{
    SourceMod::HandleError error = SourceMod::HandleError::HandleError_None;
    m_HandleType = handlesys->CreateType(name, this, parent, typeAccess, hndlAccess, ident, &error);
    return error;
}

template <class TypePointer, const char *TypeName>
inline void IHandleTypeDispatchAdapter<TypePointer, TypeName>::RemoveHandleType() noexcept
{
    if (m_HandleType)
    {
        handlesys->RemoveType(m_HandleType, myself->GetIdentity());
        m_HandleType = NO_HANDLE_TYPE;
    }
}

template <class TypePointer, const char *TypeName>
inline void IHandleTypeDispatchAdapter<TypePointer, TypeName>::OnHandleDestroy(SourceMod::HandleType_t type, void *object) noexcept
{
    delete static_cast<TypePointer>(object);
}


}       // namespace Log4sp
