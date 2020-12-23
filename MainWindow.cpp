#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Shader", "", "Fragment Shader Files (*.frag *.glsl)");

    if(fileName == "") 
    {
        ui->textBrowser->setPlainText("Error: No shader selected.");
        return;
    }

    
}