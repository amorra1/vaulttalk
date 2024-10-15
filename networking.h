#ifndef NETWORKING_H
#define NETWORKING_H

#include <QObject>
#include <QWebSocket>

class networking : public QObject {
    Q_OBJECT

public:
    explicit networking(QObject *parent = nullptr);
    ~networking();

    void sendMessage(const QString& message);

private slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);

private:
    QWebSocket *m_webSocket;
};

#endif
