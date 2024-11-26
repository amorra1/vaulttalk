#include "message.h"
#include "encryption.h"
#include <iostream>
#include <cstring> 
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;


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
    string method = user.getEncryptionMethod();
    string encryptedString = this->content;

    if (method == "RSA") {
        mpz_class encrypted = encryption::RSA_Encrypt(this->content, user.getKeys());
        encryptedString = encrypted.get_str(10);
    } else if (method == "AES") {
        // AES encryption logic
        std::vector<unsigned char> messageBuffer(this->content.begin(), this->content.end());
        messageBuffer.push_back('\0'); // Null-terminate for safety

        unsigned char* message = messageBuffer.data();
        int originalLen = messageBuffer.size() - 1; // Size minus null-terminator
        size_t paddedMessageLen = (originalLen % 16 == 0) ? originalLen : (originalLen / 16 + 1) * 16;

        // Create padded and encrypted message buffers
        unsigned char* paddedMessage = new unsigned char[paddedMessageLen];
        unsigned char* encryptedMessage = new unsigned char[paddedMessageLen];

        // Pad the message
        for (size_t i = 0; i < paddedMessageLen; i++) {
            paddedMessage[i] = (i < originalLen) ? message[i] : 0x00;
        }

        // Read AES key from file
        string str;
        ifstream infile("./aes_keyfile", ios::in | ios::binary);
        if (!infile) {
            cerr << "Error: Unable to open aes_keyfile. Ensure the file exists in the working directory." << endl;
            delete[] paddedMessage;
            delete[] encryptedMessage;
            return encryptedString; // Return unencrypted content
        }

        getline(infile, str);
        infile.close();

        // Debugging: Print the content of the key file
        cout << "Key file content: " << str << endl;

        unsigned char AES_key[16];
        istringstream hex_chars_stream(str);
        unsigned int c;
        int i = 0;
        while (hex_chars_stream >> hex >> c) {
            if (i >= 16) {
                cerr << "Error: Key file contains more than 16 bytes. Ensure the key is valid." << endl;
                delete[] paddedMessage;
                delete[] encryptedMessage;
                return encryptedString;
            }
            AES_key[i] = c;
            i++;
        }

        if (i < 16) {
            cerr << "Error: Key file contains fewer than 16 bytes. Ensure the key is valid." << endl;
            delete[] paddedMessage;
            delete[] encryptedMessage;
            return encryptedString;
        }

        // Expand the 16-byte key to a 176-byte key
        unsigned char expandedKey[176];
        encryption::KeyExpansion(AES_key, expandedKey);

        // Encrypt the message in 16-byte blocks
        for (size_t i = 0; i < paddedMessageLen; i += 16) {
            encryption::AESEncrypt(paddedMessage + i, expandedKey, encryptedMessage + i);
        }

        // Convert the encrypted message to a string
        encryptedString = std::string(reinterpret_cast<char*>(encryptedMessage), paddedMessageLen);

        // Clean up allocated memory
        delete[] paddedMessage;
        delete[] encryptedMessage;
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
        string str;
        ifstream infile("./aes_keyfile", ios::in | ios::binary);
        if (!infile) {
            cerr << "Error: Unable to open aes_keyfile. Ensure the file exists in the working directory." << endl;
            return decryptedMessage;
        }

        getline(infile, str);
        infile.close();

        // Debugging: Print the content of the key file
        cout << "Key file content: " << str << endl;

        unsigned char AES_key[16];
        istringstream hex_chars_stream(str);
        unsigned int c;
        int i = 0;
        while (hex_chars_stream >> hex >> c) {
            if (i >= 16) {
                cerr << "Error: Key file contains more than 16 bytes. Ensure the key is valid." << endl;
                return decryptedMessage;
            }
            AES_key[i] = c;
            i++;
        }

        if (i < 16) {
            cerr << "Error: Key file contains fewer than 16 bytes. Ensure the key is valid." << endl;
            return decryptedMessage;
        }

        // Expand the 16-byte key to a 176-byte key
        unsigned char expandedKey[176];
        encryption::KeyExpansion(AES_key, expandedKey);

        // Decrypt the message in 16-byte blocks
        std::vector<unsigned char> encryptedBuffer(this->content.begin(), this->content.end());
        size_t paddedMessageLen = encryptedBuffer.size();
        std::vector<unsigned char> decryptedBuffer(paddedMessageLen, 0);

        for (size_t i = 0; i < paddedMessageLen; i += 16) {
            encryption::AESDecrypt(encryptedBuffer.data() + i, expandedKey, decryptedBuffer.data() + i);
        }

        // Remove padding
        int decryptedLen = paddedMessageLen;
        encryption::removePadding(decryptedBuffer.data(), decryptedLen);

        // Convert decrypted binary back to string
        decryptedMessage.assign(reinterpret_cast<char*>(decryptedBuffer.data()), decryptedLen);
    }

    return decryptedMessage;
}
void Message::displayMessage() const {
    cout << "From: " << sender.getUsername()
    << ", Timestamp: " << ctime(&timestamp)
    << ", Encrypted Content: " << content << endl;
}
