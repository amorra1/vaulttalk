#include <ctime>
#include <gmpxx.h>
#include <iostream>
#include <math.h>
#include <chrono>
#include <sstream>
#include "aes_structures.h"
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

    // std::cout << "Encryption is being performed using: " << std::endl;
    // std::cout << "E: " << keys.publicKey[1] << std::endl;
    // std::cout << "Modulus: " << keys.publicKey[0].get_str().substr(0, 10) << "... " << std::endl;

    // Performs the encryption of the decimal value of the string.
    mpz_powm(encrypted.get_mpz_t(), decHexString.get_mpz_t(), keys.publicKey[1].get_mpz_t(), keys.publicKey[0].get_mpz_t());

    // std::cout << std::endl << "Encrypted message is: " << encrypted << std::endl;

    return encrypted;

}

// Used to decrypt the mpz_class integer generated during encryption.
std::string encryption::RSA_Decrypt(mpz_class inputValue, RSA_keys keys) {

    // std::cout << std::endl << "Decrypting string... " << std::endl;

    mpz_class decryptedDecVal;

    // std::cout << "Decryption is being performed using: " << std::endl;
    // std::cout << "D: " << keys.privateKey[1].get_str().substr(0, 10) << "... " << std::endl;
    // std::cout << "Modulus: " << keys.privateKey[0].get_str().substr(0, 10) << "... " << std::endl;

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



/* Serves as the initial round during encryption
 * AddRoundKey is simply an XOR of a 128-bit block with the 128-bit key.
 */
void AddRoundKey(unsigned char * state, unsigned char * roundKey) {
	for (int i = 0; i < 16; i++) {
		state[i] ^= roundKey[i];
	}
}

/* Perform substitution to each of the 16 bytes
 * Uses S-box as lookup table 
 */
void SubBytes(unsigned char * state) {
	for (int i = 0; i < 16; i++) {
		state[i] = s[state[i]];
	}
}

void MixColumns(unsigned char * state) {
	unsigned char tmp[16];

	tmp[0] = (unsigned char) mul2[state[0]] ^ mul3[state[1]] ^ state[2] ^ state[3];
	tmp[1] = (unsigned char) state[0] ^ mul2[state[1]] ^ mul3[state[2]] ^ state[3];
	tmp[2] = (unsigned char) state[0] ^ state[1] ^ mul2[state[2]] ^ mul3[state[3]];
	tmp[3] = (unsigned char) mul3[state[0]] ^ state[1] ^ state[2] ^ mul2[state[3]];

	tmp[4] = (unsigned char)mul2[state[4]] ^ mul3[state[5]] ^ state[6] ^ state[7];
	tmp[5] = (unsigned char)state[4] ^ mul2[state[5]] ^ mul3[state[6]] ^ state[7];
	tmp[6] = (unsigned char)state[4] ^ state[5] ^ mul2[state[6]] ^ mul3[state[7]];
	tmp[7] = (unsigned char)mul3[state[4]] ^ state[5] ^ state[6] ^ mul2[state[7]];

	tmp[8] = (unsigned char)mul2[state[8]] ^ mul3[state[9]] ^ state[10] ^ state[11];
	tmp[9] = (unsigned char)state[8] ^ mul2[state[9]] ^ mul3[state[10]] ^ state[11];
	tmp[10] = (unsigned char)state[8] ^ state[9] ^ mul2[state[10]] ^ mul3[state[11]];
	tmp[11] = (unsigned char)mul3[state[8]] ^ state[9] ^ state[10] ^ mul2[state[11]];

	tmp[12] = (unsigned char)mul2[state[12]] ^ mul3[state[13]] ^ state[14] ^ state[15];
	tmp[13] = (unsigned char)state[12] ^ mul2[state[13]] ^ mul3[state[14]] ^ state[15];
	tmp[14] = (unsigned char)state[12] ^ state[13] ^ mul2[state[14]] ^ mul3[state[15]];
	tmp[15] = (unsigned char)mul3[state[12]] ^ state[13] ^ state[14] ^ mul2[state[15]];

	for (int i = 0; i < 16; i++) {
		state[i] = tmp[i];
	}
}

void ShiftRows(unsigned char * state) {
	unsigned char tmp[16];

	/* Column 1 */
	tmp[0] = state[0];
	tmp[1] = state[5];
	tmp[2] = state[10];
	tmp[3] = state[15];
	
	/* Column 2 */
	tmp[4] = state[4];
	tmp[5] = state[9];
	tmp[6] = state[14];
	tmp[7] = state[3];

	/* Column 3 */
	tmp[8] = state[8];
	tmp[9] = state[13];
	tmp[10] = state[2];
	tmp[11] = state[7];
	
	/* Column 4 */
	tmp[12] = state[12];
	tmp[13] = state[1];
	tmp[14] = state[6];
	tmp[15] = state[11];

	for (int i = 0; i < 16; i++) {
		state[i] = tmp[i];
	}
}


/* Each round operates on 128 bits at a time
 * The number of rounds is defined in AESEncrypt()
 */
void Round(unsigned char * state, unsigned char * key) {
	SubBytes(state);
	ShiftRows(state);
	MixColumns(state);
	AddRoundKey(state, key);
}

 // Same as Round() except it doesn't mix columns
void FinalRound(unsigned char * state, unsigned char * key) {
	SubBytes(state);
	ShiftRows(state);
	AddRoundKey(state, key);
}

void AESEncrypt(unsigned char * message, unsigned char * expandedKey, unsigned char * encryptedMessage) {
	unsigned char state[16]; // Stores the first 16 bytes of original message

	for (int i = 0; i < 16; i++) {
		state[i] = message[i];
	}

	int numberOfRounds = 9;

	AddRoundKey(state, expandedKey); // Initial round

	for (int i = 0; i < numberOfRounds; i++) {
		Round(state, expandedKey + (16 * (i+1)));
	}

	FinalRound(state, expandedKey + 160);

	// Copy encrypted state to buffer
	for (int i = 0; i < 16; i++) {
		encryptedMessage[i] = state[i];
	}
}