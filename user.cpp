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
#include <QUrlQuery>

using namespace std;

struct Contact {
    QString name;
    QString publicKeyN;
    QString publicKeyE;
};

// Default constructor
User::User() : username(""), hashedPassword(""), encryptionMethod(""), regenDuration(""), caeserShiftValue(1){
}

User::User(string username) : username(username), hashedPassword(""), encryptionMethod(""), regenDuration("") {
}

User::User(string username, int value) : username(username), hashedPassword(""), encryptionMethod(""), regenDuration(""), caeserShiftValue(value) {
}

// Constructor with username and password, default encryptionMethod and regenDuration
User::User(string username, string password)
    : username(username), hashedPassword(hashPassword(password)), caeserShiftValue(1) {}

// Constructor with username, password, encryption method, and regeneration duration
User::User(string username, string password, string method, string duration)
    : username(username), hashedPassword(hashPassword(password)), encryptionMethod(method), regenDuration(duration), caeserShiftValue(1) {}

// Constructor with username, password, and keys
User::User(string username, string password, RSA_keys keys)
    : username(username), hashedPassword(hashPassword(password)), encryptionMethod("RSA"), regenDuration("Never"), RSAKeys(keys), lastKeyChanged(std::time(nullptr)), caeserShiftValue(1) {}

User::User(string username, string encryptionMethod, string regenDuration, RSA_keys keys, int value)
    : username(username), encryptionMethod(encryptionMethod), regenDuration(regenDuration), RSAKeys(keys), caeserShiftValue(value) {}

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
void User::setRegenDuration(string duration) {
    this->regenDuration = duration;
}

//Getter and Setter for lastKeyChanged
time_t User::getLastKeyChanged() const {
    return lastKeyChanged;
}
void User::setLastKeyChanged(time_t lastKeyChanged) {
    this->lastKeyChanged = lastKeyChanged;
}

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

// Getter and Setter for Caeser Shift Value
int User::getCaeserShiftValue() const {
    qDebug() << "getting shift value: ";
    qDebug() << this->caeserShiftValue;
    return this->caeserShiftValue;
}

void User::setCaeserShiftValue(int value){
    this->caeserShiftValue = value;
    qDebug() << "inside setter";
    qDebug() << this->caeserShiftValue;
}

void User::addRequest(QString user) {
    if (!requests.contains(user)) {
        this->requests.append(user); //appends the request from the user to the requests list if it doesnt already exist
        qDebug() << "added user: " + user;
    }
}

void User::removeRequest(QString user) {
    if (requests.contains(user)) {
        requests.removeAll(user); //if that user is in the requests list, remove it
    }
}

