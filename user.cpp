#include "user.h"
#include <iostream>
#include <unordered_map>
#include <functional> // for hash functions like SHA-256
#include <sstream>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QMessageBox>
#include <functional>

using namespace std;
User::User() : username(""), hashedPassword("") {}
User::User(string username, string password) : username(username), hashedPassword(hashPassword(password)) {}

// to be used once public and private key generation is implemented
// User::User() : username(""), hashedPassword(""), publicKey(""), privateKey("") {}
// User::User (string username, string password, string pubKey, string privKey)
//     : username(username), hashedPassword(hashPassword(password)), publicKey(pubKey), privateKey(privKey) {}

User::~User() {
    username.clear();
    hashedPassword.clear();
}

string User::getUsername() const { return this->username; }
// string User::getPublicKey() const { return publicKey; }
// string User::getPrivateKey() const { return privateKey; }

//need to add a hashing function such as sha-256, this is used for temp replacement right now
string User::hashPassword(const string &password) {
    // Example of simple hash function using std::hash (replace with a more secure one)
    hash<string> hasher;
    size_t hashedValue = hasher(password);
    stringstream ss;
    ss << hex << hashedValue; // Convert to hexadecimal string
    return ss.str();
}

// for registering
void  User::registerUser(User user) {
    QNetworkAccessManager* networkManager = new QNetworkAccessManager();

    //create json object
    QJsonObject json;
    json["username"] = QString::fromStdString(user.username);
    json["password"] = QString::fromStdString(user.hashedPassword);
    QJsonDocument jsonDoc(json);

    QNetworkRequest request(QUrl("http://127.0.0.1:8000/register"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = networkManager->post(request, jsonDoc.toJson());

    //reply
    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QMessageBox::information(nullptr, "Success", "User registered successfully!");
        } else {
            QMessageBox::critical(nullptr, "Error", "Failed to register: " + reply->errorString());
        }
        reply->deleteLater();
    });
}

// for logging in an existing user
void User::loginUser(User user, std::function<void(bool)> callback) {
    QNetworkAccessManager* networkManager = new QNetworkAccessManager();

    //create json object
    QJsonObject json;
    json["username"] = QString::fromStdString(user.username);
    json["password"] = QString::fromStdString(user.hashedPassword);
    QJsonDocument jsonDoc(json);

    QNetworkRequest request(QUrl("http://127.0.0.1:8000/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = networkManager->post(request, jsonDoc.toJson());

    //reply
    QAbstractSocket::connect(reply, &QNetworkReply::finished, [reply, callback]() {
        bool successCheck = false;
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "login success";
            successCheck = true;
        }
        reply->deleteLater();

        callback(successCheck);
    });
}
