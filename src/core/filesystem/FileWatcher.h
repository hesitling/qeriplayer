/// @file FileWatcher.h
/// @brief File change monitoring using QFileSystemWatcher
/// @date 2024-01-15

#ifndef NERIPLAYERQT_FILEWATCHER_H
#define NERIPLAYERQT_FILEWATCHER_H

#include <QFileSystemWatcher>
#include <QObject>
#include <QString>

namespace NeriPlayerQt {

/**
 * @brief Monitors a file or directory for changes
 */
class FileWatcher : public QObject {
    Q_OBJECT

public:
    explicit FileWatcher(QObject *parent = nullptr);

    /**
     * @brief Start watching a path
     * @param path File or directory to watch
     */
    void watch(const QString &path);

    /**
     * @brief Stop watching all paths
     */
    void stop();

    /**
     * @brief Check if currently watching
     */
    bool isWatching() const;

Q_SIGNALS:
    /**
     * @brief Emitted when a watched file changes
     * @param path Path that changed
     */
    void fileChanged(const QString &path);

private:
    QFileSystemWatcher m_watcher;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_FILEWATCHER_H
