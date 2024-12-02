#include "mainwindow.h"
#include "chatroom.h"
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
Chatroom* currentChatroom = nullptr;
std::vector<Chatroom> chatrooms;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , network(nullptr)
    //, stackedWidget(new QStackedWidget(this))
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui->stackedWidget->setStyleSheet("QStackedWidget { background-color:rgb(81,90,130);}");
    //login page
    ui->usernameInput->setPlaceholderText("Username");
    ui->passwordInput->setPlaceholderText("Password");
    ui->passwordInput->setEchoMode(QLineEdit::Password);

    //home page
    ui->messageInput->setPlaceholderText("Type message here...");
    ui->receiverInput->setPlaceholderText("Receiver");
    ui->messageDisplay->setReadOnly(true);

    //register page
    ui->signUpUsernameInput->setPlaceholderText("Username");
    ui->signUpPasswordInput->setPlaceholderText("Password");
    ui->signUpConfirmPassusernameInput->setPlaceholderText("Confirm Password");
    ui->signUpPasswordInput->setEchoMode(QLineEdit::Password);
    ui->signUpConfirmPassusernameInput->setEchoMode(QLineEdit::Password);

    //settings page
    ui->settingsPasswordBox->setEchoMode(QLineEdit::Password);
    ui->settingsUsernameBox->setReadOnly(true);
    ui->settingsPasswordBox->setReadOnly(true);
    ui->methodDropdown->addItems({"RSA", "AES", "ROT13", "Caesar Cipher", "ELEC376 Cipher", "None"});
    ui->regenDurationDropdown->addItems({"Never","Per session", "Daily", "Monthly", "Weekly"});
    ui->caesarShiftDropdown->addItems({"1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                                  "11", "12", "13", "14", "15", "16", "17", "18", "19",
                                  "20", "21", "22", "23", "24", "25"});
    ui->caesarShiftDropdown->setCurrentText("1");

    // Connect buttons to switch between pages
    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::login);
    connect(ui->homeButton, &QPushButton::clicked, this, &MainWindow::goToMain);
    connect(ui->settingsButton, &QPushButton::clicked, this, &MainWindow::goToSettings);
    connect(ui->logOutButton, &QPushButton::clicked, this, &MainWindow::logout);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    connect(ui->signUpButton, &QPushButton::clicked, this, &MainWindow::Register);
    connect(ui->backToLoginButton, &QCommandLinkButton::clicked, this, &MainWindow::logout);
    connect(ui->registerButton, &QPushButton::clicked, this, &MainWindow::registerUser);
    connect(ui->changeUsername, &QPushButton::clicked, this, &MainWindow::changeUsername);
    connect(ui->changePassword, &QPushButton::clicked, this, &MainWindow::changePassword);
    connect(ui->methodDropdown, &QComboBox::currentIndexChanged, this, &MainWindow::settingsChange);
    connect(ui->regenDurationDropdown, &QComboBox::currentIndexChanged, this, &MainWindow::settingsChange);
    connect(ui->saveChanges, &QPushButton::clicked, this, &MainWindow::saveChanges);
    connect(ui->addContact, &QPushButton::clicked, this, &MainWindow::addContact);
    connect(ui->toolButton, &QPushButton::clicked, this, &MainWindow::friendRequest);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/* NAVIGATION METHODS */
void MainWindow::goToMain() {
    buildSettingsDisplay();
    ui->stackedWidget->setCurrentIndex(1);
}
void MainWindow::goToSettings() {
    ui->stackedWidget->setCurrentIndex(3);
}
void MainWindow::Register() {
    ui->stackedWidget->setCurrentIndex(2);
}

