#include "mainwindow.h"
#include "bookmarksmodel.h"
#include "bookmarksview.h"
#include <QtWidgets>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    bookmarks_model = new BookmarksModel();
    bookmarks_view = new BookmarksView();
    auto generate_button = new QPushButton("Generate Bookmarks");
    connect(generate_button, &QPushButton::clicked, this, &MainWindow::on_generate_button_clicked);
    connect(bookmarks_model, &BookmarksModel::bookmarks_generated, [] () { qDebug() << "bookmarks generated"; } );
    connect(bookmarks_view, &BookmarksView::resized, [] () { qDebug() << "view resized"; } );

    auto button_layout = new QHBoxLayout();
    button_layout->addStretch();
    button_layout->addWidget(generate_button);

    auto main_layout = new QVBoxLayout();
    main_layout->addWidget(bookmarks_view);
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
    if (!ok) {
        return;
    }

    qDebug() << "Generate" << num << "bookmarks";
    bookmarks_model->generate_bookmarks(num);
    bookmarks_view->update_bookmark_groups(bookmarks_model->bookmarks());
    bookmarks_view->update();
}
