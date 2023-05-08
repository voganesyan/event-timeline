#pragma once
#include "bookmark.h"
#include <QFutureWatcher>
#include <QObject>

using BookmarksVector = QVector<Bookmark>;

class BookmarksModel: public QObject
{
    Q_OBJECT

public:
    explicit BookmarksModel(QObject *parent = nullptr);

    void regenerate_bookmarks(int amount);

    const BookmarksVector& bookmarks() const;

signals:
    void bookmarks_changed();

private:
    void update_bookmarks();

    BookmarksVector m_bookmarks;

    QFutureWatcher<BookmarksVector> watcher;
};
