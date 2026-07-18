/// @file QmlEnums.cpp
/// @brief QML enum type registration

#include "app/QmlEnums.h"

#include <QString>
#include <QtQml/qqml.h>

namespace QeriPlayerQt {

void registerQmlEnums()
{
    qmlRegisterUncreatableType<QmlMusicPlatform>("QeriPlayer", 1, 0, "MusicPlatform",
                                                  QStringLiteral("MusicPlatform only provides enum values"));
    qmlRegisterUncreatableType<QmlRepeatMode>("QeriPlayer", 1, 0, "RepeatMode",
                                               QStringLiteral("RepeatMode only provides enum values"));
    qmlRegisterUncreatableType<QmlAudioQuality>("QeriPlayer", 1, 0, "AudioQuality",
                                                 QStringLiteral("AudioQuality only provides enum values"));
    qmlRegisterUncreatableType<QmlMainView>("QeriPlayer", 1, 0, "MainView",
                                            QStringLiteral("MainView only provides enum values"));
}

} // namespace QeriPlayerQt
