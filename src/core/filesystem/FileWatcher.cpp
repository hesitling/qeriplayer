/// @file FileWatcher.cpp
/// @brief File change monitoring implementation
/// @date 2024-01-15

#include "core/filesystem/FileWatcher.h"

namespace NeriPlayerQt {

FileWatcher::FileWatcher(QObject *parent)
    : QObject(parent)
{
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &FileWatcher::fileChanged);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &FileWatcher::fileChanged);
}

void FileWatcher::watch(const QString &path)
{
    m_watcher.addPath(path);
}

void FileWatcher::stop()
{
    m_watcher.removePaths(m_watcher.files());
    m_watcher.removePaths(m_watcher.directories());
}

bool FileWatcher::isWatching() const
{
    return !m_watcher.files().isEmpty() || !m_watcher.directories().isEmpty();
}

} // namespace NeriPlayerQt
