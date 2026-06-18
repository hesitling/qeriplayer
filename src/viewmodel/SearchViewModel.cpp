/// @file SearchViewModel.cpp
/// @brief Implementation of SearchViewModel

#include "viewmodel/SearchViewModel.h"

#include "core/logger/Logger.h"

namespace QeriPlayerQt {

SearchViewModel::SearchViewModel(QVector<IMusicPlatformPlugin *> plugins, ISongRepository *songRepo, QObject *parent)
    : QObject(parent)
    , m_plugins(plugins)
    , m_songRepo(songRepo)
    , m_results(new SongListModel(this))
    , m_debounceTimer(new QTimer(this))
{
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(500);

    connect(m_debounceTimer, &QTimer::timeout, this, [this]() { m_pendingSearch = search(); });
}

SearchViewModel::~SearchViewModel() = default;

// --- Getters ---

QString SearchViewModel::query() const
{
    return m_query;
}

MusicPlatform SearchViewModel::selectedPlatform() const
{
    return m_selectedPlatform;
}

SongListModel *SearchViewModel::results() const
{
    return m_results;
}

bool SearchViewModel::isLoading() const
{
    return m_isLoading;
}

bool SearchViewModel::hasMore() const
{
    return m_hasMore;
}

bool SearchViewModel::hasError() const
{
    return m_hasError;
}

ViewModelError SearchViewModel::error() const
{
    return m_error;
}

QVariantList SearchViewModel::availablePlatforms() const
{
    QVariantList list;
    for (auto *plugin : m_plugins) {
        list.append(plugin->platformName());
    }
    return list;
}

// --- Setters ---

void SearchViewModel::setQuery(const QString &query)
{
    if (m_query == query) {
        return;
    }
    m_query = query;
    Q_EMIT queryChanged();

    if (query.isEmpty()) {
        m_debounceTimer->stop();
        clearResults();
    } else {
        m_debounceTimer->start();
    }
}

void SearchViewModel::setSelectedPlatform(MusicPlatform platform)
{
    if (m_selectedPlatform == platform) {
        return;
    }
    m_selectedPlatform = platform;
    Q_EMIT selectedPlatformChanged();

    // Clear and re-search if query is non-empty
    if (!m_query.isEmpty()) {
        clearResults();
        m_pendingSearch = search();
    }
}

// --- Actions ---

QCoro::QmlTask SearchViewModel::search()
{
    return QCoro::QmlTask(searchImpl());
}

QCoro::QmlTask SearchViewModel::loadMore()
{
    return QCoro::QmlTask(loadMoreImpl());
}

void SearchViewModel::clearResults()
{
    m_results->clear();
    m_currentPage = 0;
    m_hasMore = false;
    Q_EMIT hasMoreChanged();
}

void SearchViewModel::clearError()
{
    m_hasError = false;
    m_error = ViewModelError();
    Q_EMIT errorChanged();
}

void SearchViewModel::selectPlatformByName(const QString &name)
{
    for (auto *plugin : m_plugins) {
        if (plugin->platformName() == name) {
            // Map platform name to enum based on known names
            MusicPlatform platform = MusicPlatform::Unknown;
            if (name == QStringLiteral("NetEase")) {
                platform = MusicPlatform::NetEase;
            } else if (name == QStringLiteral("Bilibili")) {
                platform = MusicPlatform::Bilibili;
            } else if (name == QStringLiteral("YouTube")) {
                platform = MusicPlatform::YouTube;
            } else if (name == QStringLiteral("QQMusic")) {
                platform = MusicPlatform::QQMusic;
            }
            setSelectedPlatform(platform);
            return;
        }
    }
}

void SearchViewModel::playSong(int index)
{
    Song song = m_results->songAt(index);
    if (!song.id.isEmpty()) {
        Q_EMIT requestPlay(song);
    }
}

// --- Private ---

QCoro::Task<void> SearchViewModel::searchImpl()
{
    if (m_query.isEmpty()) {
        co_return;
    }

    auto *plugin = currentPlugin();
    if (!plugin) {
        co_return;
    }

    // Increment request version for race safety
    const quint64 version = ++m_searchRequestVersion;

    m_isLoading = true;
    Q_EMIT isLoadingChanged();
    clearError();

    m_currentPage = 0;

    try {
        auto result = co_await plugin->search(m_query, SearchType::Song, PAGE_SIZE, m_currentPage * PAGE_SIZE);

        // Check if this result is still current
        if (version != m_searchRequestVersion) {
            co_return;
        }

        m_isLoading = false;
        Q_EMIT isLoadingChanged();

        if (result.isError()) {
            m_error = ViewModelError::fromApiError(result.error());
            m_hasError = true;
            Q_EMIT errorChanged();
            co_return;
        }

        const auto &searchResult = result.data();
        m_results->setSongs(searchResult.songs);
        m_hasMore = searchResult.hasMore;
        Q_EMIT hasMoreChanged();

        // Cache results
        m_songRepo->saveBatch(searchResult.songs);
    } catch (const std::exception &ex) {
        if (version != m_searchRequestVersion) {
            co_return;
        }
        m_isLoading = false;
        Q_EMIT isLoadingChanged();
        m_error = ViewModelError(ViewModelError::ErrorType::Unknown, QString::fromUtf8(ex.what()));
        m_hasError = true;
        Q_EMIT errorChanged();
        Logger::get("viewmodel")->warn("Search failed: {}", ex.what());
    }
}

QCoro::Task<void> SearchViewModel::loadMoreImpl()
{
    if (m_query.isEmpty() || !m_hasMore || m_isLoading) {
        co_return;
    }

    auto *plugin = currentPlugin();
    if (!plugin) {
        co_return;
    }

    const int nextPage = m_currentPage + 1;
    const quint64 version = m_searchRequestVersion;

    m_isLoading = true;
    Q_EMIT isLoadingChanged();

    try {
        auto result = co_await plugin->search(m_query, SearchType::Song, PAGE_SIZE, nextPage * PAGE_SIZE);

        // Discard if a new search started while we were waiting
        if (version != m_searchRequestVersion) {
            co_return;
        }

        m_isLoading = false;
        Q_EMIT isLoadingChanged();

        if (result.isError()) {
            m_error = ViewModelError::fromApiError(result.error());
            m_hasError = true;
            Q_EMIT errorChanged();
            co_return;
        }

        m_currentPage = nextPage;
        const auto &searchResult = result.data();
        m_results->appendSongs(searchResult.songs);
        m_hasMore = searchResult.hasMore;
        Q_EMIT hasMoreChanged();

        // Cache results
        m_songRepo->saveBatch(searchResult.songs);
    } catch (const std::exception &ex) {
        if (version != m_searchRequestVersion) {
            co_return;
        }
        m_isLoading = false;
        Q_EMIT isLoadingChanged();
        m_error = ViewModelError(ViewModelError::ErrorType::Unknown, QString::fromUtf8(ex.what()));
        m_hasError = true;
        Q_EMIT errorChanged();
        Logger::get("viewmodel")->warn("Load more failed: {}", ex.what());
    }
}

IMusicPlatformPlugin *SearchViewModel::currentPlugin() const
{
    if (m_plugins.isEmpty()) {
        return nullptr;
    }

    // Find plugin matching selectedPlatform
    if (m_selectedPlatform != MusicPlatform::Unknown) {
        for (auto *plugin : m_plugins) {
            if (m_selectedPlatform == MusicPlatform::NetEase && plugin->platformName() == QStringLiteral("NetEase")) {
                return plugin;
            }
            if (m_selectedPlatform == MusicPlatform::Bilibili && plugin->platformName() == QStringLiteral("Bilibili")) {
                return plugin;
            }
            if (m_selectedPlatform == MusicPlatform::YouTube && plugin->platformName() == QStringLiteral("YouTube")) {
                return plugin;
            }
            if (m_selectedPlatform == MusicPlatform::QQMusic && plugin->platformName() == QStringLiteral("QQMusic")) {
                return plugin;
            }
        }
    }

    // Default to first plugin
    return m_plugins.first();
}

} // namespace QeriPlayerQt
