#include "message.h"
#include <iostream>

using namespace std;

//placeholder implementation
string encryptMessage(const string &message) {
    return "encrypted_" + message;
}

string decryptMessage(const string &encryptedMessage) {
    return encryptedMessage.substr(10); //removes prefix
}

//constructor
Message::Message(const User &from, const string &msgContent)
    : sender(from), timestamp(time(nullptr)) {
    content = encryptMessage(msgContent);
}

string Message::getEncryptedContent() const {
    return content;
}

string Message::getDecryptedContent(const string &privateKey) const {
    return decryptMessage(content);
}

void Message::displayMessage() const {
    cout << "From: " << sender.getUsername()
    << ", Timestamp: " << ctime(&timestamp)
    << ", Encrypted Content: " << content << endl;
}
