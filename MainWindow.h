#pragma once

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
    
    public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

    Ui::MainWindow *ui;

    private slots:
    void openClicked();
};
