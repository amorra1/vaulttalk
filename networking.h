#ifndef NETWORKING_H
#define NETWORKING_H

#include <QObject>
#include <QWebSocket>
#include "message.h"

class networking : public QObject {
    Q_OBJECT

public:
    // constructor
    explicit networking(QObject *parent = nullptr);
    // destructor
    ~networking();

    bool sendMessage(Message &message);

    /*
    constructor establishes a connection, but if the server is offline while that connection
    is made then users cannot send messages, this function just attempts to restablish that
    */
    int reconnect();

    // slots used for asynchronous events
private slots:

    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onMessageReceived(const QString &message);

private:

    QWebSocket *m_webSocket;
};

#endif
