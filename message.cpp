#include "message.h"
#include <iostream>

using namespace std;


//constructor
Message::Message(const User &from, const string &msgContent)
    : sender(from), timestamp(time(nullptr)) {
    content = msgContent;
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
    mpz_class encryptedContent(this->content);
    string decryptedMessage = encryption::RSA_Decrypt(encryptedContent, user.getKeys());

    return decryptedMessage;
}

void Message::displayMessage() const {
    cout << "From: " << sender.getUsername()
    << ", Timestamp: " << ctime(&timestamp)
    << ", Encrypted Content: " << content << endl;
}
