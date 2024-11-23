#include "message.h"
#include <iostream>
#include <cstring> 
#include <vector>

using namespace std;


//constructor
Message::Message(const User &from, const string &msgContent)
    : sender(from), timestamp(time(nullptr)) {
    content = msgContent;
}

string Message::getEncryptedContent(const User &user) const {
    mpz_class encrypted = encryption::RSA_Encrypt(this->content, user.getKeys());
    return encrypted.get_str(10);
}

// string Message::getEncryptedContent(User user) {
//     return content;
// }

string Message::getDecryptedContent(const mpz_class &encryptedMessage, User &user) const {
    string decrypted = encryption::RSA_Decrypt(encryptedMessage, user.getKeys());
    return decrypted;
}

string Message::getAESEncryptedContent(const User &user) {

    std::vector<unsigned char> messageBuffer(content.begin(), content.end());
    messageBuffer.push_back('\0');  // Null-terminate for safety

    // Get the message as unsigned char*
    unsigned char* message = messageBuffer.data();

    // Calculate the length of the original message
    int originalLen = strlen((const char*)message);

    // Calculate the length of the padded message
    int paddedMessageLen = (originalLen % 16 == 0) ? originalLen : (originalLen / 16 + 1) * 16;

    // Create an array to store the padded message
    unsigned char* paddedMessage = new unsigned char[paddedMessageLen];

    // Pad the message with zeroes
    for (int i = 0; i < paddedMessageLen; i++) {
        paddedMessage[i] = (i < originalLen) ? message[i] : 0x00;
    }

    // Create an array to store the encrypted message
    unsigned char * encryptedMessage = new unsigned char[paddedMessageLen];

    // Open the key file and read it into a string
    string str;
    ifstream infile("aes_keyfile", ios::in | ios::binary);
    if (infile.is_open()) {
        getline(infile, str);
        infile.close();
    } else {
        cout << "Unable to open keyfile" << endl;
        return 1;
    }

    // Convert the string to a 16-byte array
    istringstream hex_chars_stream(str);
    unsigned char key[16];
    int i = 0;
    unsigned int c;
    while (hex_chars_stream >> hex >> c) {
        key[i] = c;
        i++;
    }

    // Expand the 16-byte key to a 176-byte key
    unsigned char expandedKey[176];
    KeyExpansion(key, expandedKey);

    // Encrypt the message in 16-byte blocks
    for (int i = 0; i < paddedMessageLen; i += 16) {
        AESEncrypt(paddedMessage + i, expandedKey, encryptedMessage + i);
    }

     // Convert encryptedMessage back to a std::string
    std::string encryptedString(reinterpret_cast<char*>(encryptedMessage), paddedMessageLen);

    // Clean up dynamically allocated memory
    delete[] paddedMessage;
    delete[] encryptedMessage;

    return encryptedString;
}

void Message::displayMessage() const {
    cout << "From: " << sender.getUsername()
    << ", Timestamp: " << ctime(&timestamp)
    << ", Encrypted Content: " << content << endl;
}
