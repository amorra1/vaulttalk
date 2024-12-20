#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <string>
#include <gmpxx.h>

// The data structure containing the user's "keyring"
typedef struct keys {
    mpz_class publicKey[2];  // [n,e] - RSA public key components
    mpz_class privateKey[2]; // [n,d]

} RSA_keys;

namespace encryption {

// Generation of RSA public and private key pair
RSA_keys GenerateKeys();

// Encrypt a message using the user's "keyring".
// Returns an mpz_class integer.
// NOTE: PASS IN THE ENTIRE KEYRING NOT JUST THE PUBLIC KEY
mpz_class RSA_Encrypt(std::string inputMsg, RSA_keys keys);

// Decrypt the encrypted value using the private key (returns original message as string)
std::string RSA_Decrypt(mpz_class inputValue, RSA_keys keys);

// AES Encryption function
// parameters: char * message, char * expandedKey, char * encryptedMessage (basically storage for the encrypted message)
//Requires conversion of message type as detailed in message.cpp
void AESEncrypt(unsigned char * message, unsigned char * expandedKey, unsigned char * encryptedMessage);

// AES Decryption function
void AESDecrypt(unsigned char* encryptedMessage, unsigned char* expandedKey, unsigned char* decryptedMessage);

// This function removes null bytes from message byte array
// It is used in the AES decryption function
void removePadding(unsigned char * message, int &length);

//Expands the AES key
void KeyExpansion(unsigned char inputKey[16], unsigned char expandedKeys[176]);

std::string generateAESKey();

//Function definitions for ROT13
std::string ROT13Encrypt(std::string message);
std::string ROT13Decrypt(std::string message);
//Function definitions for Ceaser Cipher
std::string CaesarEncrypt(std::string message, int shift);
std::string CaesarDecrypt(std::string message);
//Function definitions for ELEC377*/
std::string ELEC376Encrypt(std::string message);
std::string ELEC376Decrypt(std::string message);
} // end namespace encryption

#endif // ENCRYPTION_H
