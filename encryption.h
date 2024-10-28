#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <string>


typedef struct keys {

    long long publicKey[2]; // [n,e] - RSA public key components
    long long privateKey[2]; // [n,e]

}RSA_keys;

namespace encryption {

// Generation of RSA public and private key pair
RSA_keys GenerateKeys();

unsigned long long RSA_Encrypt(std::string inputMsg, RSA_keys keys);
std::string RSA_Decrypt(unsigned long long inputValue, RSA_keys keys);

// // Encrypt a message using public key (message as string, returns a single encrypted value)
// unsigned long long RSA_Encrypt(const std::string&inputMsg, const long long publicKey[2]);

// // Decrypt the encrypted value using the private key (returns original message as string)
// std::string RSA_Decrypt(unsigned long long inputValue, const long long privateKey[2]);

} // end namespace encryption

#endif // ENCRYPTION_H
