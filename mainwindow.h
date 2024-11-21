#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include "networking.h"
#include "user.h"
#include "message.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void displayReceivedMessage(QString user, QString message);

private slots:
    void onSendButtonClicked();
    void goToMain();
    void goToSettings();
    void login();
    void logout();
    void Register();
    void registerUser();
    void buildSettingsDisplay();
    void buildSettingsPage();
    void changeUsername();
    void changePassword();
    void settingsChange();
    void saveChanges();
    void buildContactList();
    void addContact();
    void checkContact(QLineEdit *contactUsernameInput);

private:
    Ui::MainWindow *ui;
    QStackedWidget *stackedWidget;
    networking *network;
};

#endif // MAINWINDOW_H
