/// @file ViewModelError.h
/// @brief Structured error type for ViewModel layer

#ifndef QERIPLAYERQT_VIEWMODELERROR_H
#define QERIPLAYERQT_VIEWMODELERROR_H

#include "api/common/ApiError.h"

#include <QMetaType>
#include <QString>

namespace QeriPlayerQt {

/**
 * @brief Structured error type for ViewModel layer
 *
 * Wraps ApiError for API failures and adds categories for
 * database, validation, and other non-API errors.
 * Exposed as Q_GADGET for QML consumption.
 */
class ViewModelError {
    Q_GADGET
    Q_PROPERTY(ErrorType type READ type CONSTANT)
    Q_PROPERTY(QString message READ message CONSTANT)
    Q_PROPERTY(QString details READ details CONSTANT)
    Q_PROPERTY(bool canRetry READ canRetry CONSTANT)

public:
    /**
     * @brief Error type classification
     */
    enum class ErrorType : quint8 {
        Network = 0, ///< Connectivity issues
        Auth,        ///< Login expired, not authenticated
        RateLimit,   ///< Too many requests
        NotFound,    ///< Resource doesn't exist
        Api,         ///< Other API errors
        Database,    ///< SQLite errors
        Validation,  ///< User input invalid
        Unknown      ///< Unclassified
    };
    Q_ENUM(ErrorType)

    ViewModelError() = default;

    /**
     * @brief Construct a ViewModelError
     * @param type Error classification
     * @param message Human-readable error message
     * @param details Optional additional details
     */
    ViewModelError(ErrorType type, const QString &message, const QString &details = {});

    // --- Factory methods ---

    /**
     * @brief Create from ApiError, mapping classification to ErrorType
     */
    static ViewModelError fromApiError(const ApiError &apiError);

    /**
     * @brief Create a network connectivity error
     */
    static ViewModelError network(const QString &message);

    /**
     * @brief Create a database error
     */
    static ViewModelError database(const QString &message);

    /**
     * @brief Create a validation error
     */
    static ViewModelError validation(const QString &message);

    // --- Accessors ---

    ErrorType type() const;
    const QString &message() const;
    const QString &details() const;

    /**
     * @brief Check if this error is retryable
     * @return true for Network, RateLimit, and Api errors
     */
    bool canRetry() const;

    /**
     * @brief Check if this is an auth error
     */
    bool isAuthError() const;

    /**
     * @brief Check if this is a network error
     */
    bool isNetworkError() const;

private:
    ErrorType m_type = ErrorType::Unknown;
    QString m_message;
    QString m_details;
};

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::ViewModelError)

#endif // QERIPLAYERQT_VIEWMODELERROR_H
