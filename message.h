#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <ctime>
#include "user.h"

using namespace std;

class Message {
private:
    User sender;
    User receiver;
    string content;  //actual content of the message
    time_t timestamp;

public:
    Message(const User &from, const User &to, const string &msgContent);

    string getEncryptedContent() const;
    string getDecryptedContent(const string &privateKey) const;

    void displayMessage() const;
};

//to be implemented in encryption folder, these methods may just call other methods to encrypt/decrypt that
//way we can call encrypt on the message content and decrypt on the message content
string encryptMessage(const string &message, const string &publicKey);
string decryptMessage(const string &encryptedMessage, const string &privateKey);

#endif //MESSAGE_H
