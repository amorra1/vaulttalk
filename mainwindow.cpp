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
#include "encryption.h"

User* currentUser = nullptr;

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

    ui->settingsDisplay->addItem("Enryption Method: ");

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
    // User sender(ui->senderInput->text().toStdString(), "testPassword");
    User receiver(ui->receiverInput->text().toStdString(), "testPassword");
    std::string content = ui->messageInput->text().toStdString();

    Message msg(*currentUser, content);

    // QString text = QString::fromStdString(msg.getEncryptedContent());

    /*
     temporary placement of reconnect, maybe in the future the client should
     try to reconnect every X amount of time
     */
    // network.reconnect();


    if(network.sendMessage(msg, *currentUser) && !content.empty()){
        ui->messageDisplay->append("You: " + QString::fromStdString(content));
        ui->messageInput->clear();
    } else {
        QMessageBox::critical(nullptr, "Error", "Message failed to send.");
    }
}
void MainWindow::login() {
    //get inputs
    QString username = ui->usernameInput->text();
    QString password = ui->passwordInput->text();

    delete currentUser;
    //create user object and login, creating new keys every login right now as key storage on server is difficult
    RSA_keys keys = encryption::GenerateKeys();

    currentUser = new User(username.toStdString(), password.toStdString(), keys);
    currentUser->loginUser(*currentUser, [this](bool success) {
        if (success) {
            qDebug() << "Callback: Login was successful!";
            ui->stackedWidget->setCurrentIndex(1);
        }
    });

    ui->usernameLabel->setText(QString::fromStdString(currentUser->getUsername()));
    buildSettingsDisplay();
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
    QString username = ui->signUpUsernameInput->text();
    QString password = ui->signUpPasswordInput->text();

    //user input error handling
    if(ui->signUpPasswordInput->text() != ui->signUpConfirmPassusernameInput->text()){
        QMessageBox::critical(nullptr, "Error", "Passwords do not match");
        return;
    }else if(username.isEmpty()){
        QMessageBox::critical(nullptr, "Error", "Please enter a username");
        return;
    }else if(password.isEmpty()){
        QMessageBox::critical(nullptr, "Error", "Please enter a password");
        return;
    }else if (username.length() < 4){
        QMessageBox::critical(nullptr, "Error", "Username must be greater than 4 characters");
        return;
    } else if (username.length() > 12){
        QMessageBox::critical(nullptr, "Error", "Username must be less than 12 characters");
        return;
    } else if (password.length() < 6){
        QMessageBox::critical(nullptr, "Error", "Password must be greater than 6 characters");
        return;
    }

    //if passwords match, create user object and send data to server to be stored
    RSA_keys keys = encryption::GenerateKeys();

    currentUser = new User(username.toStdString(), password.toStdString(), keys);
    currentUser->registerUser(*currentUser);
}
void MainWindow::buildSettingsDisplay(){
    ui->settingsDisplay->clear();

    QString encryptionMethod = QString::fromStdString(currentUser->getEncryptionMethod());
    QString regenDuration = QString::fromStdString(currentUser->getRegenDuration());

    RSA_keys publicKeyPair = currentUser->getKeys();
    QString publicKey_n = QString::fromStdString(publicKeyPair.publicKey[0].get_str(16));
    QString publicKey_e = QString::fromStdString(publicKeyPair.publicKey[1].get_str(16));

    QLabel* encryptionMethodLabel = new QLabel("<b><u>Encryption Method:</u></b><br>");
    QLabel* regenDurationLabel = new QLabel("<b><u>Key Regeneration Period:</u></b><br>");
    QLabel* publicKeyLabel = new QLabel("<b><u>Public Key:</u></b><br>");
    QLabel* nLabel = new QLabel("<b><u>n Value:</u></b><br>" + publicKey_n);
    QLabel* eLabel = new QLabel("<b><u>e Value:</u></b><br>" + publicKey_e);

    encryptionMethodLabel->setWordWrap(true);
    regenDurationLabel->setWordWrap(true);
    publicKeyLabel->setWordWrap(true);

    QListWidgetItem* encryptionItem = new QListWidgetItem();
    ui->settingsDisplay->addItem(encryptionItem);
    ui->settingsDisplay->addItem(encryptionMethod);
    ui->settingsDisplay->setItemWidget(encryptionItem, encryptionMethodLabel);

    QListWidgetItem* regenDurationItem = new QListWidgetItem();
    ui->settingsDisplay->addItem(regenDurationItem);
    ui->settingsDisplay->addItem(regenDuration);
    ui->settingsDisplay->setItemWidget(regenDurationItem, regenDurationLabel);

    QListWidgetItem* publicKeyItem = new QListWidgetItem();
    ui->settingsDisplay->addItem(publicKeyItem);
    ui->settingsDisplay->setItemWidget(publicKeyItem, publicKeyLabel);

    QListWidgetItem* nValueItem = new QListWidgetItem();
    ui->settingsDisplay->addItem(nValueItem);
    ui->settingsDisplay->addItem(publicKey_n);
    ui->settingsDisplay->setItemWidget(nValueItem, nLabel);

    QListWidgetItem* eValueItem = new QListWidgetItem();
    ui->settingsDisplay->addItem(eValueItem);
    ui->settingsDisplay->addItem(publicKey_e);
    ui->settingsDisplay->setItemWidget(eValueItem, eLabel);
}

