#pragma once
#include "bookmarksmodel.h"
#include <QWidget>
#include <QTimer>
#include <QMouseEvent>
#include <span>


// A bookmarks group is basically a subrange(span) of the original bookmarks container
struct BookmarksGroup: public std::span<const Bookmark>
{
    using BookmarkIt = BookmarksVector::const_iterator;

    // Group is a pair of start and end iterators
    // plus end-time that is calculated when grouping
    explicit BookmarksGroup(BookmarkIt begin, BookmarkIt end, long end_time)
        : std::span<const Bookmark>(begin, end),
          end_time(end_time) {}

    // Start time of the first bookmark in the group
    long start_time() const
    {
        return begin()->timestamp;
    }

    // End time of the latest (not last) bookmark (pre-calculated) in the group
    long end_time;
};


class BookmarksView : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarksView(const BookmarksModel *model, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void show_group_tooltip(QMouseEvent *event);

    int msecs_to_pixels(long ms) const;
    long pixels_to_msecs(int px) const;

private slots:
    void start_grouping_bookmarks();
    void on_grouping_bookmarks_finished();

private:
    struct ItemsLane
    {
        int y;
        int height;
        bool covers(int p) const { return p >= y && p < y + height; }
    };

    struct Transform
    {
        qreal scale;
        qreal offset;
    };

    const BookmarksModel *m_model;
    QVector<BookmarksGroup> m_groups;
    QFutureWatcher<QVector<BookmarksGroup>> m_watcher;
    QTimer m_regroup_timer;
    QPoint m_cursor;
    bool m_is_mouse_moved = false;
    ItemsLane m_groups_lane;
    Transform m_transform;
};
