#include "message.h"
#include <iostream>

using namespace std;

string decryptMessage(const string &encryptedMessage) {
    return encryptedMessage.substr(10); //removes prefix
}

//constructor
Message::Message(const User &from, const string &msgContent)
    : sender(from), timestamp(time(nullptr)) {
    content = msgContent;
}

string Message::getEncryptedContent(User user) {
    mpz_class encrypted = encryption::RSA_Encrypt(this->content, user.getKeys());
    return encrypted.get_str(16);
}

// string Message::getEncryptedContent(User user) {
//     return content;
// }

string Message::getDecryptedContent(const string &privateKey) const {
    return decryptMessage(content);
}

void Message::displayMessage() const {
    cout << "From: " << sender.getUsername()
    << ", Timestamp: " << ctime(&timestamp)
    << ", Encrypted Content: " << content << endl;
}
