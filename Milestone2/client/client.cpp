

// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <stdlib.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 


int connectRPC()
{
	// Input Arguments are:
	// username
	// password
	// input format="rpc=connect;username=<Your user>;password=<Your password>;"
	// Output arguments are:
	// status     (This will be set to 1 if success and -1 if error)
	// error      ( This will be to blank or an error message)
	// output format="status=<error status>;error=<error or blank>
	return 0;
}

int disconnectRPC()
{
	// input format="rpc=disconnect"
	// output format="status=<error status>; error=<error or blank>;
	return 0;
}

int foobarRPC()
{
	return 0;
}

int incrementRPC(int& sock, char* buff)
{
	size_t valRead = 0;
	char hello[24];
	strcpy(hello, "Hello from client");

	char buffer[1024] = { 0 };
	send(sock, buff, strlen(buff), 0);
	printf("Hello message sent\n");
	if (strcmp(buff, "QUIT") == 0) {
		return 0;
	}
	else {
		valRead = read(sock, buffer, 1024);
		printf("ValRead=%lu buffer=%s\n", valRead, buffer);
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
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);


	if (inet_pton(AF_INET, szHostName, &serv_addr.sin_addr) <= 0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr*) & serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}

	return 0;
}


int disconnectServer(int sock)
{
	int status;
	status = close(sock);
	return status;
}


int main(int argc, char const* argv[])
{
	int sock = 0;
	int status;
	char buff[128];
	// We will find out how many times to send our hello

	status = connectToServer((char*)argv[1], (char*)argv[2], sock);

	strcpy(buff, "1");
	incrementRPC(sock, buff);
	strcpy(buff, "15");
	incrementRPC(sock, buff);
	strcpy(buff, "QUIT");
	incrementRPC(sock, buff);

	status = disconnectServer(sock);

	return status;
}
