#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    //, stackedWidget(new QStackedWidget(this))
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);

    //login page
    ui->usernameInput->setPlaceholderText("Username");
    ui->passwordInput->setPlaceholderText("Password");
    ui->passwordInput->setEchoMode(QLineEdit::Password);

    //home page
    ui->messageInput->setPlaceholderText("Type message here...");
    ui->senderInput->setPlaceholderText("Sender");
    ui->receiverInput->setPlaceholderText("Receiver");
    ui->messageDisplay->setReadOnly(true);

    //register page
    ui->signUpUsernameInput->setPlaceholderText("Username");
    ui->signUpPasswordInput->setPlaceholderText("Password");
    ui->signUpConfirmPassusernameInput->setPlaceholderText("Confirm Password");
    ui->signUpPasswordInput->setEchoMode(QLineEdit::Password);
    ui->signUpConfirmPassusernameInput->setEchoMode(QLineEdit::Password);


    // Connect buttons to switch between pages
    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::login);
    connect(ui->homeButton, &QPushButton::clicked, this, &MainWindow::goToMain);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::goToSettings);
    connect(ui->logOutButton, &QPushButton::clicked, this, &MainWindow::logout);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    connect(ui->signUpButton, &QPushButton::clicked, this, &MainWindow::Register);
    connect(ui->backToLoginButton, &QCommandLinkButton::clicked, this, &MainWindow::logout);
    connect(ui->registerButton, &QPushButton::clicked, this, &MainWindow::registerUser);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSendButtonClicked() {
    User sender("testSenderID", ui->senderInput->text().toStdString(), std::string("111"), std::string("hashedpassword1"));
    User receiver("testReceiverID", ui->receiverInput->text().toStdString(), std::string("222"), std::string("hashedpassword2"));
    std::string content = ui->messageInput->text().toStdString();

    Message msg(sender, receiver, content);

    // QString text = QString::fromStdString(msg.getEncryptedContent());

    network.sendMessage(msg);

    if(!content.empty()){
        ui->messageDisplay->append("You: " + QString::fromStdString(content));
        ui->messageInput->clear();
    }
}
void MainWindow::login() {
    QNetworkAccessManager* networkManager = new QNetworkAccessManager();

    QString username = ui->usernameInput->text();
    QString password = ui->passwordInput->text();

    //create json object
    QJsonObject json;
    json["username"] = username;
    json["password"] = password;
    QJsonDocument jsonDoc(json);

    QNetworkRequest request(QUrl("http://127.0.0.1:8000/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = networkManager->post(request, jsonDoc.toJson());

    //reply
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            ui->stackedWidget->setCurrentIndex(1);
        } else {
            QMessageBox::critical(this, "Error", "Login failed: " + reply->errorString());
        }
        reply->deleteLater();
    });
}
void MainWindow::goToMain() {
    ui->stackedWidget->setCurrentIndex(1);
}
void MainWindow::goToSettings() {
    ui->stackedWidget->setCurrentIndex(3);
}
void MainWindow::logout() {
    //clear register fields
    ui->signUpUsernameInput->clear();
    ui->signUpPasswordInput->clear();
    ui->signUpConfirmPassusernameInput->clear();

    // clear login fields unless remember me is checked
    ui->passwordInput->clear();
    if(!(ui->rememberMeCheck->isChecked())){
        ui->usernameInput->clear();
    }

    ui->stackedWidget->setCurrentIndex(0);
}
void MainWindow::Register() {
    ui->stackedWidget->setCurrentIndex(2);
}
void MainWindow::registerUser() {
    QNetworkAccessManager* networkManager = new QNetworkAccessManager();

    bool passwordMatch = false;
    QString username = ui->signUpUsernameInput->text();
    QString password = ui->signUpPasswordInput->text();
    if(ui->signUpPasswordInput->text() != ui->signUpConfirmPassusernameInput->text()){
        QMessageBox::critical(nullptr, "Error", "Passwords do not match");
    }else if(ui->signUpUsernameInput->text() == ""){
        QMessageBox::critical(nullptr, "Error", "Please enter a username");
    }else if(ui->signUpPasswordInput->text() == ""){
        QMessageBox::critical(nullptr, "Error", "Please enter a password");
    }else {
        passwordMatch = true;
    }

    if (passwordMatch){
        //create json object
        QJsonObject json;
        json["username"] = username;
        json["password"] = password;
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
}
