#include "mainwindow.h"
#include "bookmarksmodel.h"
#include "bookmarksview.h"
#include <QtWidgets>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_model = new BookmarksModel(this);
    auto m_view = new BookmarksView(m_model);
    auto generate_button = new QPushButton("Generate Bookmarks");
    connect(generate_button, &QPushButton::clicked, this, &MainWindow::on_generate_button_clicked);

    auto button_layout = new QHBoxLayout();
    button_layout->addStretch();
    button_layout->addWidget(generate_button);

    auto main_layout = new QVBoxLayout();
    main_layout->addWidget(m_view);
    main_layout->addLayout(button_layout);

    auto main_widget = new QWidget();
    main_widget->setLayout(main_layout);
    setCentralWidget(main_widget);
}


void MainWindow::on_generate_button_clicked()
{
    bool ok;
    int num = QInputDialog::getInt(
        this, "Generate Bookmarks", "Number of Bookmarks:", 50000000, 0, 100000000, 1, &ok);
    if (ok) {
        m_model->generate_bookmarks(num);
    }
}
