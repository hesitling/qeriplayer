/// @file SqlRowMapper.cpp
/// @brief Implementation of SQL↔domain type conversions

#include "repo/SqlRowMapper.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace QeriPlayerQt {

// ── helpers ──────────────────────────────────────────────────────────────────

QString SqlRowMapper::platformToString(MusicPlatform p)
{
    switch (p) {
        case MusicPlatform::NetEase:
            return QStringLiteral("NetEase");
        case MusicPlatform::Bilibili:
            return QStringLiteral("Bilibili");
        case MusicPlatform::YouTube:
            return QStringLiteral("YouTube");
        case MusicPlatform::QQMusic:
            return QStringLiteral("QQMusic");
        default:
            return QStringLiteral("Unknown");
    }
}

MusicPlatform SqlRowMapper::stringToPlatform(const QString &s)
{
    if (s == "NetEase")
        return MusicPlatform::NetEase;
    if (s == "Bilibili")
        return MusicPlatform::Bilibili;
    if (s == "YouTube")
        return MusicPlatform::YouTube;
    if (s == "QQMusic")
        return MusicPlatform::QQMusic;
    return MusicPlatform::Unknown;
}

static QVariant urlToVariant(const QUrl &url)
{
    if (url.isEmpty())
        return QVariant();
    return url.toString();
}

static QUrl variantToUrl(const QVariant &v)
{
    if (v.isNull() || !v.isValid())
        return QUrl();
    return QUrl(v.toString());
}

