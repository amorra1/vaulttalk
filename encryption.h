#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <string>
#include <gmpxx.h>

typedef struct keys {

    mpz_class publicKey[2];
    mpz_class privateKey[2];

}RSA_keys;

namespace encryption {

RSA_keys GenerateKeys();
mpz_class RSA_Encrypt(std::string inputMsg, RSA_keys keys);
std::string RSA_Decrypt(unsigned long long inputValue, RSA_keys keys);

}

#endif // ENCRYPTION_H
