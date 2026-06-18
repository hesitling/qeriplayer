/// @file SongListModel.cpp
/// @brief Implementation of SongListModel

#include "viewmodel/SongListModel.h"

namespace QeriPlayerQt {

SongListModel::SongListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

// --- QAbstractListModel interface ---

int SongListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0; // Flat list — no children
    }
    return m_songs.size();
}

QVariant SongListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_songs.size()) {
        return {};
    }

    const Song &song = m_songs.at(index.row());

    switch (role) {
        case IdRole:
            return song.id;
        case NameRole:
            return song.name;
        case ArtistRole:
            return song.artist;
        case AlbumRole:
            return song.album;
        case DurationMsRole:
            return song.durationMs;
        case CoverUrlRole:
            return song.coverUrl;
        case PlatformRole:
            return QVariant::fromValue(song.platform);
        case IsPlayingRole:
            return index.row() == m_playingIndex;
        default:
            return {};
    }
}

QHash<int, QByteArray> SongListModel::roleNames() const
{
    return {{IdRole, "id"},
            {NameRole, "name"},
            {ArtistRole, "artist"},
            {AlbumRole, "album"},
            {DurationMsRole, "durationMs"},
            {CoverUrlRole, "coverUrl"},
            {PlatformRole, "platform"},
            {IsPlayingRole, "isPlaying"}};
}

// --- Data management ---

void SongListModel::setSongs(const QVector<Song> &songs)
{
    beginResetModel();
    m_songs = songs;
    m_playingIndex = -1;
    endResetModel();

    Q_EMIT countChanged();
}

void SongListModel::appendSongs(const QVector<Song> &songs)
{
    if (songs.isEmpty()) {
        return;
    }

    const int first = m_songs.size();
    const int last = first + songs.size() - 1;

    beginInsertRows(QModelIndex(), first, last);
    m_songs.append(songs);
    endInsertRows();

    Q_EMIT countChanged();
}

void SongListModel::clear()
{
    beginResetModel();
    m_songs.clear();
    m_playingIndex = -1;
    endResetModel();

    Q_EMIT countChanged();
}

Song SongListModel::songAt(int index) const
{
    if (index < 0 || index >= m_songs.size()) {
        return {};
    }
    return m_songs.at(index);
}

QVector<Song> SongListModel::songs() const
{
    return m_songs;
}

int SongListModel::count() const
{
    return m_songs.size();
}

// --- Playing index ---

void SongListModel::setPlayingIndex(int index)
{
    if (index == m_playingIndex) {
        return;
    }

    // Validate index range (-1 means no song playing)
    if (index < -1 || index >= m_songs.size()) {
        return;
    }

    const int oldIndex = m_playingIndex;
    m_playingIndex = index;

    // Emit dataChanged for the old and new playing rows
    if (oldIndex >= 0 && oldIndex < m_songs.size()) {
        const QModelIndex oldIdx = createIndex(oldIndex, 0);
        Q_EMIT dataChanged(oldIdx, oldIdx, {IsPlayingRole});
    }
    if (index >= 0 && index < m_songs.size()) {
        const QModelIndex newIdx = createIndex(index, 0);
        Q_EMIT dataChanged(newIdx, newIdx, {IsPlayingRole});
    }
}

} // namespace QeriPlayerQt
