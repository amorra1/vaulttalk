#include "networking.h"
#include "message.h"
#include "user.h"
#include <QWebSocket>
#include <QUrl>
#include <QDebug>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QEventLoop>

#include <gmpxx.h>

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
        User recipientUser = getUser(recipient);
        // in the case that the user does not exist
        if (recipientUser.getUsername() == ""){
            return false;
        }
        messageJson["message"] = QString::fromStdString(message.getEncryptedContent(recipientUser));
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
    //qDebug() << "WebSocket connected.";

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
    qDebug() << "Connected to" << host;
    //qDebug() << "Sent user data to server: " << userJsonString;
}

void networking::onDisconnected() {
    qDebug() << "WebSocket disconnected.";
}

void networking::onError(QAbstractSocket::SocketError error) {
    qDebug() << "WebSocket error:" << error;
}

void networking::onMessageReceived(const QString &message) {

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());

    QJsonObject jsonObj = doc.object();
    QString sender = jsonObj["from"].toString();
    QString encryptedContent = jsonObj["message"].toString();
    if (sender.isEmpty()){
        return;
    }
    qDebug() << "sender: " << sender << " message: " << encryptedContent;
    //decode message (to be done)
}

User networking::getUser(const QString &username) {
    if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject requestJson;
        requestJson["username"] = username;

        QJsonDocument jsonDoc(requestJson);
        QString request = QString::fromUtf8(jsonDoc.toJson());

        // send request to server
        m_webSocket->sendTextMessage(request);
        qDebug() << "Requesting public key for user:" << username;

        // wait for response (bad code fix with async io)
        QEventLoop loop;
        QString responseMessage;

        connect(m_webSocket, &QWebSocket::textMessageReceived, [&loop, &responseMessage](const QString &message) {
            responseMessage = message;
            loop.quit();
        });

        loop.exec();

        // debug and check if key recieved
        //qDebug() << responseMessage;

        // process
        QJsonDocument responseDoc = QJsonDocument::fromJson(responseMessage.toUtf8());
        QJsonObject responseJson = responseDoc.object();

        // if something goes wrong return blank user (not to crash any future code)
        if (responseJson.contains("error")) {
            qDebug() << "Error: " << responseJson["error"].toString();
            return User();
        }

        // extract data from response
        QString encryptionMethod = responseJson["encryptionMethod"].toString();
        QString regenDuration = responseJson["regenDuration"].toString();

        // handle large integers for public key (e and n)
        QString eStr = responseJson["publicKey"].toObject()["e"].toString();
        QString nStr = responseJson["publicKey"].toObject()["n"].toString();

        mpz_class e, n;
        e.set_str(eStr.toStdString(), 16);
        n.set_str(nStr.toStdString(), 16);

        RSA_keys keys;
        keys.publicKey[0] = e;
        keys.publicKey[1] = n;

        // return user
        User requestedUser(username.toStdString(), encryptionMethod.toStdString(), regenDuration.toStdString(), keys);
        return requestedUser;

    } else {
        qDebug() << "Connection not established. Cannot request public key!";
        return User();
    }
}

/* some footer notes:
 * json stuff is handled through strings currently, but websockets have the ability to
 * send binary data instead, but im not sure how much better that would be?
 */
