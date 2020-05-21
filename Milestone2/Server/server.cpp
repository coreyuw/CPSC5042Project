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

using namespace std;
#define PORT 8675

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
        if (bind(m_server_fd, (struct sockaddr*) & m_address,
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

        if ((new_socket = accept(m_server_fd, (struct sockaddr*) & m_address,
            (socklen_t*)&m_addrlen)) < 0) {
            perror("accept");
            return (-1);
        }
        return new_socket;
    }

    int chatter(int new_socket) {
        int valread;
        char buffer[1024] = { 0 };
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

void* rpcThread(void* arg) {
    int nSocket = *(int*)socket;
    int valread;
    char buffer[1024] = { 0 };
    void* status = NULL;

    // We will  block when there are too many connections. Lets say 100
    ServerContextData* pServerContextData = (ServerContextData*)arg;
    nSocket = pServerContextData->getSocket();
    ConnectionContextData* connectionContextDataObj = new ConnectionContextData();
    for (;;) {
        const char* statusOK = "STATUS=OK";
        valread = (int)read(nSocket, (void*)buffer, (size_t)1024);
        if (strcmp(buffer, "QUIT") == 0) {
            connectionContextDataObj->addRpcAmount();
            pServerContextData->incrementTotalRpc();
            printf("Client with socket %d is leaving.\nTotal Server RPC Count: %d\n", nSocket, pServerContextData->getTotalRpc());
            pthread_exit(status);
        }

        int incAmt = atoi((char const*)buffer);
        connectionContextDataObj->addSumAmount(incAmt);
        connectionContextDataObj->addRpcAmount();
        pServerContextData->incrementTotalRpc();
        printf("%s Bytes read = %d  from socket %d\n", buffer, valread, nSocket);
        printf("sumAmt=%d  ThreadRPCAmt=%d\n", connectionContextDataObj->getSumAmount(), connectionContextDataObj->getRpcAmount());
        send(nSocket, statusOK, strlen(statusOK), 0);

        printf("Response message sent.\n");
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
    int rpcAmt = serverContextDataObj->getTotalRpc();
    printf("Total Server RPC Count: %d\n", rpcAmt);

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
        printf("Server accepted one thread. On to another!\n");
    } while (status == 0);
}

#if 0
/*
    int status = 0;
    do
    {
        pthread_t p1, p2;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
            (socklen_t*)&addrlen)) < 0)
        {
            perror("accept error");
            status = -1;
        }
        printf("Accepted Connected");
        int newClientSocket = new_socket;
        pthread_create(&p1, NULL, serverThread, &newClientSocket);
    } while (status == 0);



}



void *serverThread(void *arg)
{
    int socket = *(int *) arg;
    pthread_mutex_lock(&global_mutex);

    pthread_mutex_unlock(&global_mutex);
}




void *serverThread(void *arg) {
    int i;

    for (i = 0; i < loops; i++) {
        pthread_mutex_lock(&count_mutex);
        counter++;
        pthread_mutex_unlock(&count_mutex);
    }

    return NULL;
}
int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage threads <value > \n");
        exit(-1);
    }


    loops = atoi(argv[1]);
    pthread_t p1, p2;
    pthread_create(&p1, NULL, worker, NULL);
    pthread_create(&p2, NULL, worker, NULL);
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    printf("Final value : %d\n", counter);
    return 0;
*/



// We need to guard the 



//#define PORT 8080
#define PORT 8082
/*
int main(int argc, char const *argv[])
{


    int server_fd, new_socket;
    ssize_t valread;

    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = { 0 };
    const char *hello = "Hello from server";
    printf("Hello world\n");
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("Got Socket\n");
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
        &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    printf("About to bind\n");
    if (bind(server_fd, (struct sockaddr *)&address,
        sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Waiting");

    int status = 0;
    do
    {
        pthread_t p1, p2;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
            (socklen_t*)&addrlen)) < 0)
        {
            perror("accept error");
            status = -1;
        }
        printf("Accepted Connected");
        int newClientSocket = new_socket;
        pthread_create(&p1, NULL, serverThread, &newClientSocket);
    } while (status == 0);


    printf("Only reach this in an error condition");

    // We will create a new thread and pass the new_socket to the thread
    // Thread will receive some type of notification that client disconnected and will terminate
    // we don't have to worry about our children (Threads). Yes we are a bad parent. We simply worry about always taking new connections


    // We will read the very simple HELLO message and return a Hello message back

    valread = read(new_socket, buffer, 1024);
    printf("%s Bytes read = %d\n", buffer, valread);
    // What RPC is it
    // Parse out arguments
    // Call the correct RPC Function

    send(new_socket, hello, strlen(hello), 0);
    printf("Hello message sent\n");
    return 0;
}

*/
#endif