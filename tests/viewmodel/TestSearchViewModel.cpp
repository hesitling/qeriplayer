/// @file TestSearchViewModel.cpp
/// @brief Unit tests for SearchViewModel with mocked plugin

#include "api/common/ApiResult.h"
#include "api/common/IMusicPlatformPlugin.h"
#include "domain/SearchResult.h"
#include "domain/Song.h"
#include "repo/ISongRepository.h"
#include "viewmodel/SearchViewModel.h"

#include <QCoroTask>
#include <QSignalSpy>
#include <QTest>

using namespace QeriPlayerQt;

// --- Mock IMusicPlatformPlugin ---

class MockPlugin : public IMusicPlatformPlugin {
public:
    QCoro::Task<ApiResult<SearchResult>> search(const QString &keyword, SearchType, int limit, int offset) override
    {
        m_lastKeyword = keyword;
        m_lastLimit = limit;
        m_lastOffset = offset;
        m_searchCount++;

        if (m_shouldFail) {
            co_return ApiResult<SearchResult>(ApiError(500, "Mock error"));
        }

        SearchResult result;
        for (int i = 0; i < m_resultCount && i < limit; ++i) {
            Song song;
            song.id = QString::number(offset + i);
            song.name = QString("Song %1").arg(offset + i);
            song.artist = "Artist";
            result.songs.append(song);
        }
        result.hasMore = m_hasMore;
        result.totalCount = m_totalCount;
        co_return ApiResult<SearchResult>(result);
    }

    QCoro::Task<ApiResult<Song>> getSongDetail(const QString &) override
    {
        co_return ApiResult<Song>(Song {});
    }
    QCoro::Task<ApiResult<SongUrlResult>> getSongUrl(const QString &, AudioQuality) override
    {
        co_return ApiResult<SongUrlResult>(SongUrlResult {});
    }
    QCoro::Task<ApiResult<Lyrics>> getLyrics(const QString &) override
    {
        co_return ApiResult<Lyrics>(Lyrics {});
    }
    bool isAuthenticated() const override
    {
        return true;
    }
    QString platformName() const override
    {
        return QStringLiteral("Mock");
    }

    // Test control
    int m_resultCount = 10;
    bool m_hasMore = false;
    int m_totalCount = 10;
    bool m_shouldFail = false;

    // Tracking
    QString m_lastKeyword;
    int m_lastLimit = 0;
    int m_lastOffset = 0;
    int m_searchCount = 0;
};

// --- Mock ISongRepository ---

class MockSongRepo : public ISongRepository {
public:
    std::optional<Song> findById(const QString &) override
    {
        return std::nullopt;
    }
    QVector<Song> findByIds(const QStringList &) override
    {
        return {};
    }
    void save(const Song &) override { }
    void saveBatch(const QVector<Song> &songs) override
    {
        m_savedBatches.append(songs);
    }
    void remove(const QString &) override { }
    bool exists(const QString &) override
    {
        return false;
    }
    QVector<Song> findByPlatform(MusicPlatform) override
    {
        return {};
    }
    QVector<Song> search(const QString &, int) override
    {
        return {};
    }

    QVector<QVector<Song>> m_savedBatches;
};

// --- Test class ---

class TestSearchViewModel : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initialState();
    void setQuery_startsDebounce();
    void setQuery_empty_clearsResults();
    void search_dispatchesToPlugin();
    void search_setsLoadingState();
    void search_populatesResults();
    void search_cachesResults();
    void search_failure_setsError();
    void search_requestVersioning_discardStale();
    void loadMore_appendsResults();
    void loadMore_respectsHasMore();
    void clearResults_emptiesModel();
    void clearError_resetsError();
    void setSelectedPlatform_researches();
};

void TestSearchViewModel::initialState()
{
    MockPlugin plugin;
    MockSongRepo repo;
    SearchViewModel vm({&plugin}, &repo);

    QVERIFY(vm.query().isEmpty());
    QCOMPARE(vm.results()->count(), 0);
    QVERIFY(!vm.isLoading());
    QVERIFY(!vm.hasMore());
    QVERIFY(!vm.hasError());
}

void TestSearchViewModel::setQuery_startsDebounce()
{
    MockPlugin plugin;
    MockSongRepo repo;
    SearchViewModel vm({&plugin}, &repo);

    QSignalSpy spy(&vm, &SearchViewModel::queryChanged);
    vm.setQuery("hello");

    QCOMPARE(vm.query(), QStringLiteral("hello"));
    QCOMPARE(spy.count(), 1);
}

void TestSearchViewModel::setQuery_empty_clearsResults()
{
    MockPlugin plugin;
    MockSongRepo repo;
    SearchViewModel vm({&plugin}, &repo);

    vm.setQuery("hello");
    vm.clearResults();
    QCOMPARE(vm.results()->count(), 0);

    vm.setQuery("");
    QCOMPARE(vm.results()->count(), 0);
}

