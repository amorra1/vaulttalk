#include "mainwindow.h"
#include "networking.h"
#include "encryption.h"

#include <QApplication>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // for testing websockets
    // networking net;
    // net.sendMessage("Start from main");

    std::string testMsg = "Yo";

    RSA_keys keys = encryption::GenerateKeys();
    unsigned long long encrypted = encryption::RSA_Encrypt(testMsg, keys);
    string decrypted = encryption::RSA_Decrypt(encrypted, keys);

    MainWindow w;
    w.show();

    return a.exec();
}
