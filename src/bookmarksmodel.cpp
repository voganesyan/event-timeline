#include "bookmarksmodel.h"

#include <ranges>
#include <chrono>

using namespace std::chrono_literals;
static constexpr long BOOKMARK_MAX_TIMESTAMP = std::chrono::milliseconds(24h).count();
static constexpr long BOOKMARK_MAX_DURATION = std::chrono::milliseconds(3h).count();


BookmarksModel::BookmarksModel(QObject *parent)
    : QObject(parent)
{
}


void BookmarksModel::generate_bookmarks(int amount)
{
    m_bookmarks.clear();
    m_bookmarks.reserve(amount);
    for (int i = 0; i < amount; i++) {
        auto name = QString("Bookmark %1").arg(i);
        long timestamp = std::rand() % BOOKMARK_MAX_TIMESTAMP;
        long duration = std::rand() % BOOKMARK_MAX_DURATION;
        m_bookmarks.emplace_back(name, timestamp, duration);
    }
    std::ranges::sort(
        m_bookmarks,
        [] (const Bookmark &a, const Bookmark &b) {
            return a.timestamp < b.timestamp;
        }
    );
    emit bookmarks_generated();
}


const std::vector<Bookmark>& BookmarksModel::bookmarks() const
{
    return m_bookmarks;
}