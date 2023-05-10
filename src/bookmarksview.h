#pragma once
#include "bookmarksmodel.h"
#include <QWidget>
#include <QTimer>
#include <QMouseEvent>
#include <span>


// A bookmarks group is a subrange(span) of the original bookmarks container
struct BookmarksGroup: public std::span<const Bookmark>
{
    using BookmarkIt = BookmarksVector::const_iterator;

    explicit BookmarksGroup(BookmarkIt begin, BookmarkIt end, long end_time)
        : std::span<const Bookmark>(begin, end),
          end_time(end_time) {}

    long start_time() const
    {
        return begin()->timestamp;
    }

    long end_time; // group's end time in milliseconds.
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
    void wheelEvent(QWheelEvent *event) override;

    void show_group_tooltip(QMouseEvent *event);

    int msecs_to_pixels(long ms) const;
    long pixels_to_msecs(int px) const;

private slots:
    void start_grouping_bookmarks();
    void on_grouping_bookmarks_finished();

private:
    const BookmarksModel *m_model;
    QVector<BookmarksGroup> m_groups;
    QFutureWatcher<QVector<BookmarksGroup>> m_watcher;
    QTimer m_regroup_timer;
    QPoint m_cursor;

    struct ItemsLane {
        int y = 0;
        int height = 0;

        bool covers(int p) const
        {
            return p >= y && p < y + height;
        }
    };

    struct Transform {
        float scale = -1.f;
        float offset = 0.f;
    };

    ItemsLane m_groups_lane;
    Transform m_transform;

};
