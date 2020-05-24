#include <cstdio>

// Server side C/C++ program to demonstrate Socket programming
#include <assert.h>
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
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
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
// This class will perform the various network Operations to set up the server

class ServerContextData {
    // You will put in your own "Global Data" that you will share among all the various client connections you have. Change your "getters" to reflect that
    int connectionAmount;
    int rpcAmount;
    int nSocket;
    pthread_mutex_t lock;
    pthread_cond_t fill;
    int nMaxAmount;

   public:
    ServerContextData() {
        connectionAmount = 0;
        rpcAmount = 0;
        pthread_mutex_init(&lock, NULL);
        pthread_cond_init(&fill, NULL);
        nSocket = 0;
        nMaxAmount = 0;
    }
    int getConnectionAmount() {
        int nConAmount;
        pthread_mutex_lock(&lock);
        nConAmount = connectionAmount;
        pthread_mutex_unlock(&lock);
        return nConAmount;
    }

    void incrementConnectionAmount() {
        pthread_mutex_lock(&lock);
        connectionAmount++;
        pthread_mutex_unlock(&lock);
    }

    void incrementTotalRpc() {
        pthread_mutex_lock(&lock);
        rpcAmount++;
        pthread_mutex_unlock(&lock);
    }

    int getTotalRpc() {
        int nRpcAmount;
        pthread_mutex_lock(&lock);
        nRpcAmount = rpcAmount;
        pthread_mutex_unlock(&lock);
        return nRpcAmount;
    }

    int getSocket() {
        return nSocket;
    }

    void setSocket(int nSocket) {
        this->nSocket = nSocket;
    }
};

class ConnectionContextData {
    // You will put in your own "Global Data" that you will share among all the various client connections you have. Change your "getters" to reflect that

   public:
    ConnectionContextData() {
        rpcAmount = 0;
    }

    void addRpcAmount() {
        rpcAmount++;
    }

    void addSumAmount(int incAmt) {
        sumAmount += incAmt;
    }

    int getRpcAmount() {
        return rpcAmount;
    }

    int getSumAmount() {
        return sumAmount;
    }

   private:
    int rpcAmount = 0;
    int sumAmount = 0;
};

class Server {
   private:
    int m_server_fd;
    struct sockaddr_in m_address;
    int m_addrlen = sizeof(m_address);
    int m_port;

   public:
    Server(int nPort) {
        m_port = nPort;
    }

    ~Server() {
    }

