#include "networking.h"
#include "message.h"
#include "user.h"
#include <QWebSocket>
#include <QUrl>
#include <QDebug>
#include <QString>


QString host = "ws://localhost:12345";
// constructor
networking::networking(QObject *parent)
    : QObject(parent), m_webSocket(new QWebSocket()) {
    // use whatever address the webserver is hosted on (localhost for testing)
    QUrl url(host);

    // start websocket connection at url
    m_webSocket->open(url);

    // connections for WebSocket events
    connect(m_webSocket, &QWebSocket::connected, this, &networking::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &networking::onDisconnected);
    connect(m_webSocket, &QWebSocket::errorOccurred, this, &networking::onError);

    // incoming messages
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &networking::onMessageReceived);
}

// destructor
networking::~networking() {
    delete m_webSocket;
}

// send message function
void networking::sendMessage(Message &message) {
    if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
        QString encryptedMessage = QString::fromStdString(message.getEncryptedContent());
        m_webSocket->sendTextMessage(encryptedMessage);
        qDebug() << "Sent message: " << encryptedMessage;
    } else {
        qDebug() << "Connection not established. Message was not sent!";
    }
}

int networking::reconnect(){
    if (m_webSocket->state() != QAbstractSocket::ConnectedState) {
        m_webSocket->open(QUrl(host));
        qDebug() << "Reconnecting to WebSocket at" << host;
    }
}

// webSocket event slots, these output on events (self explanatory)
void networking::onConnected() {
    qDebug() << "WebSocket connected.";
}

void networking::onDisconnected() {
    qDebug() << "WebSocket disconnected.";
}

void networking::onError(QAbstractSocket::SocketError error) {
    qDebug() << "WebSocket error:" << error;
}

void networking::onMessageReceived(const QString &message) {
    qDebug() << "Message received: " << message;
}
