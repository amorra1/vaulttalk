#ifndef USER_H
#define USER_H

#include <string>

using namespace std;

class User {
private:
    string userId; //can be used for port identification
    string username;
    string publicKey;
    // maybe add password or other user info as we go

public:
    User(string id, string name, string pubKey);

    string getUserId() const;
    string getUsername() const;
    string getPublicKey() const;

    void displayUser() const;
};

#endif //USER_H
