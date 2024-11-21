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
#include <QTimer>
#include <unordered_map>

QString host = "ws://localhost:8001";

// constructor
networking::networking(User &user, QObject *parent)
    : QObject(parent), user(user), webSocket(new QWebSocket()) {
    QUrl url(host);
    webSocket->open(url);

    // initialize public key cache
    cachedPublicKeys = std::unordered_map<QString, RSA_keys>();

    // set up websocket event connections
    connect(webSocket, &QWebSocket::connected, this, &networking::onConnected);
    connect(webSocket, &QWebSocket::disconnected, this, &networking::onDisconnected);
    connect(webSocket, &QWebSocket::errorOccurred, this, &networking::onError);
    connect(webSocket, &QWebSocket::textMessageReceived, this, &networking::onMessageReceived);
}

// destructor
networking::~networking() {
    delete webSocket;
}

// send message function
bool networking::sendMessage(const QString &recipient, Message &message, User user) {
    if (webSocket->state() == QAbstractSocket::ConnectedState) {
        // create message json
        QJsonObject messageJson;
        messageJson["sender"] = QString::fromStdString(user.getUsername());
        messageJson["recipient"] = recipient;

        // fetch the recipient's public key from the cache or request it if not available
        User recipientUser = getUser(recipient);
        if (recipientUser.getUsername() == "") {
            return false; // user not found
        }

        messageJson["message"] = QString::fromStdString(message.getEncryptedContent(recipientUser));
        QJsonDocument jsonDoc(messageJson);
        QString messageJsonString = QString::fromUtf8(jsonDoc.toJson());

        // send json
        webSocket->sendTextMessage(messageJsonString);
        qDebug() << "Sent message: " << messageJsonString;
        return true;
    } else {
        qDebug() << "Connection not established. Message was not sent!";
        return false;
    }
}

// function to cache the public key
void networking::cacheUserPublicKey(const QString &username, const RSA_keys &keys) {
    cachedPublicKeys[username] = keys;
}

// get user information (with public key if needed)
User networking::getUser(const QString &username) {
    // check if user is already in cache
    if (cachedPublicKeys.find(username) != cachedPublicKeys.end()) {
        // return cached user data
        return User(username.toStdString(), user.getEncryptionMethod(), user.getRegenDuration(), cachedPublicKeys[username]);
    }

    if (webSocket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject requestJson;
        requestJson["username"] = username;

        QJsonDocument jsonDoc(requestJson);
        QString request = QString::fromUtf8(jsonDoc.toJson());

        webSocket->sendTextMessage(request);
        qDebug() << "Requesting public key for user:" << username;

        // waits for a response (now with 50% less crashing)
        QEventLoop loop;
        QString responseMessage;

        // connect to the lambda and store the connection ID
        QMetaObject::Connection connection = connect(webSocket, &QWebSocket::textMessageReceived, [&loop, &responseMessage](const QString &message) {
            responseMessage = message;
            loop.quit();  // exit the event loop after receiving the message
        });

        // execute the event loop, waiting for the response
        loop.exec();

        // disconnect after receiving the response PREVENTING A CRASH
        disconnect(connection);

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

        // cache the public key for future use
        cacheUserPublicKey(username, keys);

        // return the user with the public key
        User requestedUser(username.toStdString(), encryptionMethod.toStdString(), regenDuration.toStdString(), keys);
        return requestedUser;

    } else {
        qDebug() << "Connection not established. Cannot request public key!";
        return User();
    }
}

// websocket event slots, these output on events (self explanatory)
void networking::onConnected() {
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

    // send the user data to the server
    webSocket->sendTextMessage(userJsonString);
    qDebug() << "Connected to" << host;
}

void networking::onDisconnected() {
    qDebug() << "WebSocket disconnected.";
}

void networking::onError(QAbstractSocket::SocketError error) {
    qDebug() << "WebSocket error:" << error;
}

void networking::onMessageReceived(const QString &message) {
    // check if the message is empty
    if (message.isEmpty()) {
        return;
    }

    // parse the message as json
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "Invalid JSON format.";
        return;
    }

    QJsonObject jsonObj = doc.object();
    QString sender = jsonObj["from"].toString();
    QString encryptedMessage = jsonObj["message"].toString();

    // additional checks for empty information
    if (sender.isEmpty() || encryptedMessage.isEmpty()) {
        qDebug() << "Empty message or sender";
        return;
    }

    // decrypt the message using the cached public key
    try {
        mpz_class encryptedContent(encryptedMessage.toStdString(), 10);
        string decryptedMessage = encryption::RSA_Decrypt(encryptedContent, user.getKeys());
        qDebug() << "Sender:" << sender << "Decrypted message:" << QString::fromStdString(decryptedMessage);
    } catch (const std::exception &e) {
        qDebug() << "Error converting encrypted message or decrypting:" << e.what();
        return;
    }
}
