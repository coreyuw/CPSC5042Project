#flag
#or g++ depend on you
#$(CC) is how you use it
CC=g++
CFLAGS= -std=c++11 -Wall -pedantic -ggdb

#The first line is default target for makefile.
#to use type "make" in the terminal (just "make"), it will run "all"
#syntax:
#[name]:[target]
#[tab][command]
all: server client

#"make server" only compiles server 
server: server.cpp 
	$(CC) $(CFLAGS) server.cpp -o server.o
#same here "make client"
client: client.cpp
	$(CC) $(CFLAGS) client.cpp -o client.o
#find the server
find:
	ps -ef|grep server


#make clean here only remove all .o file
#sometime, compiler cant overwrite the old .o file so you have to delete them
#It's best practice to clear all .o files before any compile 
#or you can make clean folder to submit 
clean: 
	rm *.o