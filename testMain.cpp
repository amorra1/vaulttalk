// #include <iostream>

// int main(){
//     std::cout << "This is the main file";
//     return 0;
// }  

#include <iostream>
#include "src/user.h"
#include "src/message.h"

using namespace std;

int main() {
    User alex("1", "Alice", "AlicePublicKey");
    User sydney("2", "Bob", "BobPublicKey");

    alex.displayUser();
    sydney.displayUser();

    Message msg(alex, sydney, "Hello, Bob!");
    msg.displayMessage();

    string decryptedContent = msg.getDecryptedContent("BobPrivateKey");
    cout << "Decrypted Content: " << decryptedContent << endl;

    return 0;
}