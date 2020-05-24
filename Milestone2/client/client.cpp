

// Client side C/C++ program to demonstrate Socket programming
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
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
int valread;
char buffer[1024] = {0};
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

//Connnect Rpc
int connectRPC(int& sock) {
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
DisconnectRPC
*/
int disconnectRPC(int& sock) {
    send(sock, "rpc=5;", strlen("rpc=5;"), 0);
    valread = read(sock, buffer, 1024);
    cout << buffer << endl;
    return 0;
}

int incrementRPC(int& sock, char* buff) {
    size_t valRead = 0;
    char hello[24];
    strcpy(hello, "Hello from client");

    char buffer[1024] = {0};
    send(sock, buff, strlen(buff), 0);
    printf("Hello message sent\n");
    if (strcmp(buff, "QUIT") == 0) {
        return 0;
    } else {
        valRead = read(sock, buffer, 1024);
        printf("ValRead=%lu buffer=%s\n", valRead, buffer);
    }
    return 0;
}

int connectToServer(char* szHostName, char* szPort, int& sock) {
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    uint16_t port = (uint16_t)atoi(szPort);

    serv_addr.sin_port = htons(port);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return 0;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, szHostName, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return 0;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return 0;
    }

    return 1;
}

int disconnectServer(int& sock) {
    send(sock, "rpc=5;", strlen("rpc=5;"), 0);
    close(sock);
    return 0;
}

//Menu item for user
int menu(int& sock) {
    string option;

    cout << "\nMenu:\n 1.View item List\n 2.View your cart\n 3.Add item to cart\n 4.remove from list\n 5.discconect " << endl;
    cout << "Enter your option: ";
    getline(cin, option);
    if (option == "1") {
        send(sock, "rpc=1;", strlen("rpc=1;"), 0);
        valread = read(sock, buffer, 1024);
        cout << buffer << endl;

    } else if (option == "2") {
        send(sock, "rpc=2;", strlen("rpc=2;"), 0);
        valread = read(sock, buffer, 1024);
        cout << buffer << endl;
    } else if (option == "3") {
        send(sock, "rpc=3;", strlen("rpc=3;"), 0);
        valread = read(sock, buffer, 1024);
        cout << buffer << endl;
    } else if (option == "4") {
        send(sock, "rpc=4;", strlen("rpc=4;"), 0);
        valread = read(sock, buffer, 1024);
        cout << buffer << endl;

    } else if (option == "5") {
        return disconnectServer(sock);

    } else
        cout << "Invalid option!\n";

    return 1;
}
int main(int argc, char const* argv[]) {
    int sock = 0;
    int status = 0;
    char buff[128];

    status = connectToServer((char*)argv[1], (char*)argv[2], sock);
    int temp = connectRPC(sock);
    while (status && temp) {
        status = menu(sock);
    }
    return 0;
}