static QString lyricLineToJson(const LyricLine &line)
{
    QJsonObject obj;
    obj["s"] = line.startTimeMs;
    obj["e"] = line.endTimeMs;
    obj["t"] = line.text;
    if (!line.words.isEmpty()) {
        QJsonArray wordsArr;
        for (const auto &w : line.words) {
            QJsonObject wo;
            wo["t"] = w.text;
            wo["s"] = w.startTimeMs;
            wo["e"] = w.endTimeMs;
            wordsArr.append(wo);
        }
        obj["w"] = wordsArr;
    }
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

static LyricLine jsonToLyricLine(const QJsonObject &obj)
{
    LyricLine line;
    line.startTimeMs = obj["s"].toInteger();
    line.endTimeMs = obj["e"].toInteger();
    line.text = obj["t"].toString();
    if (obj.contains("w")) {
        QJsonArray wordsArr = obj["w"].toArray();
        for (const auto &wv : wordsArr) {
            QJsonObject wo = wv.toObject();
            LyricWord lw;
            lw.text = wo["t"].toString();
            lw.startTimeMs = wo["s"].toInteger();
            lw.endTimeMs = wo["e"].toInteger();
            line.words.append(lw);
        }
    }
    return line;
}

static QString lyricsToJson(const Song &song)
{
    QJsonObject root;

    if (!song.lyrics.rawText.isEmpty()) {
        root["raw"] = song.lyrics.rawText;
    }
    if (!song.lyrics.lines.isEmpty()) {
        QJsonArray linesArr;
        for (const auto &line : song.lyrics.lines) {
            QJsonObject lineObj;
            lineObj["s"] = line.startTimeMs;
            lineObj["e"] = line.endTimeMs;
            lineObj["t"] = line.text;
            if (!line.words.isEmpty()) {
                QJsonArray wordsArr;
                for (const auto &w : line.words) {
                    QJsonObject wo;
                    wo["t"] = w.text;
                    wo["s"] = w.startTimeMs;
                    wo["e"] = w.endTimeMs;
                    wordsArr.append(wo);
                }
                lineObj["w"] = wordsArr;
            }
            linesArr.append(lineObj);
        }
        root["lines"] = linesArr;
    }
    if (!song.matchedLyric.isEmpty()) {
        root["ml"] = song.matchedLyric;
    }
    if (!song.matchedTranslatedLyric.isEmpty()) {
        root["mtl"] = song.matchedTranslatedLyric;
    }

    if (root.isEmpty())
        return QString();
    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

static void jsonToLyrics(const QString &json, Song &song)
{
    if (json.isEmpty())
        return;

    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isObject())
        return;
    QJsonObject root = doc.object();

    song.lyrics.rawText = root["raw"].toString();

    if (root.contains("lines")) {
        QJsonArray linesArr = root["lines"].toArray();
        for (const auto &lv : linesArr) {
            song.lyrics.lines.append(jsonToLyricLine(lv.toObject()));
        }
    }

    song.matchedLyric = root["ml"].toString();
    song.matchedTranslatedLyric = root["mtl"].toString();
}

static QString extraToJson(const QVariantMap &extra)
{
    if (extra.isEmpty())
        return QString();
    QJsonObject obj = QJsonObject::fromVariantMap(extra);
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

static QVariantMap jsonToExtra(const QString &json)
{
    if (json.isEmpty())
        return {};
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isObject())
        return {};
    return doc.object().toVariantMap();
}

// ── column indices for SELECT * FROM songs_cache ─────────────────────────────
// Matches the column order in applyInitialSchema():
//   0  id                   10 custom_artist
//   1  platform             11 custom_cover_url
//   2  name                 12 original_name
//   3  artist               13 original_artist
//   4  album                14 original_cover_url
//   5  album_id             15 local_file_name
//   6  duration_ms          16 local_file_path
//   7  cover_url            17 matched_lyric_source
//   8  media_uri            18 matched_song_id
//   9  custom_name          19 user_lyric_offset_ms
//  20  lyrics_json          24 extra_json
//  21  channel_id           25 cached_at
//  22  audio_id             26 last_played_at
//  23  sub_audio_id

Song SqlRowMapper::toSong(const QueryRow &row)
{
    if (row.size() < 25)
        return {};

    Song s;
    s.id = row[0].toString();
    s.platform = stringToPlatform(row[1].toString());
    s.name = row[2].toString();
    s.artist = row[3].toString();
    s.album = row[4].toString();
    s.albumId = row[5].toString();
    s.durationMs = row[6].toLongLong();
    s.coverUrl = variantToUrl(row[7]);
    s.mediaUri = variantToUrl(row[8]);
    s.customName = row[9].toString();
    s.customArtist = row[10].toString();
    s.customCoverUrl = row[11].toString();
    s.originalName = row[12].toString();
    s.originalArtist = row[13].toString();
    s.originalCoverUrl = row[14].toString();
    s.localFileName = row[15].toString();
    s.localFilePath = row[16].toString();
    s.matchedLyricSource = stringToPlatform(row[17].toString());
    s.matchedSongId = row[18].toString();
    s.userLyricOffsetMs = row[19].toLongLong();

    jsonToLyrics(row[20].toString(), s);

    s.channelId = row[21].toString();
    s.audioId = row[22].toString();
    s.subAudioId = row[23].toString();
    s.extra = jsonToExtra(row[24].toString());

    return s;
}

// INSERT columns (25): id through extra_json (excluding cached_at, last_played_at)
QVariantList SqlRowMapper::songToInsertParams(const Song &s)
{
    return {
        s.id,                                   // 1
        platformToString(s.platform),           // 2
        s.name,                                 // 3
        s.artist,                               // 4
        s.album,                                // 5
        s.albumId,                              // 6
        s.durationMs,                           // 7
        urlToVariant(s.coverUrl),               // 8
        urlToVariant(s.mediaUri),               // 9
        s.customName,                           // 10
        s.customArtist,                         // 11
        s.customCoverUrl,                       // 12
        s.originalName,                         // 13
        s.originalArtist,                       // 14
        s.originalCoverUrl,                     // 15
        s.localFileName,                        // 16
        s.localFilePath,                        // 17
        platformToString(s.matchedLyricSource), // 18
        s.matchedSongId,                        // 19
        s.userLyricOffsetMs,                    // 20
        lyricsToJson(s),                        // 21
        s.channelId,                            // 22
        s.audioId,                              // 23
        s.subAudioId,                           // 24
        extraToJson(s.extra),                   // 25
    };
}

// UPDATE: same 25 columns, but id moves to the end (for WHERE clause)
QVariantList SqlRowMapper::songToUpdateParams(const Song &s)
{
    return {
        platformToString(s.platform),           // SET platform
        s.name,                                 // SET name
        s.artist,                               // SET artist
        s.album,                                // SET album
        s.albumId,                              // SET album_id
        s.durationMs,                           // SET duration_ms
        urlToVariant(s.coverUrl),               // SET cover_url
        urlToVariant(s.mediaUri),               // SET media_uri
        s.customName,                           // SET custom_name
        s.customArtist,                         // SET custom_artist
        s.customCoverUrl,                       // SET custom_cover_url
        s.originalName,                         // SET original_name
        s.originalArtist,                       // SET original_artist
        s.originalCoverUrl,                     // SET original_cover_url
        s.localFileName,                        // SET local_file_name
        s.localFilePath,                        // SET local_file_path
        platformToString(s.matchedLyricSource), // SET matched_lyric_source
        s.matchedSongId,                        // SET matched_song_id
        s.userLyricOffsetMs,                    // SET user_lyric_offset_ms
        lyricsToJson(s),                        // SET lyrics_json
        s.channelId,                            // SET channel_id
        s.audioId,                              // SET audio_id
        s.subAudioId,                           // SET sub_audio_id
        extraToJson(s.extra),                   // SET extra_json
        s.id,                                   // WHERE id
    };
}

PlaylistSummary SqlRowMapper::toPlaylistSummary(const QueryRow &row)
{
    PlaylistSummary ps;
    if (row.size() < 6)
        return ps;

    ps.id = row[0].toString(); // id
    // row[1] = platform
    ps.name = row[2].toString(); // name
    // row[3] = description
    ps.coverUrl = variantToUrl(row[4]); // cover_url
    ps.trackCount = row[5].toInt();     // song_count

    return ps;
}

QVariantMap SqlRowMapper::playerStateToJson(const PersistedPlayerState &state)
{
    QVariantMap map;
    map["current_index"] = state.currentIndex;
    map["media_url"] = state.mediaUrl;
    map["position_ms"] = state.positionMs;
    map["should_resume"] = state.shouldResumePlayback;
    map["repeat_mode"] = static_cast<int>(state.repeatMode);
    map["shuffle_enabled"] = state.shuffleEnabled;

    // Serialize playlist as JSON array
    QJsonArray arr;
    for (const auto &song : state.playlist) {
        QJsonObject obj;
        obj["id"] = song.id;
        obj["plat"] = platformToString(song.platform);
        obj["name"] = song.name;
        obj["art"] = song.artist;
        obj["alb"] = song.album;
        obj["albid"] = song.albumId;
        obj["dur"] = song.durationMs;
        obj["cov"] = song.coverUrl.toString();
        obj["uri"] = song.mediaUri.toString();
        obj["cst"] = song.customCoverUrl;
        obj["csn"] = song.customName;
        obj["csa"] = song.customArtist;
        obj["lon"] = song.localFileName;
        obj["lop"] = song.localFilePath;
        obj["mls"] = platformToString(song.matchedLyricSource);
        obj["msid"] = song.matchedSongId;
        obj["ulo"] = song.userLyricOffsetMs;
        obj["chn"] = song.channelId;
        obj["aid"] = song.audioId;
        obj["said"] = song.subAudioId;
        if (!song.lyrics.rawText.isEmpty() || !song.lyrics.lines.isEmpty()) {
            obj["lrc"] = lyricsToJson(song);
        }
        if (!song.extra.isEmpty()) {
            obj["ext"] = extraToJson(song.extra);
        }
        arr.append(obj);
    }
    map["playlist_json"] = QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));

    return map;
}

