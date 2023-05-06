#include "mainwindow.h"
#include "bookmarksview.h"
#include <QtWidgets>

#include <ranges>
#include <chrono>

using namespace std::chrono_literals;
static constexpr int BOOKMARK_MAX_TIMESTAMP = static_cast<int>(std::chrono::milliseconds(24h).count());
static constexpr int BOOKMARK_MAX_DURATION = static_cast<int>(std::chrono::milliseconds(3h).count());


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto bookmarks_view = new BookmarksView();
    auto generate_button = new QPushButton("Generate Bookmarks");
    connect(generate_button, &QPushButton::clicked, this, &MainWindow::on_generate_button_clicked);

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
        this, "Generate Bookmarks", "Number of Bookmarks:", 50, 0, 100000000, 1, &ok);
    if (!ok) {
        return;
    }
    qDebug() << "Generate" << num << "bookmarks";
    bookmarks.reserve(num);
    bookmarks.clear();
    for (int i = 0; i < num; i++) {
        auto name = QString("Bookmark %1").arg(i);
        int timestamp = std::rand() % BOOKMARK_MAX_TIMESTAMP;
        int duration = std::rand() % BOOKMARK_MAX_DURATION;
        bookmarks.emplace_back(name, timestamp, duration);
    }
    std::ranges::sort(
        bookmarks,
        [] (const Bookmark &a, const Bookmark &b) {
            return a.timestamp < b.timestamp;
        }
    );

    qDebug() << "Generate" << num << "bookmarks";
}
