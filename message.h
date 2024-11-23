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

    string getEncryptedContent(const User &user) const;
    string getDecryptedContent(const mpz_class &encryptedMessage, User &user) const;

    string getAESEncryptedContent(const User &user);

    void displayMessage() const;
};

#endif //MESSAGE_H
