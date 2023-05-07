#pragma once
#include "bookmark.h"
#include <QObject>

class BookmarksModel: public QObject
{
    Q_OBJECT

public:
    explicit BookmarksModel(QObject *parent = nullptr);

    void generate_bookmarks(int amount);

signals:
    void bookmarks_generated();

private:
    std::vector<Bookmark> bookmarks;
};
