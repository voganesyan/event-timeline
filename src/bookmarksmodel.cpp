#include "bookmarksmodel.h"
#include <random>
#include <ranges>
#include <chrono>
#include <QDebug>

using namespace std::chrono_literals;
static constexpr long BOOKMARK_MAX_TIMESTAMP = std::chrono::milliseconds(24h).count();
static constexpr long BOOKMARK_MAX_DURATION = std::chrono::milliseconds(3h).count();


BookmarksModel::BookmarksModel(QObject *parent)
    : QObject(parent)
{
}


static long rand(long max)
{
    thread_local std::mt19937 generator;
    std::uniform_int_distribution<long> distribution(0, max);
    return distribution(generator);
}


void BookmarksModel::generate_bookmarks(int amount)
{
    m_bookmarks.clear();
    m_bookmarks.reserve(amount);

    auto t1 = std::chrono::steady_clock::now();

    for (int i = 0; i < amount; i++) {
        auto name = QString("Bookmark %1").arg(i);
        m_bookmarks.emplace_back(name, timestamp, duration);
        long timestamp = rand(BOOKMARK_MAX_TIMESTAMP);
        long duration = rand(BOOKMARK_MAX_DURATION);
    }

    auto t2 = std::chrono::steady_clock::now();

    std::ranges::sort(
        m_bookmarks,
        [] (const Bookmark &a, const Bookmark &b) {
            return a.timestamp < b.timestamp;
        }
    );

    auto t3 = std::chrono::steady_clock::now();

    qDebug() << "A" << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    qDebug() << "B" << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();

    emit bookmarks_generated();
}


const std::vector<Bookmark>& BookmarksModel::bookmarks() const
{
    return m_bookmarks;
}