/* SEND AND RECEIVE MESSAGE METHODS */
void MainWindow::onSendButtonClicked() {
    // User sender(ui->senderInput->text().toStdString(), "testPassword");
    // User receiver(ui->senderInput->text().toStdString(), "testPassword");

    std::string receiverUsername = ui->receiverInput->text().toStdString();
    std::string content = ui->messageInput->text().toStdString();

    if(content.length()>255){
        QMessageBox::critical(nullptr, "Error", "Input length exceeds allowed character limit");
    }

    if (receiverUsername.empty() || content.empty()) {
        QMessageBox::critical(nullptr, "Error", "Receiver and message content cannot be empty.");
        return;
    }

    //search for if a chatroom exists already
    auto it = std::find_if(chatrooms.begin(), chatrooms.end(),
                           [&receiverUsername](const Chatroom& chatroom) {
                               return chatroom.getName() == receiverUsername;
                           });

    // initalize a charoom and msg object
    Chatroom* chatroom = nullptr;
    Message msg(*currentUser, content);

    if (it != chatrooms.end()) { //if exists, set chatroom to that address
        chatroom = &(*it);
    } else { //otherwise create new chatroom
        Chatroom newChatroom(receiverUsername, currentUser->getUsername(), receiverUsername);
        chatrooms.push_back(newChatroom); //add to chatrooms list
        chatroom = &chatrooms.back();
    }

    // QString text = QString::fromStdString(msg.getEncryptedContent());

    /*
     temporary placement of reconnect, maybe in the future the client should
     try to reconnect every X amount of time
     */
    //network->reconnect();
    currentChatroom = chatroom;

    // try to send the message
    if(network->sendMessage(ui->receiverInput->text(), msg, *currentUser) && !content.empty()){
        chatroom->addMessage(msg); //add message to list of the current chatroom

        QTextCursor cursor = ui->messageDisplay->textCursor();
        QTextBlockFormat blockFormat = cursor.blockFormat();

        //create a format to properly display time stamp on right side
        QList<QTextOption::Tab> tabPositions;
        QTextOption::Tab rightTab;
        rightTab.type = QTextOption::RightTab;
        rightTab.position = ui->messageDisplay->width() - 10;
        tabPositions.append(rightTab);

        blockFormat.setTabPositions(tabPositions);
        cursor.setBlockFormat(blockFormat);

        //get the timestamp from the messages
        time_t tempTimestamp = msg.getTimestamp();
        QString timestamp = QString::fromStdString(std::string(std::ctime(&tempTimestamp))).trimmed();

        //display the user's name in green
        QTextCharFormat userFormat;
        userFormat.setForeground(Qt::green);
        QString userLabel = QString::fromStdString(currentUser->getUsername()) + ": ";
        cursor.insertText(userLabel, userFormat);

        //display the message content in white
        QTextCharFormat messageFormat;
        messageFormat.setForeground(Qt::white);
        cursor.insertText(QString::fromStdString(content), messageFormat);

        //tab over and display timestamp in gray
        cursor.insertText("\t");
        QTextCharFormat timestampFormat;
        timestampFormat.setForeground(Qt::gray);
        cursor.insertText("[" + timestamp + "]", timestampFormat);

        cursor.insertText("\n");

        ui->messageDisplay->setTextCursor(cursor);

        ui->messageInput->clear(); //clear message input box
    } else {
        QMessageBox::critical(nullptr, "Error", "Message failed to send.");
    }
}
void MainWindow::displayReceivedMessage(QString user, QString message){
    qDebug() << "signal hit";
    qDebug() << user;

    //search for chatroom if it already exists
    auto it = std::find_if(chatrooms.begin(), chatrooms.end(),
                           [&user](const Chatroom& chatroom) {
                               return chatroom.getName() == user.toStdString();
                           });

    Chatroom* chatroom = nullptr;

    if (it != chatrooms.end()) { //if chatroom exists, set current to address of it
        chatroom = &(*it);
    } else { //otherwise, create a new chatroom
        Chatroom newChatroom(user.toStdString(), currentUser->getUsername(), user.toStdString());
        chatrooms.push_back(newChatroom); // add to list of chatrooms
        chatroom = &chatrooms.back();
    }

    User tempUser(user.toStdString()); //create a temporary user to be able to create a message object
    Message msg(tempUser, message.toStdString());

    //add the message to the chatroom's chatlog
    chatroom->addMessage(msg);

    if(chatroom == currentChatroom){ //if the current chatroom on the screen is the chatroom we just created then display the message
        QTextCursor cursor = ui->messageDisplay->textCursor();
        QTextBlockFormat blockFormat = cursor.blockFormat();

        //create a tab formatting
        QList<QTextOption::Tab> tabPositions;
        QTextOption::Tab rightTab;
        rightTab.type = QTextOption::RightTab;
        rightTab.position = ui->messageDisplay->width() - 10;
        tabPositions.append(rightTab);

        blockFormat.setTabPositions(tabPositions);
        cursor.setBlockFormat(blockFormat);

        //get the timestamp
        time_t tempTimestamp = msg.getTimestamp();
        QString timestamp = QString::fromStdString(std::string(std::ctime(&tempTimestamp))).trimmed();

        //get the user who sent and display their name in yellow
        QTextCharFormat userFormat;
        userFormat.setForeground(Qt::yellow);
        cursor.insertText(user + ": ", userFormat);

        //display the message content in white
        QTextCharFormat messageFormat;
        messageFormat.setForeground(Qt::white);
        cursor.insertText(message, messageFormat);

        //tab over and display timestamp in gray
        cursor.insertText("\t");
        QTextCharFormat timestampFormat;
        timestampFormat.setForeground(Qt::gray);
        cursor.insertText("[" + timestamp + "]", timestampFormat);

        cursor.insertText("\n");

        ui->messageDisplay->setTextCursor(cursor);
    } else{
        //otherwise, notify the user
        notificationReceived(user);
    }

}

