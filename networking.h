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

    void sendMessage(Message &message);

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
