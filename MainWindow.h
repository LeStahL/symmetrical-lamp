#pragma once

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

    Ui::MainWindow *ui;
};
