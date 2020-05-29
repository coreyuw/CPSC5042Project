

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
char buffer[1024] = { 0 };

////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

string getMessage()
{
	string username;
	string password;
	cout << "\nEnter your username: ";
	getline(cin, username);
	cout << "Enter your password: ";
	getline(cin, password);

	string rpcMessage = "rpc=connect;user=" + username + ";password=" + password + ";";
	const char* messageTOserver = rpcMessage.c_str();

	return messageTOserver;
}

string signUpUser()
{
	string username;
	string password;
	cout << "\nEnter your username: ";
	getline(cin, username);
	cout << "Enter your password: ";
	getline(cin, password);

	string rpcMessage = "rpc=signUp;user=" + username + ";password=" + password + ";";
	const char* messageTOserver = rpcMessage.c_str();

	return messageTOserver;
}

//Connnect Rpc
int connectRPC(int& sock)
{
	string option;
	cout << "Do You want to connect to the server?\n";
	while (true)
	{
		memset(buffer, 0, 1024);
		cout << "\nMenu:\n1.Sign in\n2.Sign up\n3.Exit" << endl;
		cout << "Enter number for option: ";
		getline(cin, option);
		if (option.compare("1") == 0)
		{
			const char* messageTOserver = getMessage().c_str();
			send(sock, messageTOserver, strlen(messageTOserver), 0);
			cout << "\nServer is busy. Please wait ..." << endl;
			valread = (int)(int)read(sock, buffer, 1024);
			if (strcmp(buffer, "Not Authorized") == 0)
			{
				cout << buffer << "!";
				cout << "\n";
			}
			else
			{
				cout << buffer;
				cout << "\n";
				return 1;
			}
		}
		else if (option.compare("2") == 0)
		{
			const char* messageTOserver = signUpUser().c_str();
			send(sock, messageTOserver, strlen(messageTOserver), 0);
			valread = (int)read(sock, buffer, 1024);
			cout << buffer << endl;
		}
		else if (option.compare("3") == 0)
		{
			break;
		}
		else
		{
			cout << "Invalid option!\n";
		}
	}
	return 0;
}

int connectToServer(char* szHostName, char* szPort, int& sock)
{
	struct sockaddr_in serv_addr;

	serv_addr.sin_family = AF_INET;
	uint16_t port = (uint16_t)atoi(szPort);

	serv_addr.sin_port = htons(port);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return 0;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	if (inet_pton(AF_INET, szHostName, &serv_addr.sin_addr) <= 0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return 0;
	}

	if (connect(sock, (struct sockaddr*) & serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return 0;
	}

	return 1;
}

/*
DisconnectRPC
*/
int disconnectServer(int& sock)
{
	send(sock, "rpc=5;", strlen("rpc=5;"), 0);
	valread = (int)read(sock, buffer, 1024);
	cout << buffer << endl;
	close(sock);
	return 0;
}

string rpcAddItem()
{
	string input;
	string sendMess = "rpc=3;";
	cout << "Enter ID item you want to add (0 to exit)" << endl;
	getline(cin, input);
	if (atoi(input.c_str()) == 0)
	{
		cout << "digit number only or user exist rpc" << endl;
		return "";
	}
	sendMess += input + "=";
	cout << "Enter the quantity you want to add (0 to exit)" << endl;
	getline(cin, input);
	if (atoi(input.c_str()) == 0)
	{
		cout << "digit number only or user exist rpc" << endl;
		return "";
	}
	sendMess += input + ";";
	return sendMess;
}
string rpcDeleteItem()
{
	string input;
	string sendMess = "rpc=4;";
	cout << "Enter ID item you want to delete (0 to exit)" << endl;
	getline(cin, input);
	if (atoi(input.c_str()) == 0)
	{
		cout << "digit number only or user exist rpc" << endl;
		return "";
	}
	sendMess += input + "=";
	cout << "Enter the quantity you want to delete (0 to exit)" << endl;
	getline(cin, input);
	if (atoi(input.c_str()) == 0)
	{
		cout << "digit number only or user exist rpc" << endl;
		return "";
	}
	sendMess += input + ";";
	return sendMess;
}
//Menu item for user
int menu(int& sock)
{
	memset(buffer, 0, sizeof(buffer));
	string option;

	cout << "\nMenu:\n 1.View item List\n 2.View your cart\n 3.Add item to cart\n 4.remove from list\n 5.discconect " << endl;
	cout << "Enter your option: ";
	getline(cin, option);

	if (option == "1")
	{
		send(sock, "rpc=1;", strlen("rpc=1;"), 0);
		valread = (int)read(sock, buffer, 1024);
		cout << buffer << endl;

	}
	else if (option == "2")
	{
		send(sock, "rpc=2;", strlen("rpc=2;"), 0);
		valread = (int)read(sock, buffer, 1024);
		cout << buffer << endl;
	}
	else if (option == "3")
	{
		string message = rpcAddItem();
		if (!message.empty())
		{
			send(sock, message.c_str(), strlen(message.c_str()), 0);
			valread = (int)read(sock, buffer, 1024);
			cout << buffer << endl;
		}

	}
	else if (option == "4")
	{
		string message = rpcDeleteItem();
		if (!message.empty())
		{
			send(sock, message.c_str(), strlen(message.c_str()), 0);
			valread = (int)read(sock, buffer, 1024);
			cout << buffer << endl;
		}
	}
	else if (option == "5")
	{
		return disconnectServer(sock);

	}
	else
		cout << "Invalid option!\n";

	return 1;
}

int main(int argc, char const* argv[])
{
	int sock = 0;
	int status = 0;

	status = connectToServer((char*)argv[1], (char*)argv[2], sock);
	int temp = connectRPC(sock);
	while (status && temp)
	{
		status = menu(sock);
	}
	close(sock);
	return 0;
}
