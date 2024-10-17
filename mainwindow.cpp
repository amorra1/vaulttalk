#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLineEdit>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->senderInput->setPlaceholderText("Sender");
    ui->receiverInput->setPlaceholderText("Reveiver");
    ui->messageInput->setPlaceholderText("Type message here");

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