/* REGISTER, LOGIN, LOGOUT METHODS */
void MainWindow::login() {
    //get inputs
    QString username = ui->usernameInput->text();
    QString password = ui->passwordInput->text();

    //ensure currentUser is wiped
    delete currentUser;

    //reinitialize current user
    currentUser = new User(username.toStdString(), password.toStdString());
    currentUser->loginUser(*currentUser, [this](bool success) { //log them in and wait for a callback
        if (success) {
            qDebug() << "Callback: Login was successful!";
            ui->stackedWidget->setCurrentIndex(1);
            // update network user
            network = new networking(*currentUser);

            // check the regeneration period
            currentUser->checkRegen();

            //connect the network now that its been initialized to displaying received messages
            connect(network, &networking::messageReceived, this, &MainWindow::displayReceivedMessage);

            //set the label at top of UI to their username
            ui->usernameLabel->setText(QString::fromStdString(currentUser->getUsername()));
            //call methods to build necessary UI elements
            buildSettingsDisplay();
            buildSettingsPage();
            buildContactList();
        }
    });
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

    //clear necessary UI elements
    ui->messageDisplay->clear();
    ui->receiverInput->clear();
    ui->settingsDisplay->clear();
    ui->messageInput->clear();

    //clear contacts widget
    QWidget *container = ui->scrollArea->widget();
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(container->layout());
    if (layout) {
        while (QLayoutItem *item = layout->takeAt(0)) {
            if (QWidget *widget = item->widget()) {
                widget->deleteLater();
            }
            delete item;
        }
    }

    chatrooms.clear();

    ui->stackedWidget->setCurrentIndex(0);
}
void MainWindow::registerUser() {
    //get inputs
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

    //initalize user and call register method
    currentUser = new User(username.toStdString(), password.toStdString(), keys);
    currentUser->registerUser(*currentUser);
}

