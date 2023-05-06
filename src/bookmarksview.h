#pragma once
#include <QWidget>

class BookmarksView : public QWidget
{
    Q_OBJECT

public:
    BookmarksView(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};