QList<QString> User::getRequests() const {
    return requests;
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

    //create json object and add necessary parameters
    QJsonObject json;
    json["username"] = QString::fromStdString(user.username);
    json["password"] = QString::fromStdString(user.hashedPassword);
    json["encryptionMethod"] = QString::fromStdString(user.encryptionMethod);
    json["regenDuration"] = QString::fromStdString(user.regenDuration);
    json["lastKeyChanged"] = QString::number(static_cast<qint64>(user.lastKeyChanged));

    QJsonObject publicKey;
    publicKey["n"] = QString::fromStdString(user.RSAKeys.publicKey[0].get_str(16));
    publicKey["e"] = QString::fromStdString(user.RSAKeys.publicKey[1].get_str(16));

    QJsonObject privateKey;
    privateKey["n"] = QString::fromStdString(user.RSAKeys.privateKey[0].get_str(16));
    privateKey["d"] = QString::fromStdString(user.RSAKeys.privateKey[1].get_str(16));

    json["publicKey"] = publicKey;
    json["privateKey"] = privateKey;

    QJsonDocument jsonDoc(json);
    \
        //make the api call
        QNetworkRequest request(QUrl("http://127.0.0.1:8000/register"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = networkManager->post(request, jsonDoc.toJson());

    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QMessageBox::information(nullptr, "Success", "User registered successfully!"); // on success, notify user
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

    //create the json object
    QJsonObject json;
    json.insert("username", QString::fromStdString(user.getUsername()));
    json.insert("password", QString::fromStdString(user.getPassword()));

    QJsonDocument jsonDoc(json);

    //make the api call
    QNetworkRequest request(QUrl("http://127.0.0.1:8000/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = networkManager->post(request, jsonDoc.toJson());

    QObject::connect(reply, &QNetworkReply::finished, [reply, callback, &user]() {
        bool successCheck = false;
        if (reply->error() == QNetworkReply::NoError) { // if success, parse the incoming info from server
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObject = jsonResponse.object();

            if (jsonObject.contains("msg") && jsonObject["msg"].toString() == "Login successful") { //if the server sends back info
                qDebug() << "Login successful";
                successCheck = true;

                //check for encryption method
                if (jsonObject.contains("encryptionMethod")) {
                    qDebug() << jsonObject["encryptionMethod"].toString().toStdString();
                    user.setEncryptionMethod(jsonObject["encryptionMethod"].toString().toStdString());
                }
                //check for regenDuration
                if (jsonObject.contains("regenDuration")) {
                    user.setRegenDuration(jsonObject["regenDuration"].toString().toStdString());
                }
                //check for lastKeyChanged
                if (jsonObject.contains("lastKeyChanged")) {
                    //have to convert back from int to time_t type
                    QString lastKeyChangedStr = jsonObject["lastKeyChanged"].toString();
                    bool conversionSuccessful = false;
                    time_t lastKeyChanged = static_cast<time_t>(lastKeyChangedStr.toLongLong(&conversionSuccessful));
                    qDebug() << "got time: " + lastKeyChangedStr;
                    if (conversionSuccessful) {
                        user.setLastKeyChanged(lastKeyChanged);
                    } else {
                        qDebug() << "Failed to convert lastKeyChanged to time_t";
                    }
                }
                //check for publicKey
                if (jsonObject.contains("publicKey")) {
                    QJsonObject publicKey = jsonObject["publicKey"].toObject();
                    user.setPublicKey(
                        mpz_class(publicKey["n"].toString().toStdString(), 16),
                        mpz_class(publicKey["e"].toString().toStdString(), 16)
                        );
                }
                //check for private key
                if (jsonObject.contains("privateKey")) {
                    QJsonObject privateKey = jsonObject["privateKey"].toObject();
                    user.setPrivateKey(
                        mpz_class(privateKey["n"].toString().toStdString(), 16),
                        mpz_class(privateKey["d"].toString().toStdString(), 16)
                        );
                }
            }
        } else { //if fails tell user
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
    //make the api call to get the the list of contacts from the server with username
    QString url = QString("http://127.0.0.1:8000/get-contacts/%1").arg(username);

    QUrl requestUrl(url);
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = networkManager.get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QList<User::Contact> contactsList;

    if (reply->error() == QNetworkReply::NoError) { //if success
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObject = jsonResponse.object();

        //check for contacts in api response
        if (jsonObject.contains("contacts")) {
            QJsonObject contacts = jsonObject["contacts"].toObject();

            //loop through all the contcts in the response
            for (const QString& contactName : contacts.keys()) {
                QJsonObject contactInfo = contacts[contactName].toObject();
                QString publicKeyN = contactInfo["publicKey"].toObject()["n"].toString();
                QString publicKeyE = contactInfo["publicKey"].toObject()["e"].toString();

                contactsList.append({contactName, publicKeyN, publicKeyE}); // append their info to the contacts list
            }
        }
    } else {
        QMessageBox::critical(nullptr, "Error", "Failed to retrieve contacts."); //if failed tell user
    }

    reply->deleteLater();
    return contactsList; //return the list
}
bool User::addContact(const QString& username, const QString& contactName) {
    QNetworkAccessManager* networkManager = new QNetworkAccessManager();

    //create api call with username as argument
    QUrl url("http://127.0.0.1:8000/add-contact/" + username);
    QUrlQuery query;
    query.addQueryItem("contactUsername", contactName);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = networkManager->post(request, QByteArray());

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) { //if error tell user
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObject = jsonResponse.object();
        QString errorMessage = jsonObject.contains("detail")
                                   ? jsonObject["detail"].toString()
                                   : "Failed to add contact due to an unknown error.";

        QMessageBox::critical(nullptr, "Error", errorMessage);
        return false;
    }
    return true;
    reply->deleteLater();
}

void User::regenerateKeys() {
    qDebug() << "Keys regenerated";
    this->RSAKeys = encryption::GenerateKeys();
    // write the time last changed
    this->lastKeyChanged = time(nullptr);

    QNetworkAccessManager* networkManager = new QNetworkAccessManager();

    //create json object to send new date and keys to server
    QJsonObject json;
    json["username"] = QString::fromStdString(this->username);
    json["lastKeyChanged"] = QString::number(static_cast<qint64>(this->lastKeyChanged)); //send as an int, just easier to understand on sever and json

    QJsonObject publicKey;
    publicKey["n"] = QString::fromStdString(this->RSAKeys.publicKey[0].get_str(16));
    publicKey["e"] = QString::fromStdString(this->RSAKeys.publicKey[1].get_str(16));

    QJsonObject privateKey;
    privateKey["n"] = QString::fromStdString(this->RSAKeys.privateKey[0].get_str(16));
    privateKey["d"] = QString::fromStdString(this->RSAKeys.privateKey[1].get_str(16));

    json["publicKey"] = publicKey;
    json["privateKey"] = privateKey;

    QJsonDocument jsonDoc(json);

    //make api call
    QUrl url("http://127.0.0.1:8000/update-keys");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = networkManager->post(request, jsonDoc.toJson());

    QObject::connect(reply, &QNetworkReply::finished, [reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Keys updated successfully!";
        } else {
            qDebug() << "Error updating keys:" << reply->errorString();
        }
        reply->deleteLater();
    });
}

void User::checkRegen() {
    // constants bc im lazy (one might call that good programming practice)
    const int DAY = 86400;
    const int WEEK = 604800;
    const int MONTH = 2678400;

    time_t current = time(nullptr);
    int timeDiff = current - this->lastKeyChanged;
    qDebug() << "Epoch time since last key change: " << timeDiff;

    // if difference is greater than regen keys
    if ((this->getRegenDuration() == "Per session") ||
        (this->getRegenDuration() == "Daily" && timeDiff >= DAY) ||
        (this->getRegenDuration() == "Weekly" && timeDiff >= WEEK) ||
        (this->getRegenDuration() == "Monthly" && timeDiff >= MONTH)) {
        this->regenerateKeys();
    }
}