/* QUICK VIEW SETTINGS METHODS */
void MainWindow::buildSettingsDisplay(){
    //initally clear the display incase anything remains
    ui->settingsDisplay->clear();

    //get the encryption method and regen duration
    QString encryptionMethod = QString::fromStdString(currentUser->getEncryptionMethod());
    qDebug() << "settings display" + currentUser->getEncryptionMethod();
    QString regenDuration = QString::fromStdString(currentUser->getRegenDuration());

    //get the last key changed
    time_t lastKeyChanged = currentUser->getLastKeyChanged();
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&lastKeyChanged));
    QString lastKeyChangedStr = QString::fromStdString(buffer);

    //get the public key
    RSA_keys publicKeyPair = currentUser->getKeys();
    QString publicKey_n = QString::fromStdString(publicKeyPair.publicKey[0].get_str(16));
    QString publicKey_e = QString::fromStdString(publicKeyPair.publicKey[1].get_str(16));

    //build html labels, bolded and underlined
    QLabel* encryptionMethodLabel = new QLabel("<b><u>Encryption Method:</u></b><br>");
    QLabel* regenDurationLabel = new QLabel("<b><u>Key Regeneration Period:</u></b><br>");
    QLabel* lastKeyChangedLabel = new QLabel("<b><u>Last Key Changed:</u></b><br>" + lastKeyChangedStr);
    // QLabel* publicKeyLabel = new QLabel("<b><u>Public Key:</u></b><br>");
    // QLabel* nLabel = new QLabel("<b><u>n Value:</u></b><br>" + publicKey_n);
    // QLabel* eLabel = new QLabel("<b><u>e Value:</u></b><br>" + publicKey_e);

    encryptionMethodLabel->setWordWrap(true);
    regenDurationLabel->setWordWrap(true);
    lastKeyChangedLabel->setWordWrap(true);
    // publicKeyLabel->setWordWrap(true);

    //add items to display
    QListWidgetItem* encryptionItem = new QListWidgetItem();
    ui->settingsDisplay->addItem(encryptionItem);
    ui->settingsDisplay->addItem(encryptionMethod);
    ui->settingsDisplay->setItemWidget(encryptionItem, encryptionMethodLabel);

    QListWidgetItem* regenDurationItem = new QListWidgetItem();
    ui->settingsDisplay->addItem(regenDurationItem);
    ui->settingsDisplay->addItem(regenDuration);
    ui->settingsDisplay->setItemWidget(regenDurationItem, regenDurationLabel);

    QListWidgetItem* lastKeyChangedItem = new QListWidgetItem();
    ui->settingsDisplay->addItem(lastKeyChangedItem);
    ui->settingsDisplay->addItem(lastKeyChangedStr);
    ui->settingsDisplay->setItemWidget(lastKeyChangedItem, lastKeyChangedLabel);

    // QListWidgetItem* publicKeyItem = new QListWidgetItem();
    // ui->settingsDisplay->addItem(publicKeyItem);
    // ui->settingsDisplay->setItemWidget(publicKeyItem, publicKeyLabel);

    // QListWidgetItem* nValueItem = new QListWidgetItem();
    // ui->settingsDisplay->addItem(nValueItem);
    // ui->settingsDisplay->addItem(publicKey_n);
    // ui->settingsDisplay->setItemWidget(nValueItem, nLabel);

    // QListWidgetItem* eValueItem = new QListWidgetItem();
    // ui->settingsDisplay->addItem(eValueItem);
    // ui->settingsDisplay->addItem(publicKey_e);
    // ui->settingsDisplay->setItemWidget(eValueItem, eLabel);
}

