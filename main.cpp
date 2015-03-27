#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    w.startLoginDefault(); //Not force - if it is from Settings, we will not show the Login dialog.
    return a.exec();
}
