#include "mainwindow.h"
#include "analogclock.h"
#include <QtWidgets>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto clock = new AnalogClock();
    auto generate_button = new QPushButton("Generate Bookmarks");
    connect(generate_button, &QPushButton::clicked, this, &MainWindow::on_generate_button_clicked);

    auto button_layout = new QHBoxLayout();
    button_layout->addStretch();
    button_layout->addWidget(generate_button);

    auto main_layout = new QVBoxLayout();
    main_layout->addWidget(clock);
    main_layout->addLayout(button_layout);

    auto main_widget = new QWidget();
    main_widget->setLayout(main_layout);
    setCentralWidget(main_widget);
}


void MainWindow::on_generate_button_clicked()
{
    qDebug() << "Generate Bookmarks";
}
