#include "message.h"
#include "encryption.h"
#include <iostream>
#include <cstring>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

std::vector<unsigned char> AES_encrypted_binary;

//constructor
Message::Message(const User &from, const string &msgContent)
    : sender(from), timestamp(time(nullptr)) {
    content = msgContent;
}

Message::Message(const std::string &user, const std::string &text)
    : username(user) {
    content = text;
}

const User& Message::getSender() const {
    return sender;
}

const string& Message::getContent() const {
    return content;
}

time_t Message::getTimestamp() const {
    return timestamp;
}

string Message::getEncryptedContent(const User &user) const {
    string method = user.getEncryptionMethod(); //get the users encryption method
    string encryptedString = this->content;

    cout << "Message before encrypting: " << encryptedString << endl;

    if (method == "RSA") {
        mpz_class encrypted = encryption::RSA_Encrypt(this->content, user.getKeys());
        encryptedString = encrypted.get_str(10);
    } else if (method == "AES") {
        // Calculate the length of the original message
        int originalLen = encryptedString.length();

        // Calculate the length of the padded message
        int paddedMessageLen = (originalLen % 16 == 0) ? originalLen : (originalLen / 16 + 1) * 16;

        // Create an array to store the padded message
        unsigned char * paddedMessage = new unsigned char[paddedMessageLen];
        for (int i = 0; i < paddedMessageLen; i++) {
            paddedMessage[i] = (i < originalLen) ? static_cast<unsigned char>(encryptedString[i]) : 0x00;
        }

        // Create an array to store the encrypted message
        unsigned char * encryptedMessage = new unsigned char[paddedMessageLen];

        string AESKey = encryption::generateAESKey(); // Store the key in the instance
        if (AESKey.empty()) {
            cerr << "Error: Failed to generate AES Key!" << endl;
        }
        cout << "Debug: this->AESKey after generation: " << AESKey << endl;

        // Convert the string to a 16-byte array
        istringstream hex_chars_stream(AESKey);
        unsigned char key[16];
        int i = 0;
        unsigned int c;
        while (hex_chars_stream >> hex >> c) {
            key[i] = c;
            i++;
        }

        // Expand the 16-byte key to a 176-byte key
        unsigned char expandedKey[176];
        encryption::KeyExpansion(key, expandedKey);

        // Encrypt the message in 16-byte blocks
        for (int i = 0; i < paddedMessageLen; i += 16) {
            encryption::AESEncrypt(paddedMessage + i, expandedKey, encryptedMessage + i);

            // Append the encrypted block to the global variable
            AES_encrypted_binary.insert(AES_encrypted_binary.end(), encryptedMessage + i, encryptedMessage + i + 16);
        }

        std::ostringstream hexStream;
        for (int i = 0; i < paddedMessageLen; i++) {
            hexStream << hex << std::setfill('0') << std::setw(2) << (int) encryptedMessage[i];
        }

        // Convert the stream to a string
        encryptedString = hexStream.str();

        cout << "this is the encrypted string: " << encryptedString << endl;

        // Clean up allocated memory
        delete[] paddedMessage;
        delete[] encryptedMessage;
    }
    //Encrypting using the ROT13 function in encryption
    else if(method == "ROT13") {
        //Need to provide message string
        encryptedString = encryption::ROT13Encrypt(encryptedString);
    }
    else if(method == "ELEC376 Cipher") {
        //Need to provide message string
        encryptedString = encryption::ELEC376Encrypt(encryptedString);
    }
    return encryptedString;
}

string Message::getDecryptedContent(const User &user) const {
    string method = user.getEncryptionMethod();
    string decryptedMessage = this->content;

    if (method == "RSA") {
        mpz_class encryptedContent(this->content);
        decryptedMessage = encryption::RSA_Decrypt(encryptedContent, user.getKeys());
    } else if (method == "AES") {
        // Read AES key from file
        string key;
        ifstream infile("aes_keyfile.txt", ios::in | ios::binary);
        if (!infile) {
            qDebug() << "Error: Unable to open aes_keyfile. Ensure the file exists in the working directory.";
            return decryptedMessage;
        }

        getline(infile, key);
        infile.close();


        // Debugging: Print the content of the key file
        cout << "Key file content: " << key << endl;

        // unsigned char AES_key[16];
        // istringstream hex_chars_stream(str);
        // unsigned int c;
        // int i = 0;
        // while (hex_chars_stream >> hex >> c) {
        //     if (i >= 16) {
        //         qDebug() << "Error: Key file contains more than 16 bytes. Ensure the key is valid.";
        //         return decryptedMessage;
        //     }
        //     AES_key[i] = c;
        //     i++;
        // }

        // if (i < 16) {
        //     qDebug() << "Error: Key file contains fewer than 16 bytes. Ensure the key is valid.";
        //     return decryptedMessage;
        // }

        // Convert the hexadecimal key to raw bytes
        // unsigned char AES_key[16];
        // for (int i = 0; i < 16; ++i) {
        //     AES_key[i] = std::stoi(NewAESKey.substr(i * 2, 2), nullptr, 16);
        // }

        // // Expand the 16-byte key to a 176-byte key
        // unsigned char expandedKey[176];
        // encryption::KeyExpansion(AES_key, expandedKey);

        // Convert the string to a 16-byte array
        istringstream hex_chars_stream(key);
        unsigned char aes_key[16];
        int i = 0;
        unsigned int c;
        while (hex_chars_stream >> hex >> c) {
            aes_key[i] = c;
            i++;
        }

        cout << "Decryption key: " << aes_key << endl;

        // Expand the 16-byte key to a 176-byte key
        unsigned char expandedKey[176];
        encryption::KeyExpansion(aes_key, expandedKey);

        // Decrypt the message in 16-byte blocks
        std::vector<unsigned char> encryptedBuffer(this->content.begin(), this->content.end());
        size_t paddedMessageLen = encryptedBuffer.size();
        std::vector<unsigned char> decryptedBuffer(paddedMessageLen, 0);

        for (size_t i = 0; i < paddedMessageLen; i += 16) {
            encryption::AESDecrypt(encryptedBuffer.data() + i , expandedKey, decryptedBuffer.data() + i);
        }

        // Remove padding
        int decryptedLen = paddedMessageLen;
        encryption::removePadding(decryptedBuffer.data(), decryptedLen);

        // Convert decrypted binary back to string
        decryptedMessage.assign(reinterpret_cast<char*>(decryptedBuffer.data()), decryptedLen);
    }
    //Decrypting using the ROT13 function in encryption
    else if(method == "ROT13"){
        //Need to provide message string
        decryptedMessage = encryption::ROT13Decrypt(decryptedMessage);
    }
    return decryptedMessage;
}
//used for debugging
void Message::displayMessage() const {
    cout << "From: " << sender.getUsername()
    << ", Timestamp: " << ctime(&timestamp)
    << ", Encrypted Content: " << content << endl;
}