/* SETTINGS PAGE METHODS */
void MainWindow::buildSettingsPage(){
    //set default texts in settings page to user settings
    ui->settingsUsernameBox->setText(QString::fromStdString(currentUser->getUsername()));
    ui->settingsPasswordBox->setText(QString::fromStdString(currentUser->getPassword()));
    ui->methodDropdown->setCurrentText(QString::fromStdString(currentUser->getEncryptionMethod()));
    ui->regenDurationDropdown->setCurrentText(QString::fromStdString(currentUser->getRegenDuration()));
}
void MainWindow::changeUsername(){
    //when the user hits change username, allow them to edit the field
    ui->settingsUsernameBox->setReadOnly(false);
    ui->settingsUsernameBox->setFocus();
    settingsChange(); //call settings change
}
void MainWindow::changePassword(){
    //when user hits change password, allow them to edit the field
    ui->settingsPasswordBox->setReadOnly(false);
    ui->settingsPasswordBox->setFocus();
    settingsChange(); //call settings change
}
void MainWindow::settingsChange(){
    //sets the colour of the button to red to notify user a change has been made
    ui->saveChanges->setStyleSheet("QPushButton { background-color: red; color: white; }");
}
void MainWindow::saveChanges() {
    ui->saveChanges->setStyleSheet(""); //reset colour of save changes button

    //get all the parameters
    QString oldUsername = QString::fromStdString(currentUser->getUsername());
    QString newUsername = ui->settingsUsernameBox->text();
    QString password = ui->settingsPasswordBox->text();
    QString encryptionMethod = ui->methodDropdown->currentText();
    QString regenDuration = ui->regenDurationDropdown->currentText();
    QString newShiftValue = ui->caesarShiftDropdown->currentText();

    //build the json object
    QJsonObject json;
    json["oldUsername"] = oldUsername;
    json["newUsername"] = newUsername;
    json["password"] = password;
    json["encryptionMethod"] = encryptionMethod;
    json["regenDuration"] = regenDuration;

    QJsonDocument jsonDoc(json);
    QByteArray jsonData = jsonDoc.toJson();

    //make the api call
    QUrl url("http://127.0.0.1:8000/update-settings");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QNetworkReply *reply = manager->post(request, jsonData);

    //on reply from api...
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            //if success, update all user info accordingly to new info
            QMessageBox::information(this, "Success", "User settings updated successfully.");
            qDebug() << "settings saved" + ui->methodDropdown->currentText().toStdString();
            currentUser->setUsername(ui->settingsUsernameBox->text().toStdString());
            currentUser->setPassword(currentUser->hashPassword(ui->settingsPasswordBox->text().toStdString()));
            currentUser->setEncryptionMethod(ui->methodDropdown->currentText().toStdString());
            currentUser->setRegenDuration(ui->regenDurationDropdown->currentText().toStdString());
            currentUser->setCaeserShiftValue(ui->caesarShiftDropdown->currentText().toInt());
        } else {
            //if failed, notify user
            QString errorMessage = reply->errorString();
            QMessageBox::critical(this, "Error", "Failed to update user settings: " + errorMessage);
        }

        // reconnect to the server (sends any new encrpyption changes)
        network->reconnect();

        reply->deleteLater();
    });

}

