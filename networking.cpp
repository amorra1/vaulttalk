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
    : QObject(parent), user(user), m_webSocket(new QWebSocket()) {
    QUrl url(host);
    m_webSocket->open(url);

    connect(m_webSocket, &QWebSocket::connected, this, &networking::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &networking::onDisconnected);
    connect(m_webSocket, &QWebSocket::errorOccurred, this, &networking::onError);
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

    RSA_keys publicKeyPair = user.getKeys();

    QJsonObject publicKey;
    publicKey["n"] = QString::fromStdString(publicKeyPair.publicKey[0].get_str(10));
    publicKey["e"] = QString::fromStdString(publicKeyPair.publicKey[1].get_str(10));

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
    // check if empty
    if (message.isEmpty()) {
        return;
    }

    // json parsing
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "Invalid JSON format.";
        return;
    }

    QJsonObject jsonObj = doc.object();
    QString sender = jsonObj["from"].toString();
    QString encryptedMessage = jsonObj["message"].toString();

    // additional check if empty info
    if (sender.isEmpty() || encryptedMessage.isEmpty()) {
        qDebug() << "Empty message or sender";
        return;
    }

    // try catch statement in case of error
    try {
        mpz_class encryptedContent(encryptedMessage.toStdString(), 10);
        string decryptedMessage = encryption::RSA_Decrypt(encryptedContent, user.getKeys());
        // TODO: put message contents into message object (currently just returns a string)
        qDebug() << "Sender:" << sender << "Decrypted message:" << QString::fromStdString(decryptedMessage);
    } catch (const std::exception &e) {
        qDebug() << "Error converting encrypted message or decrypting:" << e.what();
        return;
    }
}


// gets the requested user, including public key
User networking::getUser(const QString &username) {
    if (m_webSocket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject requestJson;
        requestJson["username"] = username;

        QJsonDocument jsonDoc(requestJson);
        QString request = QString::fromUtf8(jsonDoc.toJson());


        m_webSocket->sendTextMessage(request);
        qDebug() << "Requesting public key for user:" << username;

        // waits for a response (this part can cause crashes bc of my bad code)
        QEventLoop loop;
        QString responseMessage;
\
        connect(m_webSocket, &QWebSocket::textMessageReceived, [&loop, &responseMessage](const QString &message) {
            responseMessage = message;
            loop.quit();
        });

        loop.exec();

        // it didnt crash and a response was recieved
        qDebug() << "Response received: " << responseMessage;

        QJsonDocument responseDoc = QJsonDocument::fromJson(responseMessage.toUtf8());
        if (responseDoc.isNull()) {
            qDebug() << "Invalid JSON response";
            return User();
        }

        QJsonObject responseJson = responseDoc.object();

        // final error check (if unchecked it crashes so)
        if (responseJson.contains("error")) {
            qDebug() << "Error: " << responseJson["error"].toString();
            return User();
        }


        // extract information from json
        QString encryptionMethod = responseJson["encryptionMethod"].toString();
        QString regenDuration = responseJson["regenDuration"].toString();


        QString eStr = responseJson["publicKey"].toObject()["e"].toString();
        QString nStr = responseJson["publicKey"].toObject()["n"].toString();

        mpz_class e, n;
        e.set_str(eStr.toStdString(), 10);
        n.set_str(nStr.toStdString(), 10);

        RSA_keys keys;
        keys.publicKey[1] = e;
        keys.publicKey[0] = n;

        //returns user
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
