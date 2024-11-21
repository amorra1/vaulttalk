#include "chatroom.h"

Chatroom::Chatroom(string &name, mpz_class &key)
    : name(name), key(key) {
}

// add message to vector
int Chatroom::addMessage(const Message &message) {
    try {
        chatLog.push_back(message);
    } catch (...) {
        return 0;
    }
    return 1;
}

// add user to chat
int Chatroom::addMember(const User &user) {
    try {
        members.push_back(user);
    } catch (...) {
        return 0;
    }
    return 1;
}

// remove member

std::vector<User> Chatroom::getMembers() const {
    return members;
}


// name getter function
std::string Chatroom::getName() const {
    return this->name;
}


