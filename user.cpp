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
#include <QEventLoop>

using namespace std;

struct Contact {
    QString name;
    QString publicKeyN;
    QString publicKeyE;
};

// Default constructor
User::User() : username(""), hashedPassword(""), encryptionMethod(""), regenDuration("") {
}

// Constructor with username and password, default encryptionMethod and regenDuration
User::User(string username, string password)
    : username(username), hashedPassword(hashPassword(password)), encryptionMethod("RSA"), regenDuration("Never") {}

// Constructor with username, password, encryption method, and regeneration duration
User::User(string username, string password, string method, string duration)
    : username(username), hashedPassword(hashPassword(password)), encryptionMethod(method), regenDuration(duration) {}

// Constructor with username, password, and keys
User::User(string username, string password, RSA_keys keys)
    : username(username), hashedPassword(hashPassword(password)), encryptionMethod("RSA"), regenDuration("Never"), RSAKeys(keys) {}

User::User(string username, string encryptionMethod, string regenDuration, RSA_keys keys)
    : username(username), encryptionMethod(encryptionMethod), regenDuration(regenDuration), RSAKeys(keys) {}

User::~User() {
    //strings are automatically cleaned in c++ when delete is used
}

// Getter and setter for username
string User::getUsername() const { return this->username; }
void User::setUsername(string name) { this->username = name; }

// Getter and setter for password
string User::getPassword() const { return this->hashedPassword; }
void User::setPassword(string password) { this->hashedPassword = hashPassword(password); }

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

    QJsonObject json;
    json["username"] = QString::fromStdString(user.username);
    json["password"] = QString::fromStdString(user.hashedPassword);
    json["encryptionMethod"] = QString::fromStdString(user.encryptionMethod);
    json["regenDuration"] = QString::fromStdString(user.regenDuration);

    QJsonObject publicKey;
    publicKey["n"] = QString::fromStdString(user.RSAKeys.publicKey[0].get_str(16));
    publicKey["e"] = QString::fromStdString(user.RSAKeys.publicKey[1].get_str(16));

    QJsonObject privateKey;
    privateKey["n"] = QString::fromStdString(user.RSAKeys.privateKey[0].get_str(16));
    privateKey["d"] = QString::fromStdString(user.RSAKeys.privateKey[1].get_str(16));

    json["publicKey"] = publicKey;
    json["privateKey"] = privateKey;

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
void User::loginUser(User &user, std::function<void(bool)> callback) {
    QNetworkAccessManager* networkManager = new QNetworkAccessManager();

    QJsonObject json;
    json.insert("username", QString::fromStdString(user.username));
    json.insert("password", QString::fromStdString(user.hashedPassword));

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

                if (jsonObject.contains("encryptionMethod")) {
                    user.setEncryptionMethod(jsonObject["encryptionMethod"].toString().toStdString());
                }
                if (jsonObject.contains("regenDuration")) {
                    user.setRegenDuration(jsonObject["regenDuration"].toString().toStdString());
                }

                if (jsonObject.contains("publicKey")) {
                    QJsonObject publicKey = jsonObject["publicKey"].toObject();
                    user.RSAKeys.publicKey[0] = mpz_class(publicKey["n"].toString().toStdString(), 16);
                    user.RSAKeys.publicKey[1] = mpz_class(publicKey["e"].toString().toStdString(), 16);
                }
                if (jsonObject.contains("privateKey")) {
                    QJsonObject privateKey = jsonObject["privateKey"].toObject();
                    user.RSAKeys.privateKey[0] = mpz_class(privateKey["n"].toString().toStdString(), 16);
                    user.RSAKeys.privateKey[1] = mpz_class(privateKey["d"].toString().toStdString(), 16);
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
QList<User::Contact> User::getContactsList(const QString& username) {
    QNetworkAccessManager networkManager;

    // Construct the API endpoint URL
    QString url = QString("http://127.0.0.1:8000/get-contacts/%1").arg(username);

    // Correct construction of QUrl and QNetworkRequest
    QUrl requestUrl(url);
    QNetworkRequest request(requestUrl);  // Pass the QUrl to QNetworkRequest
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Sending the GET request
    QNetworkReply* reply = networkManager.get(request);

    // Blocking call to wait for the reply (since we're not using callbacks)
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QList<User::Contact> contactsList;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObject = jsonResponse.object();

        if (jsonObject.contains("contacts")) {
            QJsonObject contacts = jsonObject["contacts"].toObject();

            for (const QString& contactName : contacts.keys()) {
                QJsonObject contactInfo = contacts[contactName].toObject();
                QString publicKeyN = contactInfo["publicKey"].toObject()["n"].toString();
                QString publicKeyE = contactInfo["publicKey"].toObject()["e"].toString();

                contactsList.append({contactName, publicKeyN, publicKeyE});
            }
        }
    } else {
        QMessageBox::critical(nullptr, "Error", "Failed to retrieve contacts.");
    }

    reply->deleteLater();
    return contactsList;
}
void User::addContact(const QString& username, const QString& contactName) {
    QNetworkAccessManager* networkManager = new QNetworkAccessManager();

    // Send only the username and contact name to search and add the contact
    QNetworkRequest request(QUrl("http://127.0.0.1:8000/add-contact/" + username));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject contactJson;
    contactJson["contactName"] = contactName;

    QJsonDocument jsonDoc(contactJson);

    QNetworkReply* reply = networkManager->post(request, jsonDoc.toJson());

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QMessageBox::information(nullptr, "Success", "Contact added successfully!");
    } else {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObject = jsonResponse.object();
        QString errorMessage = jsonObject.contains("detail")
                                   ? jsonObject["detail"].toString()
                                   : "Failed to add contact due to an unknown error.";

        QMessageBox::critical(nullptr, "Error", errorMessage);
    }

    reply->deleteLater();
}




