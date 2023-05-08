#include "bookmarksview.h"
#include <QPainter>
#include <QToolTip>
#include <QDebug>

static constexpr int TOOLTIP_MAX_ROWS = 15;


BookmarksView::BookmarksView(const BookmarksModel *model, QWidget *parent)
    : m_model(model), QWidget(parent)
{
    m_resize_timer.setSingleShot(true);
    connect(model, &BookmarksModel::bookmarks_changed, this, &BookmarksView::group_bookmarks);
    connect(&m_resize_timer, &QTimer::timeout, this, &BookmarksView::group_bookmarks);

    setMouseTracking(true);
}


void BookmarksView::paintEvent(QPaintEvent *)
{
    const int win_width = width();
    const int num_hours = 24;
    const int hour_step = win_width / num_hours;
    const int tick_len = 20;
    static constexpr QColor tick_color(127, 0, 127);
    static constexpr QColor group_color(0, 200, 0);
    static constexpr QColor bookmark_color(0, 0, 200);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(tick_color);

    painter.save();
    const QFontMetrics font(painter.font());
    const int y_offset = font.height() + tick_len;
    m_group_rect_y = y_offset + 10;
    m_group_rect_height = tick_len;

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
        auto num_bookmarks = std::distance(start, end) + 1;
        auto label = num_bookmarks > 1
            ? QString::number(num_bookmarks)
            : QString::fromStdString(start->name);
        const QRect rect(start_px, m_group_rect_y, end_px - start_px, m_group_rect_height);
        const auto &color = num_bookmarks > 1 ? group_color : bookmark_color;
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawRoundedRect(rect, 4, 4);
        painter.setPen(color.darker());
        painter.drawRoundedRect(rect, 4, 4);
        painter.setPen(Qt::white);
        painter.drawText(rect, label);
    }
}


void BookmarksView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_groups.empty()) {
        return;
    }

    const auto pt = event->pos();
    if (pt.y() < m_group_rect_y || pt.y() >= m_group_rect_y + m_group_rect_height) {
        return;
    }

    for (auto it = --m_groups.cend(); it != m_groups.cbegin(); --it) {
        const auto start = *(it - 1);
        const auto end = *it - 1;

        auto start_px = milliseconds_to_pixels(start->timestamp);
        auto end_px = milliseconds_to_pixels(end->timestamp + end->duration);

        if (pt.x() >= start_px && pt.x() < end_px) {
            std::string tooltip;
            int num_bms = std::distance(start, end) + 1;
            const auto last_to_display = (num_bms > TOOLTIP_MAX_ROWS)
                ? (start + TOOLTIP_MAX_ROWS) : end;
            for (auto bm = start; bm != last_to_display; ++bm) {
                tooltip += bm->name + '\n';
            }
            const auto last_line = (num_bms > TOOLTIP_MAX_ROWS)
                ? QString("+ %1 other bookmarks").arg(num_bms - TOOLTIP_MAX_ROWS)
                : QString::fromStdString(end->name);
            QString qtooltip = QString::fromStdString(tooltip) + last_line;

            QToolTip::showText(
                event->globalPosition().toPoint(),
                qtooltip, this, rect());
            break;
        }
    }
    QWidget::mouseMoveEvent(event);
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
