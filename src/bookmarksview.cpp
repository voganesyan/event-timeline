#include "bookmarksview.h"

#include <QPainter>
#include <QTime>
#include <QTimer>

BookmarksView::BookmarksView(QWidget *parent)
    : QWidget(parent)
{
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

    const QFontMetrics font(painter.font());
    const int y_offset = font.height() + tick_len;

    for (int i = 0; i < num_hours; ++i) {
        const auto label = QString("%1h").arg(i);
        int x_offset = -font.boundingRect(label).width() / 2;
        painter.drawText(x_offset, y_offset, label);
        painter.drawLine(0, 0, 0, tick_len);
        painter.translate(hour_step, 0);
    }
}
