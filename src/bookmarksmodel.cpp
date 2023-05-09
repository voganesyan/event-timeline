#include "bookmarksmodel.h"
#include <random>
#include <ranges>
#include <chrono>
#include <QtConcurrent>

using namespace std::chrono_literals;
static constexpr long BOOKMARK_MAX_TIMESTAMP = std::chrono::milliseconds(24h).count();
static constexpr long BOOKMARK_MAX_DURATION = std::chrono::milliseconds(3h).count();


static long rand(long max)
{
    thread_local std::mt19937 generator;
    std::uniform_int_distribution<long> distribution(0, max);
    return distribution(generator);
}


static BookmarksVector generate_bookmarks(int amount)
{
    BookmarksVector bookmarks;
    bookmarks.reserve(amount);
    for (int i = 0; i < amount; i++) {
        long timestamp = rand(BOOKMARK_MAX_TIMESTAMP);
        long duration = rand(BOOKMARK_MAX_DURATION);
        bookmarks.emplace_back(i, timestamp, duration);
    }
    std::ranges::sort(
        bookmarks,
        [] (const Bookmark &a, const Bookmark &b) {
            return a.timestamp < b.timestamp;
        }
    );
    return bookmarks;
}


BookmarksModel::BookmarksModel(QObject *parent)
    : QObject(parent)
{
    connect(&m_watcher, &QFutureWatcher<BookmarksVector>::finished,
            this, &BookmarksModel::update_bookmarks);
}


void BookmarksModel::regenerate_bookmarks(int amount)
{
    auto future = QtConcurrent::run(generate_bookmarks, amount);
    m_watcher.setFuture(future);
}


void BookmarksModel::update_bookmarks()
{
    m_bookmarks = std::move(m_watcher.result());
    emit bookmarks_changed();
}


const BookmarksVector& BookmarksModel::bookmarks() const
{
    return m_bookmarks;
}
