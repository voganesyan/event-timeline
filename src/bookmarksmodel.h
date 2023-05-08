#pragma once
#include "bookmark.h"
#include <QFutureWatcher>
#include <QObject>

class BookmarksModel: public QObject
{
    Q_OBJECT

public:
    explicit BookmarksModel(QObject *parent = nullptr);

    void regenerate_bookmarks(int amount);

    const std::vector<Bookmark>& bookmarks() const;

signals:
    void bookmarks_changed();

private:
    void update_bookmarks();

    std::vector<Bookmark> m_bookmarks;

    QFutureWatcher<std::vector<Bookmark>> watcher;
};
