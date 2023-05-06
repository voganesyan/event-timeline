#pragma once
#include "bookmark.h"
#include <QMainWindow>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void on_generate_button_clicked();

private:
    std::vector<Bookmark> bookmarks;
};
