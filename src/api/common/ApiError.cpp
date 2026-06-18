/// @file ApiError.cpp
/// @brief API error type implementation

#include "api/common/ApiError.h"

namespace QeriPlayerQt {

ApiError::ApiError(int code, const QString &message, const QString &details)
    : m_code(code)
    , m_message(message)
    , m_details(details)
{
}

int ApiError::code() const
{
    return m_code;
}

const QString &ApiError::message() const
{
    return m_message;
}

const QString &ApiError::details() const
{
    return m_details;
}

bool ApiError::isNetworkError() const
{
    return m_code == -1;
}

bool ApiError::isAuthError() const
{
    // HTTP status codes
    if (m_code == 401 || m_code == 403) {
        return true;
    }
    // NetEase-specific body codes
    if (m_code == -10 || m_code == -460) {
        return true;
    }
    return false;
}

bool ApiError::isRateLimitError() const
{
    return m_code == 429 || m_code == -429;
}

bool ApiError::isNotFoundError() const
{
    return m_code == 404;
}

QString ApiError::userMessage() const
{
    if (isNetworkError()) {
        return QStringLiteral("Network connection failed. Please check your internet connection.");
    }
    if (isAuthError()) {
        if (m_code == -460) {
            return QStringLiteral("Account restricted. Please try again later.");
        }
        return QStringLiteral("Authentication expired. Please log in again.");
    }
    if (isRateLimitError()) {
        return QStringLiteral("Too many requests. Please wait a moment and try again.");
    }
    if (isNotFoundError()) {
        return QStringLiteral("The requested resource was not found.");
    }
    if (!m_message.isEmpty()) {
        return m_message;
    }
    return QStringLiteral("An unknown error occurred.");
}

} // namespace QeriPlayerQt
