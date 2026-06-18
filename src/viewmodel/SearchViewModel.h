/// @file SearchViewModel.h
/// @brief ViewModel for multi-platform music search with debounce

#ifndef QERIPLAYERQT_SEARCHVIEWMODEL_H
#define QERIPLAYERQT_SEARCHVIEWMODEL_H

#include "api/common/IMusicPlatformPlugin.h"
#include "domain/Enums.h"
#include "domain/Song.h"
#include "repo/ISongRepository.h"
#include "viewmodel/SongListModel.h"
#include "viewmodel/ViewModelError.h"

#include <QCoroTask>
#include <QObject>
#include <QTimer>
#include <QVector>

namespace QeriPlayerQt {

/**
 * @brief ViewModel for music search with platform dispatch and debounce
 *
 * Dispatches search to the IMusicPlatformPlugin matching selectedPlatform.
 * Uses a 300ms debounce timer and request versioning for race safety.
 * Caches results in ISongRepository.
 */
class SearchViewModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(
        MusicPlatform selectedPlatform READ selectedPlatform WRITE setSelectedPlatform NOTIFY selectedPlatformChanged)
    Q_PROPERTY(SongListModel *results READ results CONSTANT)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool hasMore READ hasMore NOTIFY hasMoreChanged)
    Q_PROPERTY(bool hasError READ hasError NOTIFY errorChanged)
    Q_PROPERTY(ViewModelError error READ error NOTIFY errorChanged)
    Q_PROPERTY(QVariantList availablePlatforms READ availablePlatforms CONSTANT)

public:
    /**
     * @brief Construct a SearchViewModel
     * @param plugins Available platform plugins (first is default)
     * @param songRepo Repository for caching search results
     * @param parent QObject parent
     */
    explicit SearchViewModel(QVector<IMusicPlatformPlugin *> plugins, ISongRepository *songRepo,
                             QObject *parent = nullptr);
    ~SearchViewModel() override;

    // --- Getters ---
    QString query() const;
    MusicPlatform selectedPlatform() const;
    SongListModel *results() const;
    bool isLoading() const;
    bool hasMore() const;
    bool hasError() const;
    ViewModelError error() const;
    QVariantList availablePlatforms() const;

    // --- Setters ---
    void setQuery(const QString &query);
    void setSelectedPlatform(QeriPlayerQt::MusicPlatform platform);

    // --- Actions ---
    Q_INVOKABLE void search();
    Q_INVOKABLE void loadMore();
    Q_INVOKABLE void clearResults();
    Q_INVOKABLE void clearError();

Q_SIGNALS:
    void queryChanged();
    void selectedPlatformChanged();
    void isLoadingChanged();
    void hasMoreChanged();
    void errorChanged();

    /// @brief Emitted when user selects a search result to play
    void requestPlay(const QeriPlayerQt::Song &song);

private:
    QCoro::Task<void> searchImpl();
    QCoro::Task<void> loadMoreImpl();
    IMusicPlatformPlugin *currentPlugin() const;

    QVector<IMusicPlatformPlugin *> m_plugins;
    ISongRepository *m_songRepo;
    SongListModel *m_results;
    QTimer *m_debounceTimer;

    QString m_query;
    MusicPlatform m_selectedPlatform = MusicPlatform::Unknown;
    bool m_isLoading = false;
    bool m_hasMore = false;
    ViewModelError m_error;
    bool m_hasError = false;

    quint64 m_searchRequestVersion = 0;
    int m_currentPage = 0;
    static constexpr int PAGE_SIZE = 30;
};

} // namespace QeriPlayerQt

#endif // QERIPLAYERQT_SEARCHVIEWMODEL_H
