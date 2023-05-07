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
    void bookmarks_changed(const std::vector<Bookmark> &bookmarks);

private:
    std::vector<Bookmark> m_bookmarks;
};
