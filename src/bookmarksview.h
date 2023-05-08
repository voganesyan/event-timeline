#pragma once
#include "bookmarksmodel.h"
#include <QWidget>
#include <QTimer>
#include <QMouseEvent>
#include <span>


class BookmarksView : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarksView(const BookmarksModel *model, QWidget *parent = nullptr);

public slots:
    void group_bookmarks();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    int milliseconds_to_pixels(long ms) const;
    long pixels_to_milliseconds(int px) const;

private:
    const BookmarksModel *m_model;

    // A bookmarks group is a subrange(span) of the original container
    struct BookmarksGroup: public std::span<const Bookmark>
    {
        using BookmarkIt = BookmarksVector::const_iterator;
        explicit BookmarksGroup(BookmarkIt begin, BookmarkIt end, long end_time)
            : std::span<const Bookmark>(begin, end), end_time(end_time) {};
        long end_time; // group's end time in milliseconds.
    };
    QVector<BookmarksGroup> m_groups;

    QTimer m_resize_timer;

    int m_group_rect_y = 0;
    int m_group_rect_height = 0;
};