/* CONTACT METHODS */
void MainWindow::buildContactList(){
    //create a container and layout to format the contacts list
    QWidget *container = ui->scrollArea->widget();
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(container->layout());

    if (!layout) {
        layout = new QVBoxLayout(container);
        layout->setAlignment(Qt::AlignTop); //align contacts top to bottom
        container->setLayout(layout);
    }

    //build a list to iterate over and get list
    QList<User::Contact> contactsList = currentUser->getContactsList(QString::fromStdString(currentUser->getUsername()));

    //iterate over the list and build a button with the text as the contacts username
    for (int i = 0; i < contactsList.size(); ++i) {
        QPushButton *button = new QPushButton(contactsList[i].name, container);
        layout->addWidget(button);

        // connect to insert name into receiver field when clicked and clear the notifcation badge
        QObject::connect(button, &QPushButton::clicked, this, [this, button, name = contactsList[i].name]() {
            clearNotificationBadge(button);
            insertReceiver(name);
        });
    }
}
void MainWindow::addContactToList(QString name){
    //create a container and layout to format the contacts list
    QWidget *container = ui->scrollArea->widget();
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(container->layout());

    //create a new button
    QPushButton *button = new QPushButton(name, container);
    layout->addWidget(button);

    // connect to insert name into receiver field when clicked
    QObject::connect(button, &QPushButton::clicked, this, [this, name = name]() {
        insertReceiver(name);
    });
}
void MainWindow::addContact() {
    //new dialog window to popup
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Add Contact");

    //contact name to be entered
    QLineEdit *contactUsernameInput = new QLineEdit(dialog);
    contactUsernameInput->setPlaceholderText("Enter username to add");

    //button to add entered name
    QPushButton *addButton = new QPushButton("Add", dialog);

    //cancel button
    QPushButton *cancelButton = new QPushButton("Cancel", dialog);

    //add elements
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(contactUsernameInput);
    layout->addWidget(addButton);
    layout->addWidget(cancelButton);

    dialog->setLayout(layout);

    connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::accept);

    connect(addButton, &QPushButton::clicked, this, [this, dialog, contactUsernameInput]() {
        QString contactUsername = contactUsernameInput->text();

        //if the user didnt enter a username
        if (contactUsername.isEmpty()) {
            QMessageBox::critical(this, "Error", "Please enter a username.");
        } else {
            checkContact(contactUsernameInput); //check if the user actually exists
        }

        dialog->accept();
    });

    dialog->exec();
}
void MainWindow::checkContact(QLineEdit *contactUsernameInput) {
    QString contactName = contactUsernameInput->text(); //get the username
    qDebug() << contactName;

    //call addContact which makes api call
    if(currentUser->addContact(QString::fromStdString(currentUser->getUsername()), contactName)){
        addContactToList(contactName); // if success, add it to the display and list
    }
}
void MainWindow::insertReceiver(QString name){
    //clear if anything is there and reset the name
    ui->messageDisplay->clear();
    ui->receiverInput->setText(name);

    //find the chatroom
    Chatroom* chatroom = findChatroom(name.toStdString());
    if(chatroom){ //if it exists, set current chatroom to it and switch to it
        currentChatroom = chatroom;
        switchToChatroom(name.toStdString());
    } else { //if doesnt exist, create new one
        createChatroom(name.toStdString());
    }

}
void MainWindow::notificationReceived(QString user) {
    //get the scroll area where the contacts are
    QWidget *scrollAreaWidget = ui->scrollArea->widget();
    if (!scrollAreaWidget) return; //if an issue return

    //get the contact buttons
    QList<QPushButton *> buttons = scrollAreaWidget->findChildren<QPushButton *>();
    bool userFound = false; //flah for if contact is found

    //loop through buttons and add a notification to that button
    for (QPushButton *button : buttons) {
        if (button->text() == user) {
            addNotificationBadge(button, 1);
            userFound = true; //set flag to true
            break;
        }
    }

    if (!userFound) { //if user isnt found, add notification to requests button
        currentUser->addRequest(user); //add request to the users request list
        addNotificationBadge(ui->toolButton, 1);
    }
}
void MainWindow::addNotificationBadge(QWidget *widget, int notificationCount) {
    //create a style for the badge label
    QLabel *badgeLabel = new QLabel(widget);
    badgeLabel->setText(QString::number(notificationCount));
    badgeLabel->setStyleSheet("background-color: red; color: white; "
                              "border-radius: 8px; font-weight: bold; "
                              "padding: 1px; min-width: 16px; text-align: center;");
    badgeLabel->setAlignment(Qt::AlignCenter);

    QRect widgetRect = widget->rect();
    int badgeSize = 16;
    badgeLabel->setGeometry(widgetRect.width() - badgeSize, 2, badgeSize, badgeSize);
    badgeLabel->show();
}

