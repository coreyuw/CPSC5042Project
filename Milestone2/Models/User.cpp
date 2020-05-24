#include <bits/stdc++.h>
#include "Product.cpp"
#include "Cart.cpp"
using namespace std;
class User
{
    // Data Members
private:
    int id;
    string username;
    string password;
    Cart cart;
    
    // Access specifier 
public:

    User(int newID, string name, string pass) {
        id = newID;
        username = name;
        password = pass;
    }

    ~User{
        cout << "User destructor called for "<< username << "with id: " << id << endl;
    }

    // Member Functions() 

    int getID() {
        return id;
    }

    void setID(int newID) {
        id = newID;
    }

    string getUsername()
    {
        return username;
    }

    void setUsername(string name)
    {
        username = name;
    }

    string getPassword()
    {
        return password;
    }

    void setPassword(string pass)
    {
        password = pass;
    }

};