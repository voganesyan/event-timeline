#pragma once
#include "bookmark.h"
#include <QWidget>

class BookmarksView : public QWidget
{
    Q_OBJECT

public:
    BookmarksView(QWidget *parent = nullptr);

    void update_bookmark_groups(const std::vector<Bookmark> &bookmarks);

protected:
    void paintEvent(QPaintEvent *event) override;

    int milliseconds_to_pixels(long ms) const;

private:
    using BookmarkGroup = std::vector<const Bookmark *>;
    std::vector<BookmarkGroup> m_groups;
};
