/// @file ApiError.h
/// @brief API error type with code classification

#ifndef QERIPLAYERQT_APIERROR_H
#define QERIPLAYERQT_APIERROR_H

#include <QMetaType>
#include <QString>

namespace QeriPlayerQt {

/**
 * @brief Represents an error from a music platform API
 *
 * Stores an integer error code (HTTP status or platform-specific body code),
 * a human-readable message, and optional details. Provides classification
 * methods for common error categories.
 */
class ApiError {
public:
    ApiError() = default;

    /**
     * @brief Construct an API error
     * @param code Error code (HTTP status or platform body code)
     * @param message Human-readable error message
     * @param details Optional additional details
     */
    ApiError(int code, const QString &message, const QString &details = {});

    int code() const;
    const QString &message() const;
    const QString &details() const;

    /**
     * @brief Check if this is a network connectivity error (code == -1)
     */
    bool isNetworkError() const;

    /**
     * @brief Check if this is an authentication error
     *
     * Matches HTTP 401/403 and platform-specific codes:
     * - NetEase: -10 (auth expired), -460 (cheating detection)
     */
    bool isAuthError() const;

    /**
     * @brief Check if this is a rate limit error
     *
     * Matches HTTP 429 and platform-specific code -429
     */
    bool isRateLimitError() const;

    /**
     * @brief Check if this is a not-found error (HTTP 404)
     */
    bool isNotFoundError() const;

    /**
     * @brief Get a user-facing error message suitable for UI display
     */
    QString userMessage() const;

private:
    int m_code = 0;
    QString m_message;
    QString m_details;
};

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::ApiError)

#endif // QERIPLAYERQT_APIERROR_H
