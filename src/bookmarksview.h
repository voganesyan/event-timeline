#pragma once
#include "bookmark.h"
#include <QWidget>
#include <QTimer>

class BookmarksView : public QWidget
{
    Q_OBJECT

public:
    BookmarksView(QWidget *parent = nullptr);

public slots:
    void group_bookmarks(const std::vector<Bookmark> &bookmarks);

signals:
    void resized();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    int milliseconds_to_pixels(long ms) const;
    long pixels_to_milliseconds(int px) const;
private:
    using BookmarkGroup = std::vector<const Bookmark *>;
    std::vector<BookmarkGroup> m_groups;
    QTimer m_resize_timer;
};
