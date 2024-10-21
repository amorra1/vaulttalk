#ifndef USER_H
#define USER_H

#include <string>
#include <iostream>
#include <unordered_map>


using namespace std;

class User {
public:
    User(std::string id, std::string name, std::string pubKey, std::string hashedPassword);

    std::string getUserId() const;
    std::string getUsername() const;
    std::string getPublicKey() const;

    bool authenticate(std::string passwordAttempt) const;

    static std::string hashPassword(const std::string &password);

    // stuff for user database (sign up and login)
    static bool registerUser(const std::string &username, const std::string &password, const std::string &publicKey);
    static User* loginUser(const std::string &username, const std::string &password);

private:
    std::string userId;
    std::string username;
    std::string publicKey;
    std::string hashedPassword;

    static std::unordered_map<std::string, User> userDatabase; // Store registered users
};

#endif //USER_H