    int startServer() {
        int opt = 1;

        // Creating socket file descriptor
        if ((m_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // Forcefully attaching socket to the port 8080
        if (setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                       &opt, sizeof(opt))) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        m_address.sin_family = AF_INET;
        m_address.sin_addr.s_addr = INADDR_ANY;

        m_address.sin_port = (uint16_t)htons((uint16_t)m_port);

        // Forcefully attaching socket to the port 8080
        if (bind(m_server_fd, (struct sockaddr*)&m_address,
                 sizeof(m_address)) < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        if (listen(m_server_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        return 0;
    }

    // Socket returned from function is going to be specific to the client that is connecting to us

    int acceptNewConnection() {
        int new_socket;

        if ((new_socket = accept(m_server_fd, (struct sockaddr*)&m_address,
                                 (socklen_t*)&m_addrlen)) < 0) {
            perror("accept");
            return (-1);
        }
        return new_socket;
    }

    int chatter(int new_socket) {
        int valread;
        char buffer[1024] = {0};
        const char* hello = "Hello from server";
        valread = (int)read(new_socket, (void*)buffer, (size_t)1024);
        printf("%s Bytes read = %d\n", buffer, valread);
        // What RPC is it
        // Parse out arguments
        // Call the correct RPC Function

        send(new_socket, hello, strlen(hello), 0);
        printf("Hello message sent\n");
        return 0;
    }

    int closeServer() {
        return 0;
    }
};

int rpcConnect(char* pszUserName, char* pszPass) {
    map<string, string> hashMap;
    hashMap["sam"] = "123";
    hashMap["corey"] = "123";
    hashMap["hung"] = "123";
    hashMap["mike"] = "123";
    const char* pass = hashMap[pszUserName].c_str();
    if (strcmp(pass, pszPass) == 0) {
        return 1;
    }

    return 0;
}

void isUserAuthorized(int new_socket, pair<char*, char*> rpc, char*& newUser, RawKeyValueString* pRawKey) {

    if (strcmp(rpc.second, "connect") == 0) {
        KeyValue user, pass;
        pair<char*, char*> userKeyValue = extractKeyValue(pRawKey, user);
        pair<char*, char*> passKeyValue = extractKeyValue(pRawKey, pass);

        if (rpcConnect(userKeyValue.second, passKeyValue.second)) {
            newUser = userKeyValue.second;
            cout << newUser << " is connected now...\n";
            send(new_socket, "\nWelcome from server!", strlen("welcome from server!\n"), 0);
        }
        else {
            cout << "client username or password incorrect" << endl;
            send(new_socket, "Not Authorized", strlen("Not Authorized"), 0);
            return;
        }
    }
}
void* rpcThread(void* arg) {
    int new_socket = *(int*)socket;
    int valread;
    char buffer[1024] = {0};
    void* status = NULL;
    char* newUser;

    // We will  block when there are too many connections. Lets say 100
    ServerContextData* pServerContextData = (ServerContextData*)arg;
    new_socket = pServerContextData->getSocket();
    ConnectionContextData* connectionContextDataObj = new ConnectionContextData();

    for (;;) {
        valread = (int)read(new_socket, (void*)buffer, (size_t)1024);
        if (valread != 0) {
            RawKeyValueString* pRawKey = new RawKeyValueString((char*)buffer);
            KeyValue rpcKeyValue;
            pair<char*, char*> rpc = extractKeyValue(pRawKey, rpcKeyValue);
            
            if (strcmp(rpc.second, "connect") == 0) {
                newUser = authorizedUser(new_socket, rpc, newUser, pRawKey);
            }
             else if (strcmp(rpc.second, "1") == 0) {
                send(new_socket, "implement view List", strlen("implement view List"), 0);
            }

            else if (strcmp(rpc.second, "2") == 0) {
                send(new_socket, "implement view cart", strlen("implement view cart"), 0);
            }

            else if (strcmp(rpc.second, "3") == 0) {
                send(new_socket, "implement add cart", strlen("implement add cart"), 0);
            }

            else if (strcmp(rpc.second, "4") == 0) {
                send(new_socket, "implement remove from cart", strlen("implement remove from cart"), 0);
            }

            else {
               
                printf("%s with socket #%d is disconnected now!\n", newUser, new_socket);
                pthread_exit(status);
            }

            delete pRawKey;
        }

        // if (strcmp(buffer, "QUIT") == 0) {
        //     connectionContextDataObj->addRpcAmount();
        //     pServerContextData->incrementTotalRpc();
        //     printf("Client with socket %d is leaving.\nTotal Server RPC Count: %d\n", nSocket, pServerContextData->getTotalRpc());
        //     pthread_exit(status);
        // }

        // int incAmt = atoi((char const*)buffer);
        // connectionContextDataObj->addSumAmount(incAmt);
        // connectionContextDataObj->addRpcAmount();
        // pServerContextData->incrementTotalRpc();
        // printf("%s Bytes read = %d  from socket %d\n", buffer, valread, nSocket);
        // printf("sumAmt=%d  ThreadRPCAmt=%d\n", connectionContextDataObj->getSumAmount(), connectionContextDataObj->getRpcAmount());
        // send(nSocket, statusOK, strlen(statusOK), 0);

        // printf("Response message sent.\n");
    }
    fflush(stdout);

    return NULL;
}

int main(int argc, char const* argv[]) {
    pthread_t p1;
    int status;
    int nPort = atoi((char const*)argv[1]);
    Server* serverObj = new Server(nPort);
    ServerContextData* serverContextDataObj;
    serverContextDataObj = new ServerContextData();

    //start the server
    serverObj->startServer();

    do {
        int newSocket = serverObj->acceptNewConnection();
        if (newSocket <= 0) {
            printf("Problems\n");
            status = -1;
        }
        //	serverObj->chatter(newSocket);
        serverContextDataObj->setSocket(newSocket);
        pthread_create(&p1, NULL, rpcThread, (void*)serverContextDataObj);
      //  printf("Server accepted one thread. On to another!\n");
    } while (status == 0);
}
