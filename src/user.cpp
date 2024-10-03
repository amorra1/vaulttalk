#include "user.h"
#include <iostream>

using namespace std;

//constructor
User::User(string id, string name, string pubKey)
    : userId(id), username(name), publicKey(pubKey) {}

string User::getUserId() const { //const ensures read only
    return userId;
}

string User::getUsername() const {
    return username;
}

string User::getPublicKey() const {
    return publicKey;
}

//to test general output for now
void User::displayUser() const {
    cout << "UserID: " << userId << ", Username: " << username << endl;
}
