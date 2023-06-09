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
    void start_generating_bookmarks(int amount);
    const BookmarksVector& bookmarks() const;

signals:
    void bookmarks_changed();

private slots:
    void on_generating_bookmarks_finished();

private:
    BookmarksVector m_bookmarks;
    QFutureWatcher<BookmarksVector> m_watcher;
};
