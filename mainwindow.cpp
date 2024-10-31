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
#include "user.h"

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
    User sender(ui->senderInput->text().toStdString(), "testPassword");
    User receiver(ui->receiverInput->text().toStdString(), "testPassword");
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
    //get inputs
    QString username = ui->usernameInput->text();
    QString password = ui->passwordInput->text();

    //create user object and login
    User user(username.toStdString(), password.toStdString());
    user.loginUser(user, [this](bool success) {
        if (success) {
            qDebug() << "Callback: Login was successful!";
            ui->stackedWidget->setCurrentIndex(1);
        } else {
            qDebug() << "Callback: Login failed.";
            QMessageBox::critical(this, "Error", "Login failed: ");
        }
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
    bool passwordMatch = false;
    QString username = ui->signUpUsernameInput->text();
    QString password = ui->signUpPasswordInput->text();

    //user input error handling
    if(ui->signUpPasswordInput->text() != ui->signUpConfirmPassusernameInput->text()){
        QMessageBox::critical(nullptr, "Error", "Passwords do not match");
    }else if(ui->signUpUsernameInput->text() == ""){
        QMessageBox::critical(nullptr, "Error", "Please enter a username");
    }else if(ui->signUpPasswordInput->text() == ""){
        QMessageBox::critical(nullptr, "Error", "Please enter a password");
    }else {
        passwordMatch = true;
    }

    //if passwords match, create user object and send data to server to be stored
    if (passwordMatch){
        User user(username.toStdString(), password.toStdString());
        user.registerUser(user);
    }
}
