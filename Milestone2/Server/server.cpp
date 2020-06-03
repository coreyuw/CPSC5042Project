#include <cstdio>
#include <cstdlib>
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

// Server side C/C++ program to demonstrate Socket programming

using namespace std;
int userCounter;
map<string, string> hashMap = {
    {"sam", "123"},
    {"corey", "123"},
    {"hung", "123"},
    {"mike", "123"}};

pthread_mutex_t counter_mutex; //PTHREAD_MUTEX_INITIALIZER;

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


////////////////////////////////////////////////////////////////////////////////////////////////
class Product
{
    // Data Members
private:
    int id;
    string name;
    int quantity;

    // Access specifier 
public:

    Product(int newID, string newName, int newQuantity)
    {
        id = newID;
        name = newName;
        quantity = newQuantity;
    }

    ~Product(){
        cout << "Product destructor called for " << name << "with id: " << id << endl;
    }

        // Member Functions() 

        int getID()
    {
        return id;
    }

    void setID(int newID)
    {
        id = newID;
    }

    string getName()
    {
        return name;
    }

    void setName(string newName)
    {
        name = newName;
    }

    int getQuantity()
    {
        return quantity;
    }

    void setQuantity(int newQuantity)
    {
        quantity = newQuantity;
    }

};

class User {
    // Data Members
   private:
    int id;
    string username;
    string password;
    map<string, int> cart;

    // Access specifier
   public:
    User(int newID, string name, string pass) {
        id = newID;
        username = name;
        password = pass;
    }

    ~User() {
        cout << "User destructor called for " << username << "with id: " << id << endl;
    }

    // Member Functions()
    string getUsername() {
        return username;
    }

    string getPassword() {
        return password;
    }

    int addItem(string item, int quantity) {

        if (cart.count(item) == 1) {
            cart[item] +=  quantity;
        } else {
            cart[item] = quantity;
        }
        return 1;
    }

    int deleteItem(string item, int quantity) {
        
        if (cart.count(item) == 1) {
            //if item<quallity
            if (cart[item] < quantity) {
                return 0;
            }
            cart[item] -= quantity;
            if (cart[item] == 0) {
                cart.erase(item);
            }
            return 1;
        }
        return 0;
    }

    map<string, int> getCart() {
        return cart;
    }

};
///////////////////////////////////////////////////////////////////////////////////////////////////


// This class will perform the various network Operations to set up the server

class ServerContextData {
    // You will put in your own "Global Data" that you will share among all the various client connections you have. Change your "getters" to reflect that
    int connectionAmount;
    int nSocket;
    pthread_mutex_t lock;
    pthread_cond_t fill;
    int nMaxAmount;
   
   public:

