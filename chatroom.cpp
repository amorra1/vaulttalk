#include "chatroom.h"

Chatroom::Chatroom(const std::string &name, const User &sender, const User &receiver)
    : name(name) {
    members.push_back(sender);
    members.push_back(receiver);
}

Chatroom::Chatroom(const std::string& chatroomName) : name(chatroomName) {}

int Chatroom::addMessage(const Message &message) {
    try {
        chatLog.push_back(message);
    } catch (...) {
        return 0;
    }
    return 1;
}

int Chatroom::addMember(const User &user) {
    try {
        members.push_back(user);
    } catch (...) {
        return 0;
    }
    return 1;
}

int Chatroom::removeMember(const std::string &username) {
    for (auto it = members.begin(); it != members.end(); ++it) {
        if (it->getUsername() == username) {
            return 1;
        }
    }
    return 0;
}

std::vector<User> Chatroom::getMembers() const {
    return members;
}

std::vector<Message> Chatroom::getChatLog() const {
    return chatLog;
}

std::string Chatroom::getName() const {
    return name;
}
