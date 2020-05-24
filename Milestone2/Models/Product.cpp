#include <bits/stdc++.h> 
using namespace std;
class Product
{
    // Data Members
private:
    int id;
    string name;
    int quantity;

    // Access specifier 
public:

    Product(int newID, string newName, int newQuantity) {
        id = newID;
        name = newName;
        quantity = newQuantity;
    }

    ~Product{
        cout << "Product destructor called for " << name << "with id: " << id << endl;
    }

    // Member Functions() 

    int getID() {
        return id;
    }

    void setID(int newID) {
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