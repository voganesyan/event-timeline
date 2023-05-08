#include "bookmarksview.h"
#include <QPainter>
#include <QDebug>


BookmarksView::BookmarksView(const BookmarksModel *model, QWidget *parent)
    : m_model(model), QWidget(parent)
{
    m_resize_timer.setSingleShot(true);
    connect(model, &BookmarksModel::bookmarks_changed, this, &BookmarksView::group_bookmarks);
    connect(&m_resize_timer, &QTimer::timeout, this, &BookmarksView::group_bookmarks);
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

    if (m_groups.empty()) {
        return;
    }

    for (auto it = ++m_groups.cbegin(); it != m_groups.cend(); ++it) {
        const auto start = *(it - 1);
        const auto end = (*it - 1);

        auto start_px = milliseconds_to_pixels(start->timestamp);
        auto end_px = milliseconds_to_pixels(end->timestamp + end->duration);
        auto num_bookmarks = std::distance(start, end);
        auto label = num_bookmarks > 1
            ? QString::number(num_bookmarks)
            : QString::fromStdString(start->name);
        QRect rect(start_px, y_offset + 10, end_px - start_px, tick_len);
        painter.drawRoundedRect(rect, 1, 1);
        painter.drawText(rect, label);
    }
}


void BookmarksView::resizeEvent(QResizeEvent *event)
{
    m_resize_timer.start(500);
    QWidget::resizeEvent(event);
}


void BookmarksView::group_bookmarks()
{
    m_groups.clear();

    const auto &bookmarks = m_model->bookmarks();
    if (bookmarks.empty()) {
        return;
    }

    auto t1 = std::chrono::steady_clock::now();

    const auto max_dist = pixels_to_milliseconds(100);
    m_groups.push_back(bookmarks.cbegin());
    for (auto it = ++bookmarks.cbegin(); it != bookmarks.cend(); ++it) {
        if (it->timestamp - m_groups.back()->timestamp > max_dist) {
            m_groups.push_back(it);
        }
    }
    m_groups.push_back(bookmarks.cend());

    auto t2 = std::chrono::steady_clock::now();

    qDebug() << "G" << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    update();
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
