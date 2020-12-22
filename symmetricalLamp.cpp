#include <QApplication>

#include "MainWindow.h"

int main(int argc, char **args)
{
    QApplication application(argc, args);
    MainWindow window;
    window.show();

    return application.exec();
}
