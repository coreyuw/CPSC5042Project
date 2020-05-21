/*
 * Hung Huynh, Sam Borhan, Corey Zhou
 * CPSC 5042, Seattle University
 * Server side C/C++ program to demonstrate Socket programming
 */

#include <bits/stdc++.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <string>

#include "assert.h"
using namespace std;
#define PORT 12110
int server_fd, new_socket, valread;
struct sockaddr_in address;
int addrlen = sizeof(address);

/*
setup server and sockets
*/
void setUp() {
    int opt = 1;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    cout << "Done setup, Server now is running" << endl;
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address,
                             (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////Mike code//////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
Extract key and value as a pair<key,value>
@param pRawKey raw input
@param rpcKeyValue key value pair
@return pair<key,value>
*/
pair<char*, char*> extractKeyValue(RawKeyValueString* pRawKey, KeyValue& rpcKeyValue) {
    pRawKey->getNextKeyValue(rpcKeyValue);
    return {rpcKeyValue.getKey(), rpcKeyValue.getValue()};
}

/*
Check user name & password
@param pszUserName username 
@param pszPass password
@rerturn -1 or 0
*/
int Connect(char* pszUserName, char* pszPass) {
    map<string, string> hashMap;
    hashMap["sam"] = "123";
    hashMap["corey"] = "123";
    hashMap["hung"] = "123";
    hashMap["mike"] = "123";
    const char* pass = hashMap[pszUserName].c_str();
    if (strcmp(pass, pszPass) == 0) {
        return 0;
    }

    return -1;
}
//add item to cart
int addItemtoCart(vector<int>& cart) {
    //check if item is on the hashmap itemlist (corey?)
    //if yes -->
    // check item in cart
    //if yes, return 1
    //if no add to cart, return 0
    //if no --> return -1
    return 0;
}
int main(int argc, char const* argv[]) {
    setUp();
    vector<int> cart;  //cart contain users items
    char buffer[1024] = {0};
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        valread = read(new_socket, buffer, 1024);
        if (valread != 0) {  //<= valread=0 if client disconnect

            RawKeyValueString* pRawKey = new RawKeyValueString((char*)buffer);
            KeyValue rpcKeyValue;

            //rpc.first = key
            //rpc.second = value
            pair<char*, char*> rpc = extractKeyValue(pRawKey, rpcKeyValue);

            if (strcmp(rpc.first, "rpc") == 0) {
                if (strcmp(rpc.second, "connect") == 0) {
                    // Get the next two arguments (user and password);
                    //cant resue rpc above
                    //if you do userkeyvalue and passkeyvalue will point at a same memory address
                    KeyValue user, pass;
                    pair<char*, char*> userKeyValue = extractKeyValue(pRawKey, user);
                    pair<char*, char*> passKeyValue = extractKeyValue(pRawKey, pass);

                    int status;
                    status = Connect(userKeyValue.second, passKeyValue.second);
                    if (status == 0) {
                        cout << userKeyValue.second << " is connected now...\n";
                        send(new_socket, "\nWelcome from server!", strlen("welcome from server!\n"), 0);
                    } else {
                        cout << "client username or password incorrect" << endl;
                        send(new_socket, "Not Authorized", strlen("Not Authorized"), 0);
                    }
                } else if (strcmp(rpc.second, "1") == 0) {
                    send(new_socket, "implement view List", strlen("implement view List"), 0);
                } else if (strcmp(rpc.second, "2") == 0) {
                    send(new_socket, "implement view cart", strlen("implement view cart"), 0);
                } else if (strcmp(rpc.second, "3") == 0) {
                    int status = addItemtoCart(cart);
                    switch (status) {
                        case 0:
                            send(new_socket, "Sucessfully added to cart", strlen("Sucessfully added to cart"), 0);
                            break;
                        case -1:
                            send(new_socket, "Item is not on the list", strlen("Item is not on the list"), 0);
                            break;
                        default:
                            send(new_socket, "Item is already on the cart", strlen("Item is already on the cart"), 0);
                            break;
                    }
                } else if (strcmp(rpc.second, "4") == 0) {
                    send(new_socket, "implement remove from cart", strlen("implement remove from cart"), 0);
                } else {
                    send(new_socket, "You are disconnected from Server", strlen("Your are disconnected from Server"), 0);
                }
                delete pRawKey;
            }
        } else {
            cout << "client just disconnected" << endl;
            //wait for new socket connect
            if ((new_socket = accept(server_fd, (struct sockaddr*)&address,
                                     (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
        }
    }
    return 0;
}
