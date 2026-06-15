/// @file QrCodeData.h
/// @brief QR code login data type

#ifndef NERIPLAYERQT_QRCODEDATA_H
#define NERIPLAYERQT_QRCODEDATA_H

#include <QMetaType>
#include <QString>
#include <QUrl>

namespace NeriPlayerQt {

/**
 * @brief Data for QR code-based login
 *
 * Contains the key used to poll for scan status, the QR code URL
 * (image or data URL for display), and the expiration time.
 */
struct QrCodeData {
    QString key;              ///< Unique key for polling scan status
    QUrl qrUrl;               ///< QR code image or data URL
    int expiresInSeconds = 0; ///< Seconds until the QR code expires
};

} // namespace NeriPlayerQt

Q_DECLARE_METATYPE(NeriPlayerQt::QrCodeData)

#endif // NERIPLAYERQT_QRCODEDATA_H
