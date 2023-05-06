#include "mainwindow.h"
#include "analogclock.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto clock = new AnalogClock();
    setCentralWidget(clock);
}
