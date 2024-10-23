#include "user.h"
#include <iostream>
#include <unordered_map>
#include <functional> // for hash functions like SHA-256
#include <sstream>

using namespace std;

// Initing a static database
unordered_map<string, User> User::userDatabase;

User::User() : userId(""), username(""), publicKey(""), hashedPassword("") {}
User::User(string id, string name, string pubKey, string hashedPassword)
    : userId(id), username(name), publicKey(pubKey), hashedPassword(hashedPassword) {}



string User::getUserId() const { return userId; }
string User::getUsername() const { return username; }
string User::getPublicKey() const { return publicKey; }


// compares hashed password
bool User::authenticate(string passwordAttempt) const {
    return hashedPassword == hashPassword(passwordAttempt);
}

//need to add a hashing function such as sha-256, this is used for temp replacement right now
string User::hashPassword(const string &password) {
    // Example of simple hash function using std::hash (replace with a more secure one)
    hash<string> hasher;
    size_t hashedValue = hasher(password);
    stringstream ss;
    ss << hex << hashedValue; // Convert to hexadecimal string
    return ss.str();
}

// for registering
bool User::registerUser(const string &username, const string &password, const string &publicKey) {
    if (userDatabase.find(username) != userDatabase.end()) {
        cout << "Username is nto available!" << endl;
        return false; // User already exists
    }

    string hashedPassword = hashPassword(password);
    User newUser(username, username, publicKey, hashedPassword);
    userDatabase[username] = newUser;
    cout << "User " << username << " registered successfully!" << endl;
    return true;
}

// for logging in an existing user
User* User::loginUser(const string &username, const string &password) {
    auto it = userDatabase.find(username);
    if (it == userDatabase.end()) {
        cout << "User is not found!" << endl;
        return nullptr;
    }

    if (it->second.authenticate(password)) {
        cout << "Login successful!" << endl;
        return &it->second;
    } else {
        cout << "Incorrect password!" << endl;
        return nullptr;
    }
}
