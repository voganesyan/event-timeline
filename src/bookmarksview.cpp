#include "bookmarksview.h"
#include <QPainter>
#include <QToolTip>
#include <QtConcurrent>

using namespace std::chrono_literals;

static constexpr int TOOLTIP_MAX_ROWS = 15;
static constexpr QColor TICK_COLOR(120, 0, 120);
static constexpr QColor GROUP_COLOR(0, 200, 0, 100);
static constexpr QColor BOOKMARK_COLOR(0, 0, 200, 100);
static constexpr int NUM_HOUR_TICKS = 25;
static constexpr int HOUR_TICK_LEN = 15;
static constexpr long TICK_INTERVAL = std::chrono::milliseconds(1h).count();
static constexpr long DEFAULT_SCALE = std::chrono::milliseconds(24h).count();
static constexpr int MAX_GROUP_DIST = 100;
static constexpr int RECT_RADIUS = 4;
static constexpr qreal SCALE_FACTOR = 1.1;
static constexpr int REGROUP_DELAY = 500;


BookmarksView::BookmarksView(const BookmarksModel *model, QWidget *parent)
    : m_model(model), QWidget(parent)
{
    m_regroup_timer.setSingleShot(true);
    m_regroup_timer.setInterval(REGROUP_DELAY);
    connect(model, &BookmarksModel::bookmarks_changed,
            this, &BookmarksView::start_grouping_bookmarks);
    connect(&m_regroup_timer, &QTimer::timeout,
            this, &BookmarksView::start_grouping_bookmarks);
    connect(&m_watcher, &QFutureWatcher<QVector<BookmarksGroup>>::finished,
            this, &BookmarksView::on_grouping_bookmarks_finished);
    setMouseTracking(true);
}


void BookmarksView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(TICK_COLOR);

    static const QFontMetrics font(painter.font());
    static const int label_offset_y = font.height() + HOUR_TICK_LEN;

    for (int i = 0; i < NUM_HOUR_TICKS; ++i) {
        int tick_x = msecs_to_pixels(i * TICK_INTERVAL);
        const auto label = QString("%1h").arg(i);
        int label_offset_x = font.boundingRect(label).width() / 2;
        painter.drawText(tick_x - label_offset_x, label_offset_y, label);
        painter.drawLine(tick_x, 0, tick_x, HOUR_TICK_LEN);
    }

    m_groups_lane.y = label_offset_y + 10;
    m_groups_lane.height = font.height() + 6;
    QRect rect(0, m_groups_lane.y, 0, m_groups_lane.height);
    for (auto it = m_groups.cbegin(); it != m_groups.cend(); ++it) {
        rect.setLeft(msecs_to_pixels(it->start_time()));
        rect.setRight(msecs_to_pixels(it->end_time));
        auto num_bms = it->size();
        auto label = num_bms > 1
            ? QString::number(num_bms)
            : QString::fromStdString(it->begin()->name);
        auto &color = num_bms > 1 ? GROUP_COLOR : BOOKMARK_COLOR;
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
        painter.drawRoundedRect(rect, RECT_RADIUS, RECT_RADIUS);
        painter.setPen(color.darker());
        painter.drawRoundedRect(rect, RECT_RADIUS, RECT_RADIUS);
        painter.setPen(Qt::white);
        painter.drawText(rect, Qt::AlignVCenter, label);
    }
}


void BookmarksView::show_group_tooltip(QMouseEvent *event)
{
    const auto pt = event->position().toPoint();
    if (!m_groups_lane.covers(pt.y())) {
        return;
    }

    // Iterate groups in reverse order
    // because we want to show tooltip only on visible part of a group
    // (a later group can partially overlap an earlier group
    // since they are drawn in direct order).
    for (auto it = m_groups.crbegin(); it != m_groups.crend(); ++it) {
        auto start = msecs_to_pixels(it->start_time());
        auto end = msecs_to_pixels(it->end_time);

        if (pt.x() >= start && pt.x() < end) {
            int num_bms = it->size();
            const auto last_to_display = (num_bms > TOOLTIP_MAX_ROWS)
                ? it->begin() + TOOLTIP_MAX_ROWS
                : --it->end();
            QString tooltip;
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
        m_transform.offset += (pt - m_cursor).x();
        m_is_mouse_moved = true;
        update();
    } else {
        show_group_tooltip(event);
    }
    m_cursor = pt;
    QWidget::mouseMoveEvent(event);
}


void BookmarksView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton) {
        if (m_is_mouse_moved) {
            m_regroup_timer.start();
            m_is_mouse_moved = false;
        }
    }
    QWidget::mouseReleaseEvent(event);
}


void BookmarksView::wheelEvent(QWheelEvent *event)
{
    int angle = event->angleDelta().y();
    int anchor = event->position().x();
    qreal factor = angle > 0 ? SCALE_FACTOR : (1 / SCALE_FACTOR);
    m_transform.scale *= factor;
    m_transform.offset = anchor - factor * (anchor - m_transform.offset);
    m_regroup_timer.start();
    update();
    event->accept();
}


void BookmarksView::resizeEvent(QResizeEvent *event)
{
    auto old_width = event->oldSize().width();
    auto new_width = static_cast<qreal>(event->size().width());
    if (old_width < 0) {
        // The first event - set default scale
        m_transform.scale = new_width / DEFAULT_SCALE;
    } else {
        qreal factor = new_width / old_width;
        m_transform.scale *= factor;
        m_transform.offset *= factor;
    }
    m_regroup_timer.start();
    QWidget::resizeEvent(event);
}


static QVector<BookmarksGroup> group_bookmarks(
    const BookmarksVector &bookmarks,
    long min_time,
    long max_time,
    long max_dist)
{
    auto begin = std::ranges::find_if(
        bookmarks,
        [min_time] (const Bookmark &bm) { return bm.timestamp >= min_time; }
    );
    if (begin == bookmarks.cend()) {
        return {};
    }
    QVector<BookmarksGroup> groups;
    auto end_time = begin->end_time();
    auto next = begin + 1;
    while (next != bookmarks.cend()) {
        if (next->timestamp - begin->timestamp > max_dist) {
            groups.emplace_back(begin, next, end_time);
            if (next->timestamp >= max_time) {
                return groups;
            }
            begin = next;
            end_time = next->end_time();
        } else {
            end_time = std::max(next->end_time(), end_time);
        }
        ++next;
    }
    if (begin != bookmarks.cend()) {
        groups.emplace_back(begin, bookmarks.cend(), end_time);
    }
    return groups;
}


void BookmarksView::start_grouping_bookmarks()
{
    m_groups.clear();
    const auto &bookmarks = m_model->bookmarks();
    if (bookmarks.empty()) {
        return;
    }
    long min_time = pixels_to_msecs(0);
    long max_time = pixels_to_msecs(width());
    long max_dist = pixels_to_msecs(MAX_GROUP_DIST) - min_time;
    auto future = QtConcurrent::run(
        group_bookmarks, std::ref(bookmarks), min_time, max_time, max_dist);
    m_watcher.setFuture(future);
}


void BookmarksView::on_grouping_bookmarks_finished()
{
    m_groups = std::move(m_watcher.result());
    update();
}


int BookmarksView::msecs_to_pixels(long ms) const
{
    return static_cast<int>(ms * m_transform.scale + m_transform.offset);
}


long BookmarksView::pixels_to_msecs(int px) const
{
    return static_cast<long>((px - m_transform.offset) / m_transform.scale);
}