    ServerContextData() {
        connectionAmount = 0;
        pthread_mutex_init(&lock, NULL);
        pthread_cond_init(&fill, NULL);
        nSocket = 0;
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

    int getSocket() {
        return nSocket;
    }

    void setSocket(int nSocket) {
        this->nSocket = nSocket;
    }
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

    int closeServer() {
        return 0;
    }
};

vector<Product*> storage;
map<string, User*> userMap;

//////////////////////////////////////////////////////////////////////////////////////////////////////////
Product* getProduct(int id)
{
    for (auto& elem : storage)
    {
        if (elem->getID() == id)
        {
            return elem;
        }
    }
    return NULL;
}

int getProductIDFromName(string name)
{
    int id = 0;

    for (auto& elem : storage)
    {
        if (name.compare(elem->getName())==0 )
        {
            id= elem->getID();
        }
    }
    return id;
}

//need to check if product is avaiable 
int isProductAvaible(Product* p, int quantity)
{
    if (p->getQuantity() == 0 || p->getQuantity() - quantity < 0)
    {
        cout << "return 0" << endl;
        return 0;
    }

    //update quantity
    pthread_mutex_lock(&counter_mutex);
    p->setQuantity(p->getQuantity() - quantity);
    pthread_mutex_unlock(&counter_mutex);

    return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int rpcConnect(char* pszUserName, char* pszPass) {
    //Check if username or password is wrong while Sign-in
    if (!userMap.count(pszUserName)) {
          
        return 0;
    }

    User* user = userMap[pszUserName];
    const char* pass = user->getPassword().c_str();

    if (strcmp(pass, pszPass) == 0) {
        return 1;
    }

    return 0;
}

char* authorizedUser(int new_socket, pair<char*, char*> rpc, RawKeyValueString* pRawKey) {
    KeyValue user, pass;
    pair<char*, char*> userKeyValue = extractKeyValue(pRawKey, user);
    pair<char*, char*> passKeyValue = extractKeyValue(pRawKey, pass);

    if (rpcConnect(userKeyValue.second, passKeyValue.second)) {
        cout << userKeyValue.second << " is connected now...\n";
        send(new_socket, "\nWelcome from server!", strlen("welcome from server!\n"), 0);
        pthread_mutex_lock(&counter_mutex);
        userCounter++;
        printf("Number of active users : %d \n", userCounter);
        pthread_mutex_unlock(&counter_mutex);

    }
    else {
        cout << "New User Connection Faild! Incorrect Username or Password! " << endl;
        send(new_socket, "Not Authorized", strlen("Not Authorized"), 0);
        return NULL;
    }
    return userKeyValue.second;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int rpcAddItem(int new_socket, RawKeyValueString* pRawKey, string newUser) {
    KeyValue item;
    pair<char*, char*> itemKeyValue = extractKeyValue(pRawKey, item);

    //get user
    User* user = userMap[newUser];

    //get product 
    Product* product = getProduct(atoi(itemKeyValue.first));

    if (product == NULL)
    {
        send(new_socket, "Product ID not exist!", strlen("Product ID not exist!"), 0);
        return 0;
    }

    //check if the product is avaiable
    if ( isProductAvaible(product, atoi(itemKeyValue.second)) == 0)
    {
        send(new_socket, "Product quantity invalid!", strlen("Product quantity invalid!"), 0);
        return 0;

    }

    int sucess = user->addItem(product->getName(), atoi(itemKeyValue.second));

    send(new_socket, "Add item to cart!", strlen("Add item to cart!"), 0);
    return sucess;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

int rpcDeleteItem(int new_socket, RawKeyValueString* pRawKey, string newUser)
{
    KeyValue item;
    pair<char*, char*> itemKeyValue = extractKeyValue(pRawKey, item);

    int quantity = atoi(itemKeyValue.second);

    //get user
    User* user = userMap[newUser];

    //get cart 
    map<string, int> cart = user->getCart();

    //if (cart.count(itemKeyValue.first) == 0) {
    //    send(new_socket, "Product not in cart, can't delete!", strlen("Product not in cart, can't delete!"), 0);
    //    return 0;
    //}

    //get product 
    Product* product = getProduct(atoi(itemKeyValue.first));

    if (product == NULL)
    {
        send(new_socket, "Product ID not exist!", strlen("Product ID not exist!"), 0);
        return 0;
    }
    
    //give item back to the storage 
    pthread_mutex_lock(&counter_mutex);
    product->setQuantity(product->getQuantity() + quantity);
    pthread_mutex_unlock(&counter_mutex);

    cout << product->getQuantity() << endl;
    int sucess = user->deleteItem(product->getName(), quantity);
    send(new_socket, "Delete item from cart!", strlen("Delete item from cart!"), 0);
    return sucess;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

int rpcViewCart(int new_socket, RawKeyValueString* pRawKey, string newUser)
{
    //get user
    User* user = userMap[newUser];

    //get cart 
    map<string, int> cart = user->getCart();

    if (cart.empty() == 1)
    {
        send(new_socket, "Cart is empty!", strlen("Cart is empty!"), 0);
        return 0;
    }
    else {
        string cartInfo="Cart info:\n";
        for (auto& cartItem : cart)
        {
            cartInfo +="id = "+to_string(getProductIDFromName(cartItem.first))+ "| name = " + cartItem.first + "| Quantity = " + to_string(cartItem.second) + ".\n";
        }
        send(new_socket, cartInfo.c_str(), strlen(cartInfo.c_str()), 0);
        return 1;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
const char* rpcListItem()
{
    string message="";
    for (auto &i:storage)
    {

        message += "id = " + to_string(i->getID()) + "| name=" + i->getName() + "| Quantity=" + to_string(i->getQuantity()) + ".\n";
    }

    return message.c_str();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void* rpcThread(void* arg) {
    int new_socket = *(int*)socket;
    int valread;
    char buffer[1024] = {0};
    void* status = NULL;
    char* newUser;
    
    // We will  block when there are too many connections. Lets say 100
    ServerContextData* pServerContextData = (ServerContextData*)arg;
    new_socket = pServerContextData->getSocket();

    for (;;) {
        valread = (int)read(new_socket, buffer, 1024);
        if (valread != 0) {
            RawKeyValueString* pRawKey = new RawKeyValueString((char*)buffer);
            cout << (char*)buffer << endl;
            KeyValue rpcKeyValue;
            pair<char*, char*> rpc = extractKeyValue(pRawKey, rpcKeyValue);

            if (strcmp(rpc.second, "connect") == 0) {
                newUser = authorizedUser(new_socket, rpc, pRawKey);
            } else if (strcmp(rpc.second, "signUp") == 0) {
                KeyValue user, pass;
                pair<char*, char*> userKeyValue = extractKeyValue(pRawKey, user);
                pair<char*, char*> passKeyValue = extractKeyValue(pRawKey, pass);

                if (userMap.count(userKeyValue.second) > 0) {
                    send(new_socket, "Sorry! User Already Exists!!", strlen("Sorry! User Already Exists!!"), 0);
                } else {
                    
                    pthread_mutex_lock(&counter_mutex);
                    User* newSignUp = new User(1, userKeyValue.second, passKeyValue.second);
                    userMap[userKeyValue.second] = newSignUp;
                    pthread_mutex_unlock(&counter_mutex);
                    send(new_socket, "You successfully Signed up!", strlen("You successfully Signed up!"), 0);
                }
            } else if (strcmp(rpc.second, "1") == 0) {
                const char* message = rpcListItem();
    
                send(new_socket, message, strlen(message), 0);
                cout << newUser << " requested list Item" << endl;
            }

            else if (strcmp(rpc.second, "2") == 0) {
                string str(newUser);
                if (rpcViewCart(new_socket, pRawKey, str))
                {
                    cout << "Send cart item to user" << endl;
                }
            }

            else if (strcmp(rpc.second, "3") == 0) {
                string str(newUser);
                
                if (!rpcAddItem(new_socket, pRawKey, str)) {
                    cout << "problem in additem" << endl;
                }
                else
                {
                    cout << "Add item fore user " <<str <<endl;
                }
            } else if (strcmp(rpc.second, "4") == 0) {
                string str(newUser);
                if (!rpcDeleteItem(new_socket, pRawKey, str))
                {
                    cout << "problem in Deleteitem" << endl;
                }
                else
                {
                    cout << "delete item for user " << str << endl;
                }
            }

            else {

                send(new_socket, "Disconnected from server...", strlen("Disconnected from server..."), 0);
                printf("%s with socket %d is leaving.\n", newUser, new_socket);
                pthread_mutex_lock(&counter_mutex);
                userCounter--;
                printf("Number of active users : %d \n", userCounter);
                pthread_mutex_unlock(&counter_mutex);
                pthread_exit(status);
            }
            memset(buffer, 0, sizeof(buffer));
            delete pRawKey;
        }
    }
    fflush(stdout);
    delete pServerContextData;
    return NULL;
}

//Populate product 
void populateProduct()
{
    Product* one = new Product(1, "toilet paper", 5);
    Product* two = new Product(2, "table", 5);
    Product* three = new Product(3, "mask", 5);
    storage.push_back(one);
    storage.push_back(two);
    storage.push_back(three);
}

//Populate member
void populateUser() {
    User* Hung = new User(1, "hung", "123");
    userMap["hung"] = Hung;

    User* sam = new User(1, "sam", "123");
    userMap["sam"] = sam;

    User* corey = new User(1, "corey", "123");
    userMap["corey"] = corey;
}

int main(int argc, char const* argv[]) {
    pthread_t p1;
    int status;
    int nPort = atoi((char const*)argv[1]);
    Server* serverObj = new Server(nPort);
    ServerContextData* serverContextDataObj;
    serverContextDataObj = new ServerContextData();

    populateUser();
    populateProduct();


    //start the server
    cout << "Sever started" << endl;
    serverObj->startServer();

    do {
        int newSocket = serverObj->acceptNewConnection();
        if (newSocket <= 0) {
            printf("Problems\n");
            status = -1;
        }
        serverContextDataObj->setSocket(newSocket);
        pthread_create(&p1, NULL, rpcThread, (void*)serverContextDataObj);

    } while (status == 0);
    delete serverObj;
    delete serverContextDataObj;
    for (auto& x : userMap)
    {
       delete x.second;
    }
    for (auto& x : storage)
    {
        delete x;
    }
    return 0;
}