void TestSearchViewModel::search_dispatchesToPlugin()
{
    MockPlugin plugin;
    MockSongRepo repo;
    SearchViewModel vm({&plugin}, &repo);

    vm.setQuery("rock");
    vm.search();

    QCOMPARE(plugin.m_lastKeyword, QStringLiteral("rock"));
    QCOMPARE(plugin.m_lastLimit, 30);
    QCOMPARE(plugin.m_lastOffset, 0);
}

void TestSearchViewModel::search_setsLoadingState()
{
    MockPlugin plugin;
    MockSongRepo repo;
    SearchViewModel vm({&plugin}, &repo);

    QSignalSpy loadingSpy(&vm, &SearchViewModel::isLoadingChanged);

    vm.setQuery("rock");
    vm.search();

    // isLoading should be true during search
    // Note: since search is async, we can't check intermediate state easily
    // But we can verify the signal was emitted
    QVERIFY(loadingSpy.count() >= 1);
}

void TestSearchViewModel::search_populatesResults()
{
    MockPlugin plugin;
    plugin.m_resultCount = 5;
    MockSongRepo repo;
    SearchViewModel vm({&plugin}, &repo);

    vm.setQuery("rock");
    vm.search();

    QCOMPARE(plugin.m_searchCount, 1);
    QCOMPARE(vm.results()->count(), 5);
}

void TestSearchViewModel::search_cachesResults()
{
    MockPlugin plugin;
    plugin.m_resultCount = 3;
    MockSongRepo repo;
    SearchViewModel vm({&plugin}, &repo);

    vm.setQuery("rock");
    vm.search();

    QCOMPARE(repo.m_savedBatches.size(), 1);
    QCOMPARE(repo.m_savedBatches.first().size(), 3);
}

void TestSearchViewModel::search_failure_setsError()
{
    MockPlugin plugin;
    plugin.m_shouldFail = true;
    MockSongRepo repo;
    SearchViewModel vm({&plugin}, &repo);

    vm.setQuery("rock");
    vm.search();

    QVERIFY(vm.hasError());
    QCOMPARE(vm.error().type(), ViewModelError::ErrorType::Api);
}

void TestSearchViewModel::search_requestVersioning_discardStale()
{
    MockPlugin plugin;
    plugin.m_resultCount = 5;
    MockSongRepo repo;
    SearchViewModel vm({&plugin}, &repo);

    // First search
    vm.setQuery("rock");
    vm.search();

    // Second search before first completes
    vm.setQuery("pop");
    vm.search();

    // Only the second search should have results
    QCOMPARE(plugin.m_searchCount, 2);
}

void TestSearchViewModel::loadMore_appendsResults()
{
    MockPlugin plugin;
    plugin.m_resultCount = 10;
    plugin.m_hasMore = true;
    MockSongRepo repo;
    SearchViewModel vm({&plugin}, &repo);

    vm.setQuery("rock");
    vm.search();
    QCOMPARE(vm.results()->count(), 10);

    plugin.m_resultCount = 5;
    plugin.m_hasMore = false;
    vm.loadMore();

    QCOMPARE(vm.results()->count(), 15);
    QCOMPARE(plugin.m_lastOffset, 30); // PAGE_SIZE = 30, page 1 = offset 30
}

void TestSearchViewModel::loadMore_respectsHasMore()
{
    MockPlugin plugin;
    plugin.m_resultCount = 10;
    plugin.m_hasMore = false;
    MockSongRepo repo;
    SearchViewModel vm({&plugin}, &repo);

    vm.setQuery("rock");
    vm.search();

    int countBefore = vm.results()->count();
    vm.loadMore();

    // Should not have called the plugin again
    QCOMPARE(plugin.m_searchCount, 1);
    QCOMPARE(vm.results()->count(), countBefore);
}

void TestSearchViewModel::clearResults_emptiesModel()
{
    MockPlugin plugin;
    plugin.m_resultCount = 10;
    MockSongRepo repo;
    SearchViewModel vm({&plugin}, &repo);

    vm.setQuery("rock");
    vm.search();

    QSignalSpy hasMoreSpy(&vm, &SearchViewModel::hasMoreChanged);
    vm.clearResults();

    QCOMPARE(vm.results()->count(), 0);
    QVERIFY(!vm.hasMore());
}

void TestSearchViewModel::clearError_resetsError()
{
    MockPlugin plugin;
    plugin.m_shouldFail = true;
    MockSongRepo repo;
    SearchViewModel vm({&plugin}, &repo);

    vm.setQuery("rock");
    vm.search();

    QVERIFY(vm.hasError());

    QSignalSpy errorSpy(&vm, &SearchViewModel::errorChanged);
    vm.clearError();

    QVERIFY(!vm.hasError());
    QCOMPARE(errorSpy.count(), 1);
}

void TestSearchViewModel::setSelectedPlatform_researches()
{
    MockPlugin plugin;
    MockSongRepo repo;
    SearchViewModel vm({&plugin}, &repo);

    vm.setQuery("rock");
    vm.search();

    int countBefore = plugin.m_searchCount;

    vm.setSelectedPlatform(MusicPlatform::NetEase);

    // Should have triggered another search
    QCOMPARE(plugin.m_searchCount, countBefore + 1);
}

QTEST_MAIN(TestSearchViewModel)
#include "TestSearchViewModel.moc"
