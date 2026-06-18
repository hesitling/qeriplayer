/// @file LoginResult.h
/// @brief Login result type

#ifndef QERIPLAYERQT_LOGINRESULT_H
#define QERIPLAYERQT_LOGINRESULT_H

#include <QMetaType>
#include <QString>
#include <QUrl>

namespace QeriPlayerQt {

/**
 * @brief Result of a successful login operation
 *
 * Contains user profile information and the session cookie string.
 * The cookie is a semicolon-delimited string of key=value pairs
 * (e.g., "MUSIC_U=xxx; __csrf=yyy") suitable for injection into HTTP Cookie headers.
 */
struct LoginResult {
    QString userId;
    QString nickname;
    QUrl avatarUrl;
    QString cookie; ///< Semicolon-delimited key=value pairs
};

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::LoginResult)

#endif // QERIPLAYERQT_LOGINRESULT_H
