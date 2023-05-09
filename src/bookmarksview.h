#pragma once
#include "bookmarksmodel.h"
#include <QWidget>
#include <QTimer>
#include <QMouseEvent>
#include <span>


// A bookmarks group is a subrange(span) of the original container
struct BookmarksGroup: public std::span<const Bookmark>
{
    using BookmarkIt = BookmarksVector::const_iterator;
    explicit BookmarksGroup(BookmarkIt begin, BookmarkIt end, long end_time)
        : std::span<const Bookmark>(begin, end), end_time(end_time) {};
    long end_time; // group's end time in milliseconds.
};


class BookmarksView : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarksView(const BookmarksModel *model, QWidget *parent = nullptr);

public slots:
    void regroup_bookmarks();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void show_group_tooltip(QMouseEvent *event);

    int milliseconds_to_pixels(long ms) const;
    long pixels_to_milliseconds(int px) const;

private:
    void update_groups();

    const BookmarksModel *m_model;
    QVector<BookmarksGroup> m_groups;
    QTimer m_resize_timer;
    QFutureWatcher<QVector<BookmarksGroup>> m_watcher;

    QPoint m_cursor;

    int m_group_rect_y = 0;
    int m_group_rect_height = 0;

    float m_scale = -1.f;
    float m_offset = 0.f;
};
