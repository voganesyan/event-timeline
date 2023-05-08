#pragma once
#include "bookmarksmodel.h"
#include <QWidget>
#include <QTimer>

class BookmarksView : public QWidget
{
    Q_OBJECT

public:
    BookmarksView(const BookmarksModel *model, QWidget *parent = nullptr);

public slots:
    void group_bookmarks();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    int milliseconds_to_pixels(long ms) const;
    long pixels_to_milliseconds(int px) const;

private:
    const BookmarksModel *m_model;

    // A bookmark group is represented as an iterator (of the original container)
    // pointing to the first element of the group.
    using GroupFirstBookmark = std::vector<Bookmark>::const_iterator;
    std::vector<GroupFirstBookmark> m_groups;

    QTimer m_resize_timer;
};
