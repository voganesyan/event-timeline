#include "bookmarksview.h"
#include <QPainter>
#include <QToolTip>
#include <QtConcurrent>
#include <QDebug>

static constexpr int TOOLTIP_MAX_ROWS = 15;


BookmarksView::BookmarksView(const BookmarksModel *model, QWidget *parent)
    : m_model(model), QWidget(parent)
{
    m_resize_timer.setSingleShot(true);
    connect(model, &BookmarksModel::bookmarks_changed, this, &BookmarksView::regroup_bookmarks);
    connect(&m_resize_timer, &QTimer::timeout, this, &BookmarksView::regroup_bookmarks);

    setMouseTracking(true);
}


void BookmarksView::paintEvent(QPaintEvent *)
{
    const int win_width = width();
    const int num_hours = 24;
    const int hour_step = win_width / num_hours;
    const int tick_len = 20;
    static constexpr QColor tick_color(120, 0, 120);
    static constexpr QColor group_color(0, 200, 0, 100);
    static constexpr QColor bookmark_color(0, 0, 200, 100);

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

    for (auto it = m_groups.cbegin(); it != m_groups.cend(); ++it) {
        auto start_px = milliseconds_to_pixels(it->begin()->timestamp);
        auto end_px = milliseconds_to_pixels(it->end_time);
        auto num_bms = it->size();
        auto label = num_bms > 1
            ? QString::number(num_bms)
            : QString::fromStdString(it->begin()->name);
        const QRect rect(start_px, m_group_rect_y, end_px - start_px, m_group_rect_height);
        const auto &color = num_bms > 1 ? group_color : bookmark_color;
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

    for (auto it = m_groups.crbegin(); it != m_groups.crend(); ++it) {
        auto start_px = milliseconds_to_pixels(it->begin()->timestamp);
        auto end_px = milliseconds_to_pixels(it->end_time);

        if (pt.x() >= start_px && pt.x() < end_px) {
            QString tooltip;
            int num_bms = it->size();
            const auto last_to_display = (num_bms > TOOLTIP_MAX_ROWS)
                ? (it->begin() + TOOLTIP_MAX_ROWS) : --it->end();
            for (auto bm = it->begin(); bm != last_to_display; ++bm) {
                tooltip += QString::fromStdString(bm->name) + '\n';
            }
            tooltip += num_bms > TOOLTIP_MAX_ROWS
                ? QString("+ %1 other bookmarks").arg(num_bms - TOOLTIP_MAX_ROWS)
                : QString::fromStdString(last_to_display->name);
            QToolTip::showText(
                event->globalPosition().toPoint(),
                tooltip, this, rect());
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


static QVector<BookmarksGroup> group_bookmarks(const BookmarksVector &bookmarks, int max_dist)
{
    auto t1 = std::chrono::steady_clock::now();

    QVector<BookmarksGroup> groups;
    auto begin = bookmarks.cbegin();
    auto end_time = begin->end_time();
    for (auto it = begin + 1; it != bookmarks.cend(); ++it) {
        if (it->timestamp - begin->timestamp > max_dist) {
            groups.emplace_back(begin, it, end_time);
            begin = it;
            end_time = it->end_time();
        } else {
            end_time = std::max(it->end_time(), end_time);
        }
    }
    groups.emplace_back(begin, bookmarks.cend(), end_time);

    auto t2 = std::chrono::steady_clock::now();
    qDebug() << "G" << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    return groups;
}


void BookmarksView::regroup_bookmarks()
{
    m_groups.clear();

    const auto &bookmarks = m_model->bookmarks();
    if (bookmarks.empty()) {
        return;
    }
    const auto max_dist = pixels_to_milliseconds(100);
    connect(&m_watcher, &QFutureWatcher<QVector<BookmarksGroup>>::finished,
            this, &BookmarksView::update_groups);
    auto future = QtConcurrent::run(group_bookmarks, std::ref(bookmarks), max_dist);
    m_watcher.setFuture(future);
}


void BookmarksView::update_groups()
{
    m_groups = std::move(m_watcher.result());
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
