#include "bookmarksview.h"
#include <QPainter>
#include <QToolTip>
#include <QtConcurrent>
#include <QDebug>

using namespace std::chrono_literals;

static constexpr int TOOLTIP_MAX_ROWS = 15;
static constexpr QColor TICK_COLOR(120, 0, 120);
static constexpr QColor GROUP_COLOR(0, 200, 0, 100);
static constexpr QColor BOOKMARK_COLOR(0, 0, 200, 100);
static constexpr int NUM_HOURS = 24;
static constexpr int TICK_LEN = 20;
static constexpr int TICK_INTERVAL = std::chrono::milliseconds(1h).count();
static constexpr int MAX_GROUP_DIST = 100;
static constexpr int RECT_RADIUS = 4;
static constexpr qreal SCALE_FACTOR = 1.1;


BookmarksView::BookmarksView(const BookmarksModel *model, QWidget *parent)
    : m_model(model), QWidget(parent)
{
    m_resize_timer.setSingleShot(true);
    connect(model, &BookmarksModel::bookmarks_changed, this, &BookmarksView::regroup_bookmarks);
    connect(&m_resize_timer, &QTimer::timeout, this, &BookmarksView::regroup_bookmarks);
    connect(&m_watcher, &QFutureWatcher<QVector<BookmarksGroup>>::finished,
            this, &BookmarksView::update_groups);
    setMouseTracking(true);
}


void BookmarksView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(TICK_COLOR);

    const QFontMetrics font(painter.font());
    const int label_offset_y = font.height() + TICK_LEN;
    m_group_rect_y = label_offset_y + 10;
    m_group_rect_height = TICK_LEN;

    for (int i = 0; i < NUM_HOURS; ++i) {
        int tick_x = milliseconds_to_pixels(i * TICK_INTERVAL);
        const auto label = QString("%1h").arg(i);
        int label_offset_x = -font.boundingRect(label).width() / 2;
        painter.drawText(tick_x + label_offset_x, label_offset_y, label);
        painter.drawLine(tick_x, 0, tick_x, TICK_LEN);
    }

    if (m_groups.empty()) {
        return;
    }

    for (auto it = m_groups.cbegin(); it != m_groups.cend(); ++it) {
        auto start = milliseconds_to_pixels(it->start_time());
        auto end = milliseconds_to_pixels(it->end_time);
        auto num_bms = it->size();
        auto label = num_bms > 1
            ? QString::number(num_bms)
            : QString::fromStdString(it->begin()->name);
        const QRect rect(start, m_group_rect_y, end - start, m_group_rect_height);
        const auto &color = num_bms > 1 ? GROUP_COLOR : BOOKMARK_COLOR;
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawRoundedRect(rect, RECT_RADIUS, RECT_RADIUS);
        painter.setPen(color.darker());
        painter.drawRoundedRect(rect, RECT_RADIUS, RECT_RADIUS);
        painter.setPen(Qt::white);
        painter.drawText(rect, label);
    }
}


void BookmarksView::show_group_tooltip(QMouseEvent *event)
{
    if (m_groups.empty()) {
        return;
    }

    const auto pt = event->position().toPoint();
    if (pt.y() < m_group_rect_y || pt.y() >= m_group_rect_y + m_group_rect_height) {
        return;
    }

    for (auto it = m_groups.crbegin(); it != m_groups.crend(); ++it) {
        auto start = milliseconds_to_pixels(it->start_time());
        auto end = milliseconds_to_pixels(it->end_time);

        if (pt.x() >= start && pt.x() < end) {
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
}


void BookmarksView::mouseMoveEvent(QMouseEvent *event)
{
    auto pt = event->position().toPoint();
    if(event->buttons() & Qt::RightButton) {
        m_offset += (pt - m_cursor).x();
        update();
    } else {
        show_group_tooltip(event);
    }
    m_cursor = pt;
    QWidget::mouseMoveEvent(event);
}


void BookmarksView::wheelEvent(QWheelEvent *event)
{
    int angle = event->angleDelta().y();
    int anchor = event->position().x();
    qreal factor = angle > 0 ? SCALE_FACTOR : (1 / SCALE_FACTOR);
    m_scale *= factor;
    m_offset = anchor - factor * (anchor - m_offset);
    update();
    event->accept();
}


void BookmarksView::resizeEvent(QResizeEvent *event)
{
    if (m_scale < 0) {
        m_scale = static_cast<float>(width()) / std::chrono::milliseconds(24h).count();
    } else {
        qreal factor = static_cast<float>(width()) / event->oldSize().width();
        m_scale *= factor;
        m_offset *= factor;
    }
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
    const auto max_dist = pixels_to_milliseconds(MAX_GROUP_DIST);
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
    return static_cast<int>(ms * m_scale + m_offset);
}


long BookmarksView::pixels_to_milliseconds(int px) const
{
    return (px - m_offset) / m_scale;
}
