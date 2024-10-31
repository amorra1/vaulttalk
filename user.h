#ifndef USER_H
#define USER_H

#include <string>
#include <iostream>
#include <QMainWindow>


using namespace std;

class User {
public:
    User(); // Default constructor
    User(std::string username, std::string hashedPassword);
    // User(std::string username, std::string hashedPassword, std::string pubKey, std::string privKey);

    ~User();

    std::string getUsername() const;
    std::string getPublicKey() const;
    std::string getPrivateKey() const;

    static std::string hashPassword(const std::string &password);

    // stuff for user database (sign up and login)
    static void registerUser(User user);
    void loginUser(User user, std::function<void(bool)> callback);

private:
    std::string username;
    std::string hashedPassword;
    // std::string publicKey;
    // std::string privateKey;

};

#endif //USER_H
