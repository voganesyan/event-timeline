#include "bookmarksmodel.h"
#include <random>
#include <ranges>
#include <chrono>
#include <QtConcurrent>
#include <QDebug>

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

    auto t1 = std::chrono::steady_clock::now();

    for (int i = 0; i < amount; i++) {
        long timestamp = rand(BOOKMARK_MAX_TIMESTAMP);
        long duration = rand(BOOKMARK_MAX_DURATION);
        bookmarks.emplace_back(i, timestamp, duration);
    }

    auto t2 = std::chrono::steady_clock::now();

    std::ranges::sort(
        bookmarks,
        [] (const Bookmark &a, const Bookmark &b) {
            return a.timestamp < b.timestamp;
        }
    );

    auto t3 = std::chrono::steady_clock::now();

    qDebug() << "A" << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    qDebug() << "B" << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();

    return bookmarks;
}


BookmarksModel::BookmarksModel(QObject *parent)
    : QObject(parent)
{
}


void BookmarksModel::regenerate_bookmarks(int amount)
{
    connect(&watcher, &QFutureWatcher<BookmarksVector>::finished,
            this, &BookmarksModel::update_bookmarks);
    auto future = QtConcurrent::run(generate_bookmarks, amount);
    watcher.setFuture(future);
}


void BookmarksModel::update_bookmarks()
{
    m_bookmarks = std::move(watcher.result());
    emit bookmarks_changed();
}


const BookmarksVector& BookmarksModel::bookmarks() const
{
    return m_bookmarks;
}
