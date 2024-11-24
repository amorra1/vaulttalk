#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <ctime>
#include "user.h"

using namespace std;

class Message {
private:
    User sender;
    string username;
    string content;  //actual content of the message
    time_t timestamp;

public:
    Message(const User &from, const string &msgContent);
    Message(const std::string &user, const std::string &text);

    const User& getSender() const;
    const string& getContent() const;
    time_t getTimestamp() const;
    string getEncryptedContent(const User &user) const;
    string getDecryptedContent(const User &user) const;

    void displayMessage() const;
};

#endif //MESSAGE_H
