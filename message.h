#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <ctime>
#include "user.h"

using namespace std;

class Message {
private:
    User sender;
    string content;  //actual content of the message
    time_t timestamp;

public:
    Message(const User &from, const string &msgContent);

    string getEncryptedContent() const;
    string getDecryptedContent(const string &privateKey) const;

    void displayMessage() const;
};

//to be implemented in encryption folder, these methods may just call other methods to encrypt/decrypt that
//way we can call encrypt on the message content and decrypt on the message content
string encryptMessage(const string &message);
string decryptMessage(const string &encryptedMessage);

#endif //MESSAGE_H
