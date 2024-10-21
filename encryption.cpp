#include <ctime>
#include <iostream>
#include <math.h>
#include <random>
#include <bitset>

#include "encryption.h"

using namespace encryption;

#define MAX_PRIME 10;
#define MAX_E 10;

// Function prototypes used in encryption.cpp.
// NOTE: THESE ARE ONLY MEANT TO BE USED AS HELPER FUNCTIONS AS OF THE CURRENT VERSION.

int GCD(int a, int b);
int GeneratePrime(int a = 0, int b = 0);
bool CheckPrime(int x);
bool CheckCoPrime(int a, int b);
int ModInverse(int a, int m);

// Finds the greatest common devisor of two numbers.

int GCD(int a, int b) {

    int gcd = 1;

    for (int i = 1; i <= a; i++) {

        if (a % i == 0 && b % i == 0) {

            gcd = i;

        }
    }

    return gcd;
}

// Generates two prime numbers. Returns an array of size 2*sizeof(int).
// Has optional arguments specifying range. By default range is 1 to 256.

int GeneratePrime(int a, int b) {

    int x;
    int y;

    if (a == 0 && b == 0) {

        x = 1;
        y = MAX_PRIME;

    } else if (a != 0 && b == 0) {

        x = a;
        y = MAX_PRIME;

    }
    
    else {

        x = a;
        y = b;
        
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(x, y);

    while (true) {

        int currentNum = dis(gen);

        bool isPrime = CheckPrime(currentNum);

        if (isPrime) {

            return currentNum;

        }
    }
}

// Checks if a number is prime.

bool CheckPrime(int x) {

    if (x == 1 || x == 2) {

        return false;

    }

    float sr = sqrt(x);

    for (int i = 2; i <= sr; i++) {

        if (x % i == 0) {
            
            return false;

        }
    }

    return true;

}

// Checks if two numbers are coprime.
// in the arguments, a is meant to be manually entered as the smaller of the two numbers. 

bool CheckCoPrime(int a, int b) {

    if (GCD(a, b) != 1) {

        return false;

    } else {

        return true;
    }
}

// Hlper function for ModInverse().

int ExtendedGCD(int a, int b, int& x, int& y) {

    if (b == 0) {

        x = 1;
        y = 0;

        return a;
    }

    // To store results of recursive call

    int x1, y1; 
    int gcd = ExtendedGCD(b, a % b, x1, y1);

    x = y1;
    y = x1 - (a / b) * y1;

    return gcd;
}

// Function to find modular multiplicative inverse

int ModInverse(int a, int m) {

    int x, y;
    int gcd = ExtendedGCD(a, m, x, y);
  
    int res = (x % m + m) % m;
    return res;
    
}

RSA_keys encryption::GenerateKeys(){

    std::cout << "Generating primes..." << std::endl;

    int primes[2];

    primes[0] = GeneratePrime();
    primes[1] = GeneratePrime();

    long long modulus = primes[0] * primes[1];
    long long phi = (primes[1] - 1) * (primes[0] - 1);
    long long e = 3;
    long long d;

    std::cout << "Checking if default e is valid..." << std::endl;

    if (!CheckCoPrime(e, phi)) {

        std::cout << "e is not valid. Regenerating to new coprime value..." << std::endl;

        while (true) {

            // GeneratePrime accepts the lower range as an arguement. In this case it is e.

            int maxe = MAX_E;

            int newPrime = GeneratePrime(e, maxe);

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

unsigned long long encryption::RSA_Encrypt(std::string inputMsg, RSA_keys keys) {

    std::string bitString = "";

    for (size_t i = 0; i < inputMsg.size(); i++) {

        std::bitset<8> tempBits = std::bitset<8>(inputMsg[i]);

        std::string tempString = tempBits.to_string();

        for (size_t i = 0; i < tempString.size(); i++) {

            bitString.push_back(tempString[i]);

        }

    }

    std::cout << std::endl << "Generating binary representation of string..." << std::endl;
    std::cout << bitString << std::endl;
    std::cout << std::endl << "Generating decimal representation..." << std::endl;

    int decVal = 0;
    int max_exp = bitString.size() - 1;
    
    for (int i = 0; i < bitString.size(); i++) {

        if (bitString[i] == '1') {

            decVal += pow(2, max_exp);

        }

        max_exp--;

    }

    std::cout << decVal << std::endl;
    std::cout << std::endl << "Generating final encrypted message..." << std::endl;

    unsigned long long tempPowVal = pow(decVal, keys.publicKey[1]);
    unsigned long long encrypted = tempPowVal % keys.publicKey[0];

    std::cout << encrypted << std::endl;

    return encrypted;

}

// The RSA decryption algorithm is currently unfinished. The numbers are overflowing even an unsigned long long.
// See RSA_Encrypt for explanation on fix. 

std::string encryption::RSA_Decrypt(unsigned long long inputValue, RSA_keys keys) {

    std::cout << std::endl << "Message recieved: " << std::endl;
    std::cout << inputValue << std::endl;

    std::cout << std::endl << "Decrypting message..." << std::endl;

    unsigned long long tempPowVal = pow(inputValue, keys.privateKey[1]);

    unsigned long long decrypted = tempPowVal % keys.privateKey[0];

    std::cout << std::endl << "The decimal representation of the message is: " << std::endl;
    std::cout << decrypted << std::endl;

    std::cout << "Converting decimal to binary..." << std::endl;

    std::string binVal = std::bitset<16>(decrypted).to_string();

    std::cout << std::endl << "The binary representation of the message is: " << std::endl;
    std::cout << binVal << std::endl;

    return "Temp";

}
