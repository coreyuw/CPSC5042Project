#include <bits/stdc++.h> 
#include <vector> 
#include "Product.cpp"
using namespace std;
class Cart
{
    // Data Members
private:
    vector<Product> cart;

    // Access specifier 
public:

    Cart() {

    }

    ~Cart{
        cout << "Cart destructor called for Cart" << endl;
    }

        // Member Functions() 

    //addProd(){}
};