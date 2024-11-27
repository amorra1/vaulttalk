#ifndef NETWORKING_H
#define NETWORKING_H

#include <QObject>
#include <QWebSocket>
#include "message.h"
#include <unordered_map>

class networking : public QObject {
    Q_OBJECT

public:
    // constructor
    networking(User &user, QObject *parent = nullptr);
    // destructor
    ~networking();

    bool sendMessage(const QString &recipient, Message &message, User user);

    User getUser(const QString &username);

    /*
    constructor establishes a connection, but if the server is offline while that connection
    is made then users cannot send messages, this function just attempts to re-establish that
    */
    void reconnect();
    void closeConnection();

    // slots used for asynchronous events
private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onMessageReceived(const QString &message);

private:
    User &user;
    QWebSocket *webSocket;
    // cache for storing public keys of users
    std::unordered_map<QString, RSA_keys> cachedPublicKeys;

    // helper function to cache the public key of a user
    void cacheUserPublicKey(const QString &username, const RSA_keys &keys);

signals:
    // signal if fail or succeed
    void userRequestSucceeded(const User &user);
    void userRequestFailed(const QString &username);
    void messageReceived(QString sender, QString message);

};

#endif
