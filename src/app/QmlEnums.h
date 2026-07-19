/// @file QmlEnums.h
/// @brief Named enum values exposed to QML

#ifndef QERIPLAYERQT_QMLENUMS_H
#define QERIPLAYERQT_QMLENUMS_H

#include "domain/Enums.h"

#include <QObject>
#include <QtTypes>

namespace QeriPlayerQt {

/// @brief QML representation of MusicPlatform.
class QmlMusicPlatform {
    Q_GADGET

public:
    enum Value : quint8 {
        Unknown = static_cast<quint8>(MusicPlatform::Unknown),
        NetEase = static_cast<quint8>(MusicPlatform::NetEase),
        Bilibili = static_cast<quint8>(MusicPlatform::Bilibili),
        YouTube = static_cast<quint8>(MusicPlatform::YouTube),
        QQMusic = static_cast<quint8>(MusicPlatform::QQMusic),
    };
    Q_ENUM(Value)
};

/// @brief QML representation of RepeatMode.
class QmlRepeatMode {
    Q_GADGET

public:
    enum Value : quint8 {
        Off = static_cast<quint8>(RepeatMode::Off),
        One = static_cast<quint8>(RepeatMode::One),
        All = static_cast<quint8>(RepeatMode::All),
    };
    Q_ENUM(Value)
};

/// @brief QML representation of AudioQuality.
class QmlAudioQuality {
    Q_GADGET

public:
    enum Value : quint8 {
        Low = static_cast<quint8>(AudioQuality::Low),
        Standard = static_cast<quint8>(AudioQuality::Standard),
        High = static_cast<quint8>(AudioQuality::High),
        Lossless = static_cast<quint8>(AudioQuality::Lossless),
    };
    Q_ENUM(Value)
};

/// @brief QML representation of MainViewModel::View.
class QmlMainView {
    Q_GADGET

public:
    enum Value : quint8 { Home, Search, Library, LocalPlaylist, NeteasePlaylist, Settings };
    Q_ENUM(Value)
};

/// @brief Register enum-only types provided by the QeriPlayer QML module.
void registerQmlEnums();

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_QMLENUMS_H
