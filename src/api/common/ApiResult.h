/// @file ApiResult.h
/// @brief Result type for API operations

#ifndef NERIPLAYERQT_APIRESULT_H
#define NERIPLAYERQT_APIRESULT_H

#include "api/common/ApiError.h"

#include <optional>

namespace NeriPlayerQt {

/**
 * @brief Result type holding either a value of type T or an ApiError
 *
 * Used as the return type for all API operations. Callers must check
 * isSuccess() or use the implicit bool conversion before accessing data().
 *
 * @tparam T The success value type
 */
template <typename T>
class ApiResult {
public:
    /**
     * @brief Construct a successful result
     */
    explicit ApiResult(T value)
        : m_value(std::move(value))
    {
    }

    /**
     * @brief Construct an error result
     */
    explicit ApiResult(ApiError error)
        : m_error(std::move(error))
    {
    }

    /**
     * @brief Check if the result is successful
     */
    bool isSuccess() const
    {
        return m_value.has_value();
    }

    /**
     * @brief Check if the result is an error
     */
    bool isError() const
    {
        return !m_value.has_value();
    }

    /**
     * @brief Implicit bool conversion — true if successful
     */
    explicit operator bool() const
    {
        return isSuccess();
    }

    /**
     * @brief Get the success value
     *
     * @return const reference to the value
     * @note Behavior is undefined if isError() is true. Always check isSuccess() first.
     */
    const T &data() const
    {
        return *m_value;
    }

    /**
     * @brief Get the error
     *
     * @return const reference to the error
     * @note Behavior is undefined if isSuccess() is true. Always check isError() first.
     */
    const ApiError &error() const
    {
        return m_error;
    }

private:
    std::optional<T> m_value;
    ApiError m_error;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_APIRESULT_H
