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
#include <QCryptographicHash>
#include <QString>

using namespace std;

// Default constructor
User::User() : username(""), hashedPassword(""), encryptionMethod(""), regenDuration("") {}

// Constructor with username and password, default encryptionMethod and regenDuration
User::User(string username, string password)
    : username(username), hashedPassword(hashPassword(password)), encryptionMethod("RSA"), regenDuration("never") {}

// Constructor with username, password, encryption method, and regeneration duration
User::User(string username, string password, string method, string duration)
    : username(username), hashedPassword(hashPassword(password)), encryptionMethod(method), regenDuration(duration) {}

// Constructor with username, password, and keys
User::User(string username, string password, RSA_keys keys)
    : username(username), hashedPassword(hashPassword(password)), encryptionMethod("RSA"), regenDuration("never"), RSAKeys(keys) {}

User::~User() {
    //strings are automatically cleaned in c++ when delete is used
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

// Getter for publicKey (returns the full key as a pair [n, e])
std::pair<mpz_class, mpz_class> User::getPublicKey() const {
    return {this->RSAKeys.publicKey[0], this->RSAKeys.publicKey[1]};
}

// Setter for publicKey
void User::setPublicKey(const mpz_class& n, const mpz_class& e) {
    this->RSAKeys.publicKey[0] = n;
    this->RSAKeys.publicKey[1] = e;
}

// Getter for privateKey (returns the full key as a pair [n, d])
std::pair<mpz_class, mpz_class> User::getPrivateKey() const {
    return {this->RSAKeys.privateKey[0], this->RSAKeys.privateKey[1]};
}

// Setter for privateKey
void User::setPrivateKey(const mpz_class& n, const mpz_class& d) {
    this->RSAKeys.privateKey[0] = n;
    this->RSAKeys.privateKey[1] = d;
}

RSA_keys User::getKeys() const {
    return this->RSAKeys;
}

// Temporary hash function for password (replace with a secure hash later)
string User::hashPassword(const string &password) {
    // Convert std::string to QString
    QString qPassword = QString::fromStdString(password);

    // Convert the QString to QByteArray and hash it
    QByteArray passwordBytes = qPassword.toUtf8();
    QByteArray hashedBytes = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256);

    // Return the hash as a hexadecimal string
    return QString(hashedBytes.toHex()).toStdString();
}

// Register user function
void User::registerUser(User user) {
    QNetworkAccessManager* networkManager = new QNetworkAccessManager();

    // Create JSON object for the registration request
    QJsonObject json;
    json["username"] = QString::fromStdString(user.username);
    json["password"] = QString::fromStdString(user.hashedPassword);
    json["encryptionMethod"] = QString::fromStdString(user.encryptionMethod);
    json["regenDuration"] = QString::fromStdString(user.regenDuration);
    qDebug() << json;

    QJsonDocument jsonDoc(json);

    // Set up the network request to the register endpoint
    QNetworkRequest request(QUrl("http://127.0.0.1:8000/register"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Send the POST request
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
void User::loginUser(User &user, std::function<void(bool)> callback) {
    QNetworkAccessManager* networkManager = new QNetworkAccessManager();

    QJsonObject json;
    json.insert("username", QString::fromStdString(user.username));
    json.insert("password", QString::fromStdString(user.hashedPassword));
    json["encryptionMethod"] = QString::fromStdString(user.encryptionMethod);
    json["regenDuration"] = QString::fromStdString(user.regenDuration);
    qDebug() << json;
    QJsonDocument jsonDoc(json);

    QNetworkRequest request(QUrl("http://127.0.0.1:8000/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = networkManager->post(request, jsonDoc.toJson());

    QObject::connect(reply, &QNetworkReply::finished, [reply, callback, &user]() {
        bool successCheck = false;
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObject = jsonResponse.object();

            if (jsonObject.contains("msg") && jsonObject["msg"].toString() == "Login successful") {
                qDebug() << "Login successful";
                successCheck = true;

                // Retrieve encryptionMethod and regenDuration from the response
                if (jsonObject.contains("encryptionMethod")) {
                    user.setEncryptionMethod(jsonObject["encryptionMethod"].toString().toStdString());
                }
                if (jsonObject.contains("regenDuration")) {
                    user.setRegenDuration(jsonObject["regenDuration"].toString().toStdString());
                }
            }
        } else {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObject = jsonResponse.object();
            QString errorMessage = jsonObject.contains("detail")
                                       ? jsonObject["detail"].toString()
                                       : "Failed to login due to an unknown error.";

            QMessageBox::critical(nullptr, "Login Error", errorMessage);
        }

        reply->deleteLater();
        callback(successCheck);
    });
}




