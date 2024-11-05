#include "user.h"
#include <iostream>
#include <functional>
#include <sstream>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QMessageBox>

using namespace std;

// Default constructor
User::User() : username(""), hashedPassword(""), encryptionMethod(""), regenDuration("") {}

// Constructor with username and password, default encryptionMethod and regenDuration
User::User(string username, string password)
    : username(username), hashedPassword(hashPassword(password)), encryptionMethod("RSA"), regenDuration("never") {}

// Constructor with username, password, encryption method, and regeneration duration
User::User(string username, string password, string method, string duration)
    : username(username), hashedPassword(hashPassword(password)), encryptionMethod(method), regenDuration(duration) {}

User::~User() {
    username.clear();
    hashedPassword.clear();
    encryptionMethod.clear();
    regenDuration.clear();
}

// Getter and setter for username
string User::getUsername() const { return this->username; }
void User::setUsername(string name) { this->username = name; }

// Getter and setter for encryptionMethod
string User::getEncryptionMethod() const { return this->encryptionMethod; }
void User::setEncryptionMethod(string method) { this->encryptionMethod = method; }

// Getter and setter for regenDuration
string User::getRegenDuration() const { return this->regenDuration; }
void User::setRegenDuration(string duration) { this->regenDuration = duration; }

// Temporary hash function for password (replace with a secure hash later)
string User::hashPassword(const string &password) {
    hash<string> hasher;
    size_t hashedValue = hasher(password);
    stringstream ss;
    ss << hex << hashedValue;
    return ss.str();
}

// Register user function
void User::registerUser(User user) {
    QNetworkAccessManager* networkManager = new QNetworkAccessManager();

    QJsonObject json;
    json["username"] = QString::fromStdString(user.username);
    json["password"] = QString::fromStdString(user.hashedPassword);
    json["encryptionMethod"] = QString::fromStdString(user.encryptionMethod);
    json["regenDuration"] = QString::fromStdString(user.regenDuration);
    QJsonDocument jsonDoc(json);

    QNetworkRequest request(QUrl("http://127.0.0.1:8000/register"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = networkManager->post(request, jsonDoc.toJson());

    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(nullptr, "Success", "User registered successfully!");
        } else {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObject = jsonResponse.object();
            QString errorMessage = jsonObject.contains("detail")
                                       ? jsonObject["detail"].toString()
                                       : "Failed to register due to an unknown error.";

            QMessageBox::critical(nullptr, "Registration Error", errorMessage);
        }
        reply->deleteLater();
    });
}

// Login user function
void User::loginUser(User user, std::function<void(bool)> callback) {
    QNetworkAccessManager* networkManager = new QNetworkAccessManager();

    QJsonObject json;
    json["username"] = QString::fromStdString(user.username);
    json["password"] = QString::fromStdString(user.hashedPassword);
    json["encryptionMethod"] = QString::fromStdString(user.encryptionMethod);
    json["regenDuration"] = QString::fromStdString(user.regenDuration);
    QJsonDocument jsonDoc(json);

    QNetworkRequest request(QUrl("http://127.0.0.1:8000/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = networkManager->post(request, jsonDoc.toJson());

    QObject::connect(reply, &QNetworkReply::finished, [reply, callback]() {
        bool successCheck = false;
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Login successful";
            successCheck = true;
        } else {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObject = jsonResponse.object();
            QString errorMessage = jsonObject.contains("detail")
                                       ? jsonObject["detail"].toString()
                                       : "Failed to login due to an unknown error.";

            QMessageBox::critical(nullptr, "Registration Error", errorMessage);;
        }
        reply->deleteLater();

        callback(successCheck);
    });
}
