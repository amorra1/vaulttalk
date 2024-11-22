#include "message.h"
#include <iostream>

using namespace std;


//constructor
Message::Message(const User &from, const string &msgContent)
    : sender(from), timestamp(time(nullptr)) {
    content = msgContent;
}

const User& Message::getSender() const {
    return sender;
}

const string& Message::getContent() const {
    return content;
}

time_t Message::getTimestamp() const {
    return timestamp;
}

string Message::getEncryptedContent(const User &user) const {
    string method = user.getEncryptionMethod();
    string encryptedString = this->content;

    if (method == "RSA"){
        mpz_class encrypted = encryption::RSA_Encrypt(this->content, user.getKeys());
        encryptedString = encrypted.get_str(10);
    }
    else if (method == "AES"){
        // AES encryption call here
    }

    return encryptedString;
}

// string Message::getEncryptedContent(User user) {
//     return content;
// }

string Message::getDecryptedContent(const User &user) const {
    string method = user.getEncryptionMethod();
    string decryptedMessage = this->content;

    if (method == "RSA"){
        mpz_class encryptedContent(this->content);
        decryptedMessage = encryption::RSA_Decrypt(encryptedContent, user.getKeys());
    }
    else if (method == "AES"){
        // AES encryption call here
    }

    return decryptedMessage;
}

void Message::displayMessage() const {
    cout << "From: " << sender.getUsername()
    << ", Timestamp: " << ctime(&timestamp)
    << ", Encrypted Content: " << content << endl;
}
