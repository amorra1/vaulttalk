#include "message.h"
#include <iostream>
#include <cstring> 
#include <vector>
#include <xstring>

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

    if (method == "RSA"){
        mpz_class encrypted = encryption::RSA_Encrypt(this->content, user.getKeys());
        encryptedString = encrypted.get_str(10);
    }
    else if (method == "AES") {
        //AES Logic here
        std::vector<unsigned char> messageBuffer(this->content.begin(), this->content.end());
        messageBuffer.push_back('\0'); // Null-terminate for safety

        unsigned char* message = messageBuffer.data();
        int originalLen = strlen((const char*)message);
        int paddedMessageLen = (originalLen % 16 == 0) ? originalLen : (originalLen / 16 + 1) * 16;

        // Create padded and encrypted message buffers
        unsigned char* paddedMessage = new unsigned char[paddedMessageLen];
        unsigned char* encryptedMessage = new unsigned char[paddedMessageLen];

        // Pad the message
        for (int i = 0; i < paddedMessageLen; i++) {
            paddedMessage[i] = (i < originalLen) ? message[i] : 0x00;
        }

        // Read AES key from file
        string str;
        ifstream infile("aes_keyfile", ios::in | ios::binary);
        if (infile.is_open()) {
            getline(infile, str);
            infile.close();
        } else {
            cout << "Unable to open keyfile" << endl;
            delete[] paddedMessage;
            delete[] encryptedMessage;
            return encryptedString; // Return unencrypted content
        }

        unsigned char AES_key[16];
        istringstream hex_chars_stream(str);
        unsigned int c;
        int i = 0;
        while (hex_chars_stream >> hex >> c) {
            AES_key[i] = c;
            i++;
        }

        // Expand the 16-byte key to a 176-byte key
        unsigned char expandedKey[176];
        KeyExpansion(AES_key, expandedKey);

        // Encrypt the message in 16-byte blocks
        for (int i = 0; i < paddedMessageLen; i += 16) {
            AESEncrypt(paddedMessage + i, expandedKey, encryptedMessage + i);
        }

        // Convert the encrypted message to a string
        encryptedString = std::string(reinterpret_cast<char*>(encryptedMessage), paddedMessageLen);

        // Clean up allocated memory
        delete[] paddedMessage;
        delete[] encryptedMessage;
    }

    return encryptedString;
}

// string Message::getEncryptedContent(User user) {
//     return content;
// }

string Message::getDecryptedContent(const User &user) const {
    string method = user.getEncryptionMethod();
    string decryptedMessage = this->content;

    if (method == "RSA"){
        mpz_class encryptedContent(this->content);
        decryptedMessage = encryption::RSA_Decrypt(encryptedContent, user.getKeys());
    }
    else if (method == "AES"){
        // AES encryption call here
    }

    return decryptedMessage;
}

void Message::displayMessage() const {
    cout << "From: " << sender.getUsername()
    << ", Timestamp: " << ctime(&timestamp)
    << ", Encrypted Content: " << content << endl;
}
