/// @file ViewModelError.cpp
/// @brief Implementation of ViewModelError

#include "viewmodel/ViewModelError.h"

namespace QeriPlayerQt {

ViewModelError::ViewModelError(ErrorType type, const QString &message, const QString &details)
    : m_type(type)
    , m_message(message)
    , m_details(details)
{
}

// --- Factory methods ---

ViewModelError ViewModelError::fromApiError(const ApiError &apiError)
{
    if (apiError.isNetworkError()) {
        return ViewModelError(ErrorType::Network, apiError.userMessage(), apiError.details());
    }
    if (apiError.isAuthError()) {
        return ViewModelError(ErrorType::Auth, apiError.userMessage(), apiError.details());
    }
    if (apiError.isRateLimitError()) {
        return ViewModelError(ErrorType::RateLimit, apiError.userMessage(), apiError.details());
    }
    if (apiError.isNotFoundError()) {
        return ViewModelError(ErrorType::NotFound, apiError.userMessage(), apiError.details());
    }
    return ViewModelError(ErrorType::Api, apiError.userMessage(), apiError.details());
}

ViewModelError ViewModelError::network(const QString &message)
{
    return ViewModelError(ErrorType::Network, message);
}

ViewModelError ViewModelError::database(const QString &message)
{
    return ViewModelError(ErrorType::Database, message);
}

ViewModelError ViewModelError::validation(const QString &message)
{
    return ViewModelError(ErrorType::Validation, message);
}

// --- Accessors ---

ViewModelError::ErrorType ViewModelError::type() const
{
    return m_type;
}

const QString &ViewModelError::message() const
{
    return m_message;
}

const QString &ViewModelError::details() const
{
    return m_details;
}

bool ViewModelError::canRetry() const
{
    switch (m_type) {
        case ErrorType::Network:
        case ErrorType::RateLimit:
        case ErrorType::Api:
            return true;
        case ErrorType::Auth:
        case ErrorType::NotFound:
        case ErrorType::Database:
        case ErrorType::Validation:
        case ErrorType::Unknown:
            return false;
    }
    return false;
}

bool ViewModelError::isAuthError() const
{
    return m_type == ErrorType::Auth;
}

bool ViewModelError::isNetworkError() const
{
    return m_type == ErrorType::Network;
}

} // namespace QeriPlayerQt
