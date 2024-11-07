#include "networking.h"
#include "message.h"
#include "user.h"
#include <QWebSocket>
#include <QUrl>
#include <QDebug>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>

QString host = "ws://localhost:8001";
// constructor
networking::networking(User &user, QObject *parent)
    : QObject(parent), user(user), m_webSocket(new QWebSocket()){
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
bool networking::sendMessage(const QString &recipient, Message &message, User user) {
    if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
        // create message json
        QJsonObject messageJson;
        messageJson["sender"] = QString::fromStdString(user.getUsername());
        messageJson["recipient"] = recipient;
        messageJson["message"] = QString::fromStdString(message.getEncryptedContent(user));
        QJsonDocument jsonDoc(messageJson);
        QString messageJsonString = QString::fromUtf8(jsonDoc.toJson());

        // send json
        m_webSocket->sendTextMessage(messageJsonString);
        qDebug() << "Sent message: " << messageJsonString;
        return true;
    } else {
        qDebug() << "Connection not established. Message was not sent!";
        return false;
    }
}

void networking::reconnect(){
    if (m_webSocket->state() != QAbstractSocket::ConnectedState) {
        m_webSocket->open(QUrl(host));
        qDebug() << "Reconnecting to WebSocket at" << host;
    }
}

// webSocket event slots, these output on events (self explanatory)
void networking::onConnected() {
    qDebug() << "WebSocket connected.";

    // export user data to json to send to server
    QJsonObject userJson;
    userJson["username"] = QString::fromStdString(user.getUsername());
    userJson["encryptionMethod"] = QString::fromStdString(user.getEncryptionMethod());
    userJson["regenDuration"] = QString::fromStdString(user.getRegenDuration());

    QJsonObject publicKey;
    publicKey["n"] = QString::fromStdString(user.getPublicKey().first.get_str(16));
    publicKey["e"] = QString::fromStdString(user.getPublicKey().second.get_str(16));

    userJson["publicKey"] = publicKey;

    QJsonDocument jsonDoc(userJson);
    QString userJsonString = QString::fromUtf8(jsonDoc.toJson());

    // success message
    m_webSocket->sendTextMessage(userJsonString);
    qDebug() << "Sent user data to server: " << userJsonString;
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