PersistedPlayerState SqlRowMapper::playerStateFromJson(const QVariantMap &map)
{
    PersistedPlayerState state;
    state.currentIndex = map.value("current_index", 0).toInt();
    state.mediaUrl = map.value("media_url").toString();
    state.positionMs = map.value("position_ms", 0).toLongLong();
    state.shouldResumePlayback = map.value("should_resume", false).toBool();
    state.repeatMode = static_cast<RepeatMode>(map.value("repeat_mode", 0).toInt());
    state.shuffleEnabled = map.value("shuffle_enabled", false).toBool();

    QString playlistStr = map.value("playlist_json").toString();
    if (!playlistStr.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(playlistStr.toUtf8());
        if (doc.isArray()) {
            for (const auto &v : doc.array()) {
                QJsonObject obj = v.toObject();
                Song song;
                song.id = obj["id"].toString();
                song.platform = stringToPlatform(obj["plat"].toString());
                song.name = obj["name"].toString();
                song.artist = obj["art"].toString();
                song.album = obj["alb"].toString();
                song.albumId = obj["albid"].toString();
                song.durationMs = obj["dur"].toInteger();
                song.coverUrl = QUrl(obj["cov"].toString());
                song.mediaUri = QUrl(obj["uri"].toString());
                song.customCoverUrl = obj["cst"].toString();
                song.customName = obj["csn"].toString();
                song.customArtist = obj["csa"].toString();
                song.localFileName = obj["lon"].toString();
                song.localFilePath = obj["lop"].toString();
                song.matchedLyricSource = stringToPlatform(obj["mls"].toString());
                song.matchedSongId = obj["msid"].toString();
                song.userLyricOffsetMs = obj["ulo"].toInteger();
                song.channelId = obj["chn"].toString();
                song.audioId = obj["aid"].toString();
                song.subAudioId = obj["said"].toString();
                if (obj.contains("lrc")) {
                    jsonToLyrics(obj["lrc"].toString(), song);
                }
                if (obj.contains("ext")) {
                    song.extra = jsonToExtra(obj["ext"].toString());
                }
                state.playlist.append(song);
            }
        }
    }

    return state;
}

} // namespace QeriPlayerQt
