#ifndef USER_H
#define USER_H

#include <string>
#include <iostream>
#include <QMainWindow>
#include "encryption.h"


using namespace std;

class User {
public:
    User(); // Default constructor
    User(std::string username, std::string hashedPassword);
    User(std::string username, std::string hashedPassword, std::string method, std::string duration);
    User(std::string username, std::string hashedPassword, RSA_keys keys);
    User(string username, string encryptionMethod, string regenDuration, RSA_keys keys);

    ~User();

    std::string getUsername() const;
    std::string getPassword() const;
    std::pair<mpz_class, mpz_class> getPublicKey() const;
    std::pair<mpz_class, mpz_class> getPrivateKey() const;
    std::string getEncryptionMethod() const;
    std::string getRegenDuration() const;
    RSA_keys getKeys() const;

    void setUsername(std::string name);
    void setPassword(std::string password);
    void setEncryptionMethod(std::string method);
    void setRegenDuration(std::string duration);
    void setPublicKey(const mpz_class& n, const mpz_class& e);
    void setPrivateKey(const mpz_class& n, const mpz_class& d);;

    static std::string hashPassword(const std::string &password);

    // stuff for user database (sign up and login)
    static void registerUser(User user);
    void loginUser(User &user, std::function<void(bool)> callback);

private:
    std::string username;
    std::string hashedPassword;
    std::string encryptionMethod;
    std::string regenDuration;
    RSA_keys RSAKeys;

};

#endif //USER_H