void MainWindow::clearNotificationBadge(QWidget *widget) {
    QList<QLabel *> labels = widget->findChildren<QLabel *>();
    for (QLabel *label : labels) {
        label->deleteLater();
    }
}
void MainWindow::friendRequest() {
    //get the requests from the users list
    QList<QString> requestList = currentUser->getRequests();
    if (requestList.isEmpty()) { //if empty tell user
        QMessageBox::information(this, "Friend Requests", "No friend requests at the moment.");
        return;
    }

    //otherwise, create a new dialog pop up
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Friend Requests");
    QVBoxLayout *layout = new QVBoxLayout(dialog);

    QMap<QString, QHBoxLayout*> requestLayouts;

    //loop through requests
    for (const auto& request : requestList) {
        QHBoxLayout *requestLayout = new QHBoxLayout();

        //create labels and buttons necessary
        QLabel *nameLabel = new QLabel(request, dialog);
        requestLayout->addWidget(nameLabel);

        QPushButton *addButton = new QPushButton("Add Friend", dialog);
        requestLayout->addWidget(addButton);

        QPushButton *declineButton = new QPushButton("Decline", dialog);
        requestLayout->addWidget(declineButton);

        layout->addLayout(requestLayout);
        requestLayouts[request] = requestLayout;

        //when the user clicks to accept
        connect(addButton, &QPushButton::clicked, [this, dialog, &requestList, &requestLayouts, request, layout]() {
            currentUser->addContact(QString::fromStdString(currentUser->getUsername()), request); //add the contact
            QMessageBox::information(dialog, "Friend Added", request + " has been added as a friend."); //notify user they added the contact

            requestList.removeOne(request); //remove the request from the request list since its been accepted now
            delete requestLayouts[request];
            requestLayouts.remove(request);
            addContactToList(request); // add contact to contacts list

            if (requestList.isEmpty()) { //if empty, dont display pop up
                dialog->accept();
            }
        });

        //when the user clicks to decline
        connect(declineButton, &QPushButton::clicked, [dialog, &requestList, &requestLayouts, request, layout]() {
            QMessageBox::information(dialog, "Friend Declined", request + "'s request has been declined."); //notify user they declined

            requestList.removeOne(request); //remove the request
            delete requestLayouts[request];
            requestLayouts.remove(request);

            if (requestList.isEmpty()) { //if empty, dont display popup
                dialog->accept();
            }
        });

        currentUser->removeRequest(request); //remove the request regardless of if they hit accept or decline
    }

    clearNotificationBadge(ui->toolButton); //clear the notifcation badge requests tab

    dialog->setLayout(layout);
    dialog->exec();
}

/* CHATROOM METHODS */
Chatroom* MainWindow::findChatroom(const std::string& name) {
    //loop through chatrooms and return if found
    for (auto& chatroom : chatrooms) {
        if (chatroom.getName() == name) {
            return &chatroom;
        }
    }
    return nullptr; //if not found reutnr null
}
void MainWindow::createChatroom(const std::string& name) {
    if (!findChatroom(name)) { //if chatroom does not already exist
        chatrooms.emplace_back(name);
    }
}
void MainWindow::switchToChatroom(const std::string& chatroomName) {
    Chatroom* chatroom = findChatroom(chatroomName); //find the chatroom
    if (!chatroom) {
        QMessageBox::critical(this, "Error", "Chatroom not found."); //if it doesnt exist, notify user
        return;
    }

    ui->messageDisplay->clear(); //clear the message display
    const auto& messages = chatroom->getChatLog(); //get the chatlogs

    QTextCursor cursor = ui->messageDisplay->textCursor();
    QTextBlockFormat blockFormat = cursor.blockFormat();

    //create tab format for timestamps
    QList<QTextOption::Tab> tabPositions;
    QTextOption::Tab rightTab;
    rightTab.type = QTextOption::RightTab;
    rightTab.position = ui->messageDisplay->width() - 10;
    tabPositions.append(rightTab);

    blockFormat.setTabPositions(tabPositions);
    cursor.setBlockFormat(blockFormat);

    //loop through chatlog we got above
    for (const auto& msg : messages) {
        //get the user info and content
        QString user = QString::fromStdString(msg.getSender().getUsername());
        QString content = QString::fromStdString(msg.getContent());

        //get the timestamp
        time_t tempTimestamp = msg.getTimestamp();
        QString timestamp = QString::fromStdString(std::string(std::ctime(&tempTimestamp))).trimmed();

        //display the user name in yellow
        QTextCharFormat userFormat;
        userFormat.setForeground(Qt::yellow);
        cursor.insertText(user + ": ", userFormat);

        //display the message in white
        QTextCharFormat messageFormat;
        messageFormat.setForeground(Qt::white);
        cursor.insertText(content, messageFormat);

        //tab over and display the timestamp in gray
        cursor.insertText("\t");
        QTextCharFormat timestampFormat;
        timestampFormat.setForeground(Qt::gray);
        cursor.insertText("[" + timestamp + "]", timestampFormat);

        cursor.insertText("\n");

        ui->messageDisplay->setTextCursor(cursor);
    }
}


