#ifndef CHATROOM_H
#define CHATROOM_H
#include "message.h"
#include <string>
#include <vector>

class Chatroom
{
public:
    Chatroom(std::string &name, int &key);
    int addMessage(const Message &message);
    std::string getName() const;
    int addMember(const User &user);
private:
    std::string name;
    int key; // use ben's large numbers library in the future
    std::vector<Message> chatLog;
    std::vector<User> members;
};

#endif // CHATROOM_H
