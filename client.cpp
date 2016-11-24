/*
ALAIN CHEN
Created 10/29/2016

This program acts as the client for a simple chatroom.
Socket programming is used to send data between clients and a server.

This file is for the client side. The client can log in, create new users,
and send messages to a server.
*/

#include "winsock2.h"
#include <iostream>
#include <string>
using namespace std;

#define SERVER_PORT		19804
#define MAX_LINE		256
#define LOOPBACK_ADD	"127.0.0.1"

void main(int argc, char **argv) {   
	
	// Initialize Winsock.
	WSADATA wsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if ( iResult != NO_ERROR )
	{
		printf("Error at WSAStartup()\n");
		return;
	}
   
	// Create IP address with the loopback address.
	unsigned int ipaddr= inet_addr(LOOPBACK_ADD); 
   
	// Create a socket.
	SOCKET s;

	char buf[MAX_LINE];

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	// Connect to a server.
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ipaddr;
	addr.sin_port = htons(SERVER_PORT);
	if (connect(s, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		printf("Failed to connect.\n");
		WSACleanup();
		return;
	}
		
	string temp;//We want to allow spaces in a message, so we need to use getline.
	cout << "\nAlain's Chatroom Server Version 1.\n";

	//Whether user has logged out yet
	bool loggedout = false;

	while (!loggedout)
	{
		loggedout = false;
		getline(cin, temp);
			
		strcpy(buf, temp.c_str());//Copy contents of the temp string into the buffer.
		send(s, buf, strlen(buf), 0);
		int len = recv(s, buf, MAX_LINE, 0);//Output contents of what server has sent us.
		buf[len] = 0;
		cout << buf << endl;
		string s(buf); //copy buffer into a string.
		temp.resize(6); //We only care about the first 6 characters.
		if (temp == "logout" && s != "Server: Denied. Please login first.")
			loggedout = true;
	}
	closesocket(s);

}
