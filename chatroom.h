#ifndef CHATROOM_H
#define CHATROOM_H

#include <string>
#include <vector>
#include "user.h"
#include "message.h"
#include <gmpxx.h>

class Chatroom {
private:
    std::string name;
    mpz_class key;
    std::vector<User> members;
    std::vector<Message> chatLog; //stores message objects that were sent

public:
    Chatroom(const std::string& chatroomName);
    Chatroom(const std::string &name, const User &sender, const User &receiver);

    int addMessage(const Message &message);
    int addMember(const User &user);
    int removeMember(const std::string &username);

    std::vector<User> getMembers() const;
    std::vector<Message> getChatLog() const;
    std::string getName() const;
};

#endif // CHATROOM_H
