#include "message.h"
#include <iostream>

using namespace std;


//constructor
Message::Message(const User &from, const string &msgContent)
    : sender(from), timestamp(time(nullptr)) {
    content = msgContent;
}

string Message::getEncryptedContent(const User &user) const {
    mpz_class encrypted = encryption::RSA_Encrypt(this->content, user.getKeys());
    return encrypted.get_str(10);
}

// string Message::getEncryptedContent(User user) {
//     return content;
// }

string Message::getDecryptedContent(const mpz_class &encryptedMessage, User &user) const {
    string decrypted = encryption::RSA_Decrypt(encryptedMessage, user.getKeys());
    return decrypted;
}

void Message::displayMessage() const {
    cout << "From: " << sender.getUsername()
    << ", Timestamp: " << ctime(&timestamp)
    << ", Encrypted Content: " << content << endl;
}
