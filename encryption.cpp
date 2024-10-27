#include <ctime>
#include <iostream>
#include <math.h>
#include <random>
#include <bitset>
#include <gmpxx.h>

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
    gmp_randseed_ui(state, time(NULL));

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

// Hlper function for ModInverse().

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

RSA_keys encryption::GenerateKeys(){

    std::cout << "Generating primes..." << std::endl;

    mpz_class primes[2];

    primes[0] = GeneratePrime(512);
    primes[1] = GeneratePrime(512);

    mpz_class modulus = primes[0] * primes[1];
    mpz_class phi = (primes[1] - 1) * (primes[0] - 1);
    mpz_class e = 3;
    mpz_class d;

    std::cout << "Checking if default e is valid..." << std::endl;

    if (!CheckCoPrime(e, phi)) {

        std::cout << "e is not valid. Regenerating to new coprime value..." << std::endl;

        while (true) {

            // GeneratePrime accepts the lower range as an arguement. In this case it is e.

            mpz_class newPrime;

            mpz_nextprime(newPrime.get_mpz_t(), e.get_mpz_t());

            if (CheckCoPrime(newPrime, phi)) {

                std::cout << "New e is: " << newPrime << std::endl;

                e = newPrime;
                break;
            }
        }
    }

    std::cout << "Generating d... " << std::endl;

    d = ModInverse(e, phi);

    RSA_keys keys;

    keys.publicKey[0] = modulus;
    keys.publicKey[1] = e;

    keys.privateKey[0] = modulus;
    keys.privateKey[1] = d;

    std::cout << std::endl << "Generated Primes: " << primes[0] << " and " << primes[1] << std::endl;
    std::cout << "Modulus: " << modulus << std::endl;
    std::cout << "Phi: " << phi << std::endl;
    std::cout << "E: " << e << std::endl;
    std::cout << "D: " << d << std::endl;

    return keys;

}

// Currently the best version of RSA encryption we can create. Libraries are needed to handle the size of the numbers used.

//mpz_class encryption::RSA_Encrypt(std::string inputMsg, RSA_keys keys) {
//
//    std::string bitString = "";
//
//    for (size_t i = 0; i < inputMsg.size(); i++) {
//
//        std::bitset<8> tempBits = std::bitset<8>(inputMsg[i]);
//
//        std::string tempString = tempBits.to_string();
//
//        for (size_t i = 0; i < tempString.size(); i++) {
//
//            bitString.push_back(tempString[i]);
//
//        }
//
//    }
//
//    std::cout << std::endl << "Generating binary representation of string..." << std::endl;
//    std::cout << bitString << std::endl;
//    std::cout << std::endl << "Generating decimal representation..." << std::endl;
//
//    int decVal = 0;
//    int max_exp = bitString.size() - 1;
//
//    for (int i = 0; i < bitString.size(); i++) {
//
//        if (bitString[i] == '1') {
//
//            decVal += pow(2, max_exp);
//
//        }
//
//        max_exp--;
//
//    }
//
//    std::cout << decVal << std::endl;
//    std::cout << std::endl << "Generating final encrypted message..." << std::endl;
//
//    unsigned long long tempPowVal = pow(decVal, keys.publicKey[1]);
//    unsigned long long encrypted = tempPowVal % keys.publicKey[0];
//
//    std::cout << encrypted << std::endl;
//
//    return encrypted;
//
//}

// The RSA decryption algorithm is currently unfinished. The numbers are overflowing even an unsigned long long.
// See RSA_Encrypt for explanation on fix.

//std::string encryption::RSA_Decrypt(unsigned long long inputValue, RSA_keys keys) {
//
//    std::cout << std::endl << "Message recieved: " << std::endl;
//    std::cout << inputValue << std::endl;
//
//    std::cout << std::endl << "Decrypting message..." << std::endl;
//
//    unsigned long long tempPowVal = pow(inputValue, keys.privateKey[1]);
//
//    unsigned long long decrypted = tempPowVal % keys.privateKey[0];
//
//    std::cout << std::endl << "The decimal representation of the message is: " << std::endl;
//    std::cout << decrypted << std::endl;
//
//    std::cout << "Converting decimal to binary..." << std::endl;
//
//    std::string binVal = std::bitset<16>(decrypted).to_string();
//
//    std::cout << std::endl << "The binary representation of the message is: " << std::endl;
//    std::cout << binVal << std::endl;
//
//    return "Temp";
//
//}

