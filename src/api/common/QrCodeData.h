/// @file QrCodeData.h
/// @brief QR code login data type

#ifndef QERIPLAYERQT_QRCODEDATA_H
#define QERIPLAYERQT_QRCODEDATA_H

#include <QMetaType>
#include <QString>
#include <QUrl>

namespace QeriPlayerQt {

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

} // namespace QeriPlayerQt

Q_DECLARE_METATYPE(QeriPlayerQt::QrCodeData)

#endif // QERIPLAYERQT_QRCODEDATA_H
