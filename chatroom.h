#ifndef CHATROOM_H
#define CHATROOM_H

#include <string>
#include <vector>
#include "user.h"
#include "message.h"

class Chatroom {
private:
    std::string name;
    mpz_class key;
    std::vector<User> members;
    std::vector<Message> chatLog;

public:
    // constructor
    Chatroom(std::string &name, mpz_class &key);
    int addMessage(const Message &message);

    int addMember(const User &user);

    std::vector<User> getMembers() const;


    std::string getName() const;

};

#endif // CHATROOM_H
