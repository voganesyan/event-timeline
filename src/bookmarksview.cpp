#include "bookmarksview.h"
#include <QPainter>


BookmarksView::BookmarksView(QWidget *parent)
    : QWidget(parent)
{
    m_resize_timer.setSingleShot(true);
    connect(&m_resize_timer, &QTimer::timeout, this, &BookmarksView::resized);
}


void BookmarksView::paintEvent(QPaintEvent *)
{
    const int win_width = width();
    const int num_hours = 24;
    const int hour_step = win_width / num_hours;
    const int tick_len = 20;
    const QColor tick_color(127, 0, 127);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(tick_color);

    painter.save();
    const QFontMetrics font(painter.font());
    const int y_offset = font.height() + tick_len;

    for (int i = 0; i < num_hours; ++i) {
        const auto label = QString("%1h").arg(i);
        int x_offset = -font.boundingRect(label).width() / 2;
        painter.drawText(x_offset, y_offset, label);
        painter.drawLine(0, 0, 0, tick_len);
        painter.translate(hour_step, 0);
    }
    painter.restore();

    for (const auto &group : m_groups) {
        auto start = milliseconds_to_pixels(group.front()->timestamp);
        auto end = milliseconds_to_pixels(group.back()->timestamp + group.back()->duration);
        auto num_bookmarks = group.size();
        const auto label = num_bookmarks > 1
            ? QString::number(num_bookmarks)
            : group.front()->name;

        QRect rect(start, y_offset + 10, end - start, tick_len);
        painter.drawRoundedRect(rect, 1, 1);
        painter.drawText(rect, label);
    }
}


void BookmarksView::resizeEvent(QResizeEvent *event)
{
    m_resize_timer.start(500);
    QWidget::resizeEvent(event);
}


void BookmarksView::update_bookmark_groups(const std::vector<Bookmark> &bookmarks)
{
    m_groups.clear();

    if (bookmarks.empty()) {
        return;
    }

    m_groups.push_back({&bookmarks[0]});
    auto key_timestamp = bookmarks[0].timestamp;
    const auto max_dist = pixels_to_milliseconds(100);
    for (size_t i = 1; i < bookmarks.size(); i++) {
        const auto &bookmark = bookmarks[i];
        if (bookmark.timestamp - key_timestamp < max_dist) {
            m_groups.back().push_back(&bookmark);
        } else {
            m_groups.push_back({&bookmark});
            key_timestamp = bookmark.timestamp;
        }
    }
}


int BookmarksView::milliseconds_to_pixels(long ms) const
{
    using namespace std::chrono_literals;
    long max_timestamp = std::chrono::milliseconds(24h).count();
    return static_cast<int>(ms * width() / max_timestamp);
}


long BookmarksView::pixels_to_milliseconds(int px) const
{
    using namespace std::chrono_literals;
    long max_timestamp = std::chrono::milliseconds(24h).count();
    return px * max_timestamp / width();
}
