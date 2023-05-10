#pragma once
#include <QMainWindow>

class BookmarksModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void on_generate_button_clicked();

private:
    BookmarksModel *m_model;
};
