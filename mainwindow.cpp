#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    //, stackedWidget(new QStackedWidget(this))
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);

    ui->usernameInput->setPlaceholderText("Username");
    ui->passwordInput->setPlaceholderText("Password");
    ui->messageInput->setPlaceholderText("Type message here...");
    ui->senderInput->setPlaceholderText("Sender");
    ui->receiverInput->setPlaceholderText("Receiver");


    // Connect buttons to switch between pages
    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::login);
    connect(ui->homeButton, &QPushButton::clicked, this, &MainWindow::goToMain);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::goToSettings);
    connect(ui->logOutButton, &QPushButton::clicked, this, &MainWindow::logout);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSendButtonClicked() {
    User sender("testSenderID", ui->senderInput->text().toStdString(), "111");
    User receiver("testReceiverID", ui->receiverInput->text().toStdString(), "222");
    std::string content = ui->messageInput->text().toStdString();

    Message msg(sender, receiver, content);

    // QString text = QString::fromStdString(msg.getEncryptedContent());

    network.sendMessage(msg);
}
void MainWindow::login() {
    ui->stackedWidget->setCurrentIndex(1);
}
void MainWindow::goToMain() {
    ui->stackedWidget->setCurrentIndex(1);
}
void MainWindow::goToSettings() {
    ui->stackedWidget->setCurrentIndex(2);
}
void MainWindow::logout() {
    ui->stackedWidget->setCurrentIndex(0);
}
