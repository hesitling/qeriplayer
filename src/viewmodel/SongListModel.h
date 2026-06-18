/// @file SongListModel.h
/// @brief QAbstractListModel exposing QVector<Song> to QML

#ifndef QERIPLAYERQT_SONGLISTMODEL_H
#define QERIPLAYERQT_SONGLISTMODEL_H

#include "domain/Song.h"

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QVector>

namespace QeriPlayerQt {

/**
 * @brief QAbstractListModel adapter for QVector<Song>
 *
 * Exposes song properties via Qt model roles for QML ListView binding.
 * Owned by the ViewModel that creates it (parented QObject).
 */
class SongListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    /**
     * @brief Model roles for song data
     */
    enum Roles {
        IdRole = Qt::UserRole + 1, ///< Song.id (QString)
        NameRole,                  ///< Song.name (QString)
        ArtistRole,                ///< Song.artist (QString)
        AlbumRole,                 ///< Song.album (QString)
        DurationMsRole,            ///< Song.durationMs (qint64)
        CoverUrlRole,              ///< Song.coverUrl (QUrl)
        PlatformRole,              ///< Song.platform (MusicPlatform)
        IsPlayingRole              ///< Whether this row is the currently playing song (bool)
    };

    explicit SongListModel(QObject *parent = nullptr);

    // --- QAbstractListModel interface ---

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // --- Data management ---

    /**
     * @brief Replace all songs in the model
     */
    void setSongs(const QVector<Song> &songs);

    /**
     * @brief Append songs to the end of the model
     */
    void appendSongs(const QVector<Song> &songs);

    /**
     * @brief Remove all songs from the model
     */
    void clear();

    /**
     * @brief Get the song at the given index
     * @return Song at index, or default Song if out of bounds
     */
    Song songAt(int index) const;

    /**
     * @brief Get all songs in the model
     */
    QVector<Song> songs() const;

    /**
     * @brief Get the number of songs in the model
     */
    int count() const;

    // --- Playing index ---

    /**
     * @brief Set the index of the currently playing song
     *
     * Emits dataChanged for the old and new playing rows on IsPlayingRole.
     */
    void setPlayingIndex(int index);

Q_SIGNALS:
    void countChanged();
    void songActivated(int index);

private:
    QVector<Song> m_songs;
    int m_playingIndex = -1;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_SONGLISTMODEL_H
