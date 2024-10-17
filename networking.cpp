#include "networking.h"
#include "message.h"
#include "user.h"
#include <QWebSocket>
#include <QUrl>
#include <QDebug>
#include <QString>
#include <iostream>

// constructor
networking::networking(QObject *parent)
    : QObject(parent), m_webSocket(new QWebSocket()) {
    connect(m_webSocket, &QWebSocket::connected, this, &networking::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &networking::onDisconnected);
    connect(m_webSocket, &QWebSocket::errorOccurred, this, &networking::onError);
}

// destructor
networking::~networking() {
    delete m_webSocket;
}

// send message function
void networking::sendMessage(Message &message) {
    // use whatever address the webserver is hosted on (localhost for testing)
    QUrl url(QStringLiteral("ws://localhost:12345"));

    // start websocket connection at url
    m_webSocket->open(url);

    // once connected, send the message content
    connect(m_webSocket, &QWebSocket::connected, [this, message]() {
        QString encryptedMessage = QString::fromStdString(message.getEncryptedContent());
        m_webSocket->sendTextMessage(encryptedMessage);
        //This line below commented out was giving me a segmentation fault for some reason
        // std::cout << "Sent message: " << message.toStdString() << std::endl;
    });
}

// webSocket event slots, these output on events (self explanitory)
void networking::onConnected() {
    qDebug() << "WebSocket connected.";
}

void networking::onDisconnected() {
    qDebug() << "WebSocket disconnected.";
}

void networking::onError(QAbstractSocket::SocketError error) {
    qDebug() << "WebSocket error:" << error;
}
