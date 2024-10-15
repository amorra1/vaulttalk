#include "mainwindow.h"
#include "networking.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // for testing websockets
    networking net;
    net.sendMessage("test message");

    MainWindow w;
    w.show();

    return a.exec();
}
