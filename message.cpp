#include "message.h"
#include <iostream>

using namespace std;

//placeholder implementation
string encryptMessage(const string &message, const string &publicKey) {
    return "encrypted_" + message;
}

string decryptMessage(const string &encryptedMessage, const string &privateKey) {
    return encryptedMessage.substr(10); //removes prefix
}

//constructor
Message::Message(const User &from, const User &to, const string &msgContent)
    : sender(from), receiver(to), timestamp(time(nullptr)) {
    content = encryptMessage(msgContent, receiver.getPublicKey());
}

string Message::getEncryptedContent() const {
    return content;
}

string Message::getDecryptedContent(const string &privateKey) const {
    return decryptMessage(content, privateKey);
}

void Message::displayMessage() const {
    cout << "From: " << sender.getUsername()
    << ", To: " << receiver.getUsername()
    << ", Timestamp: " << ctime(&timestamp)
    << ", Encrypted Content: " << content << endl;
}
