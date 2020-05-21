/*
 * Hung Huynh, Sam Borhan, Corey Zhou
 * CPSC 5042, Seattle University
 * Client side C/C++ program to demonstrate Socket programming
 */

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <map>
#include <string>
#include <thread>

#include "assert.h"
using namespace std;
#define PORT 12110
int sock = 0, valread;
char buffer[1024] = {0};
////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//Mike's code
class KeyValue {
   private:
    char m_szKey[128];
    char m_szValue[2048];

   public:
    KeyValue(){};
    void setKeyValue(char* pszBuff) {
        char* pch1;

        // find out where the "=" sign is, and take everything to the left of the equal for the key
        // go one beyond the = sign, and take everything else
        pch1 = strchr(pszBuff, '=');
        assert(pch1);
        int keyLen = (int)(pch1 - pszBuff);
        strncpy(m_szKey, pszBuff, keyLen);
        m_szKey[keyLen] = 0;
        strcpy(m_szValue, pszBuff + keyLen + 1);
    }

    char* getKey() {
        return m_szKey;
    }

    char* getValue() {
        return m_szValue;
    }
};

// This class will take a string that is passed to it in this format:

// input to constructor:
// <variable1>=<value1>;<variable2>=<value2>;
//You will then call the method  getNextKeyValue until getNextKeyValue returns NULL.
// getNextKeyValue will return a KeyValue object. Inside of that KeyValue object will contain the variable and the value
// You will then call getKey or getValue to get the contents of those fields.
// The example in main() will show how to call this function.
// By extracting the contents you then can determine the rpc you need to switch to, along with variables you will need
// You can also use this class in your client program, since you will need to determine the contents that you receive from server

class RawKeyValueString {
   private:
    char m_szRawString[32768];
    int m_currentPosition;
    KeyValue* m_pKeyValue;
    char* m_pch;

   public:
    RawKeyValueString(char* szUnformattedString) {
        assert(strlen(szUnformattedString));
        strcpy(m_szRawString, szUnformattedString);

        m_pch = m_szRawString;
    }
    ~RawKeyValueString() {
        if (m_pKeyValue)
            delete (m_pKeyValue);
    }

    void getNextKeyValue(KeyValue& keyVal) {
        // It will attempt to parse out part of the string all the way up to the ";", it will then create a new KeyValue object  with that partial string
        // If it can;t it will return null;
        char* pch1;
        char szTemp[32768];

        pch1 = strchr(m_pch, ';');
        assert(pch1 != NULL);
        int subStringSize = (int)(pch1 - m_pch);
        strncpy(szTemp, m_pch, subStringSize);
        szTemp[subStringSize] = 0;
        m_pch = pch1 + 1;
        if (m_pKeyValue)
            delete (m_pKeyValue);
        keyVal.setKeyValue(szTemp);
    }
};
////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

/*
Set up socket, connect to server
@return true if sucess
@return false if fail
*/
bool setUp() {
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return false;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return false;
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
Send  username on password to server
@return 1 if pass
@return 0 if user chooses to disconnect
*/
int Connect() {
    string option;
    cout << "Do You want to connect to the server?\n";

    while (true) {
        memset(buffer, 0, 1024);
        cout << "\nMenu:\n1.Connect\n2.Disconnect" << endl;
        cout << "Enter number for option: ";
        getline(cin, option);
        if (option.compare("1") == 0) {
            string username;
            string password;
            cout << "\nEnter your username: ";
            getline(cin, username);
            cout << "Enter your password: ";
            getline(cin, password);

            string rpcMessage = "rpc=connect;user=" + username + ";password=" + password + ";";

            const char* messageTOserver = rpcMessage.c_str();
            send(sock, messageTOserver, strlen(messageTOserver), 0);
            cout << "\nServer is busy. Please wait ..." << endl;
            valread = read(sock, buffer, 1024);

            if (strcmp(buffer, "Not Authorized") == 0) {
                cout << buffer;
                cout << "\n";
            } else {
                cout << buffer;
                cout << "\n";
                return 1;
            }
        } else if (option.compare("2") == 0) {
            break;
        } else {
            cout << "Invalid option!\n";
        }
    }
    return 0;
}
/*
Send disconnect signal to server
@return 0
*/
int Disconnect() {
    send(sock, "rpc=5;", strlen("rpc=5;"), 0);
    valread = read(sock, buffer, 1024);
    cout << buffer << endl;
    return 0;
}
/*
check if string is digit
@return bool
*/
bool isNumber(string s) {
    for (int i = 0; i < (int)s.length(); i++)
        if (isdigit(s[i]) == false)
            return false;

    return true;
}
/*
add item to the cart
@return signal string
*/
string addItem() {
    string sendMess = "rpc=3;";
    string input;
    while (true) {
        cout << "Enter ID item you want to add (0 to exit)" << endl;
        getline(cin, input);
        if (input.compare("0") == 0) {
            return input;
        }
        if (isNumber(input)) {
            sendMess += "item=" + input + ";";
            cout << "Do you want to add any items (press anykey or n to exit)?" << endl;
            getline(cin, input);
            if (input.compare("n") == 0) {
                break;
            }
        } else {
            cout << "digit number only" << endl;
        }
    }
    return sendMess;
}
/*
set up random int second to sleep
*/
void randomSleep() {
    srand(time(NULL));
    int timeSleep = rand() % 10 + 1;  //random from 1 to 10
    cout << "auto disconnect after " << timeSleep << "s" << endl;
    for (int i = timeSleep; i > 0; --i) {
        cout << i << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }
}
int main(int argc, char const* argv[]) {
    //set up client
    bool connected = setUp();
    ////////////////////////////
    while (connected) {
        //Authorize username and password
        int isAuthor = Connect();

        while (isAuthor) {
            string option;
            memset(buffer, 0, sizeof(buffer));
            cout << "\nMenu:\n1.View item List\n2.View your cart\n3.Add item to cart\n4.remove from list\n5.discconect" << endl;
            //cout << "Enter your option: ";
            //getline(cin, option);<= for milestone 2
            randomSleep();
            option = "5";  //<=force to disconnected

            if (option == "1") {
                send(sock, "rpc=1;", strlen("rpc=1;"), 0);
                valread = read(sock, buffer, 1024);
                cout << buffer << endl;

            } else if (option == "2") {
                send(sock, "rpc=2;", strlen("rpc=2;"), 0);
                valread = read(sock, buffer, 1024);
                cout << buffer << endl;

            } else if (option == "3") {
                string sendMess = addItem();
                if (sendMess.compare("0") != 0) {
                    const char* messageTOserver = sendMess.c_str();
                    send(sock, messageTOserver, strlen(messageTOserver), 0);
                    valread = read(sock, buffer, 1024);
                    cout << buffer << endl;
                }

            } else if (option == "4") {
                send(sock, "rpc=4;", strlen("rpc=4;"), 0);
                valread = read(sock, buffer, 1024);
                cout << buffer << endl;

            } else if (option == "5") {
                isAuthor = Disconnect();

            } else
                cout << "Invalid option!\n";
        }
        connected = false;
    }
    return 0;
}
