#include <ctime>
#include <gmpxx.h>
#include <iostream>
#include <math.h>
#include <chrono>
#include <sstream>

#include "encryption.h"

using namespace encryption;

// Function prototypes used in encryption.cpp.
// NOTE: THESE ARE ONLY MEANT TO BE USED AS HELPER FUNCTIONS AS OF THE CURRENT VERSION.

mpz_class GeneratePrime(int bitSize);
bool CheckCoPrime(mpz_class a, mpz_class b);
mpz_class ModInverse(mpz_class a, mpz_class m);

// Generate a random prime number.

mpz_class GeneratePrime(int bitSize) {
    mpz_class random;
    mpz_class prime;

    gmp_randstate_t state;
    gmp_randinit_default(state);

    unsigned long seed = std::chrono::system_clock::now().time_since_epoch().count();

    gmp_randseed_ui(state, seed);

    mpz_urandomb(random.get_mpz_t(), state, bitSize);

    mpz_nextprime(prime.get_mpz_t(), random.get_mpz_t());

    gmp_randclear(state);

    return prime;
}

// Checks if two numbers are coprime.
// in the arguments, a is meant to be manually entered as the smaller of the two numbers.

bool CheckCoPrime(mpz_class a, mpz_class b) {
    mpz_class result = gcd(a, b);

    if (result != 1) {
        return false;

    } else {
        return true;
    }
}

// Helper function for ModInverse().

mpz_class ExtendedGCD(mpz_class a, mpz_class b, mpz_class& x, mpz_class& y) {
    if (b == 0) {
        x = 1;
        y = 0;

        return a;
    }

    // To store results of recursive call

    mpz_class x1, y1;
    mpz_class gcd = ExtendedGCD(b, a % b, x1, y1);

    x = y1;
    y = x1 - (a / b) * y1;

    return gcd;
}

// Function to find modular multiplicative inverse

mpz_class ModInverse(mpz_class a, mpz_class m) {
    mpz_class x, y;
    mpz_class gcd = ExtendedGCD(a, m, x, y);

    mpz_class res = (x % m + m) % m;
    return res;
}

// Generates a user's "keyring".
// A keyring contains the private, public and modulus needed for encryption and decryption.
// Should only be called once per user, or per regeneration of key pairs.

RSA_keys encryption::GenerateKeys() {
    // std::cout << "Generating primes..." << std::endl;

    mpz_class primes[2];

    // Calling the GeneratePrime() function to create primes used in key generation
    primes[0] = GeneratePrime(1024);
    primes[1] = GeneratePrime(1024);

    mpz_class modulus = primes[0] * primes[1];
    mpz_class phi = (primes[1] - 1) * (primes[0] - 1);
    mpz_class e = 65537;
    mpz_class d;

    // Verify if e is coprime to phi. If not generate new e.
    // std::cout << "Checking if default e is valid..." << std::endl;

    if (!CheckCoPrime(e, phi)) {
        // std::cout << "e is not valid. Regenerating to new coprime value..." << std::endl;

        while (true) {
            // GeneratePrime accepts the lower range as an arguement. In this case it is e.

            mpz_class newPrime;

            mpz_nextprime(newPrime.get_mpz_t(), e.get_mpz_t());

            if (CheckCoPrime(newPrime, phi)) {
                // std::cout << "New e is: " << newPrime << std::endl;

                e = newPrime;
                break;
            }
        }
    }

    // std::cout << "Generating d... " << std::endl;

    // Using the ModInverse() function, find the modular inverse of e and phi (d).
    d = ModInverse(e, phi);

    RSA_keys keys;

    keys.publicKey[0] = modulus;
    keys.publicKey[1] = e;

    keys.privateKey[0] = modulus;
    keys.privateKey[1] = d;

    // std::cout << std::endl
    //     << "Generated Primes: " << primes[0].get_str().substr(0, 10) << "... " << "and " << primes[1].get_str().substr(0, 10) << "... " << std::endl;
    // std::cout << "Modulus: " << modulus.get_str().substr(0, 10) << "... " << std::endl;
    // std::cout << "Phi: " << phi.get_str().substr(0, 10) << "... " << std::endl;
    // std::cout << "E: " << e << std::endl;
    // std::cout << "D: " << d.get_str().substr(0, 10) << "... " << std::endl;

    return keys;
}

// Used to encrypt the original message. Returns an mpz_class integer.
mpz_class encryption::RSA_Encrypt(std::string inputMsg, RSA_keys keys) {

    // std::cout << std::endl << "Encrypting Message..." << std::endl;

    std::stringstream ss;
    std::string hexString;

    // Convert the original message into its hexidecimal representation.
    for (int i = 0; i < inputMsg.size(); i++) {

        // Convert the character to its ASCII representation
        int decVal = int(inputMsg[i]);

        // Convert the ASCII representation to hex
        ss << std::hex << decVal;

        // Add the hex to the final string
        hexString = ss.str();
    }

    // Convert the final string to an mpz_class integer
    mpz_class decHexString(hexString, 16);

    mpz_class encrypted;

    std::cout << "Encryption is being performed using: " << std::endl;
    std::cout << "E: " << keys.publicKey[1] << std::endl;
    std::cout << "Modulus: " << keys.publicKey[0].get_str().substr(0, 10) << "... " << std::endl;

    // Performs the encryption of the decimal value of the string.
    mpz_powm(encrypted.get_mpz_t(), decHexString.get_mpz_t(), keys.publicKey[1].get_mpz_t(), keys.publicKey[0].get_mpz_t());

    std::cout << std::endl << "Encrypted message is: " << encrypted << std::endl;

    return encrypted;

}

// Used to decrypt the mpz_class integer generated during encryption.
std::string encryption::RSA_Decrypt(mpz_class inputValue, RSA_keys keys) {

    // std::cout << std::endl << "Decrypting string... " << std::endl;

    mpz_class decryptedDecVal;

    std::cout << "Decryption is being performed using: " << std::endl;
    std::cout << "D: " << keys.privateKey[1].get_str().substr(0, 10) << "... " << std::endl;
    std::cout << "Modulus: " << keys.privateKey[0].get_str().substr(0, 10) << "... " << std::endl;

    // Undoes the encryption using the user's private key and modulus.
    mpz_powm(decryptedDecVal.get_mpz_t(), inputValue.get_mpz_t(), keys.privateKey[1].get_mpz_t(), keys.privateKey[0].get_mpz_t());

    // Converts the mpz_class integer into a hexidecimal string
    std::string hexString = mpz_get_str(nullptr, 16, decryptedDecVal.get_mpz_t());

    std::string decrypted;

    // Converts the hexidecimal string to its ASCII character equivalent.
    for (int i = 0; i < hexString.length(); i += 2) {

        std::string byte = hexString.substr(i, 2);

        char chr = (char)(int)strtol(byte.c_str(), nullptr, 16);

        decrypted.push_back(chr);
    }

    return decrypted;

}

