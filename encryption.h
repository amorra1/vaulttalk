#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <string>

typedef struct keys {

    long long publicKey[2];
    long long privateKey[2];

}RSA_keys;

namespace encryption {

RSA_keys GenerateKeys();
unsigned long long RSA_Encrypt(std::string inputMsg, RSA_keys keys);
std::string RSA_Decrypt(unsigned long long inputValue, RSA_keys keys);

}

#endif // ENCRYPTION_H
