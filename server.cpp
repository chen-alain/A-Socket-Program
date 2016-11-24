/*
ALAIN CHEN
Created 10/29/2016

This program acts as the server for a simple chatroom.
Socket programming is used to send data between clients and a server.

This file is responsible for the server. The server is responsible 
for handling all the logic for logging in, creating new users, and logging out.
*/

#include <stdio.h>
#include "winsock2.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

#define SERVER_PORT   19804
#define MAX_PENDING   5
#define MAX_LINE      256

//A structure that defines a user.
struct User
{
	string username;
	string password;
};

int checkLogin(char buf[], vector<User>& users, string& user);
int newUser(string str2, string str3, vector<User>& users);
void loadUsernames(vector<User>& users);
int loginStatus(string str2, string str3, vector<User>& users, string& user);

void main() {
   
	vector<User> users;
	loadUsernames(users);

	// Initialize Winsock.
	WSADATA wsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if ( iResult != NO_ERROR )
	{
		printf("Error at WSAStartup()\n");
		return;
	}
   
	// Create a socket.
	SOCKET listenSocket;
	listenSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
   
	if ( listenSocket == INVALID_SOCKET ) 
	{
		printf( "Error at socket(): %ld\n", WSAGetLastError() );
		WSACleanup();
		return;
	}
   
	// Bind the socket.
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY; //use local address
	addr.sin_port = htons(SERVER_PORT);
	if ( bind( listenSocket, (SOCKADDR*) &addr, sizeof(addr) ) == SOCKET_ERROR ) 
	{
		printf( "bind() failed.\n" );
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
   
	// Listen on the Socket.
	if ( listen( listenSocket, MAX_PENDING ) == SOCKET_ERROR )
	{
		printf( "Error listening on socket.\n");
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
   
	// Accept connections.
	SOCKET s;
   
	cout << "\nAlain's Chatroom Server Version 1.\n";
	while(true)
	{
		s = accept( listenSocket, NULL, NULL );
		if (s == SOCKET_ERROR) 
		{
			printf("accept() error \n");
			closesocket(listenSocket);
			WSACleanup();
			return;
		}

		bool loggedIn = false;
		bool loggedOut = false;
		string username;

		// Send and receive data.
		char buf[MAX_LINE];

		//Listen for input until user is logged in.
		while (!loggedIn)
		{
			//Receive input from socket.
			int len = recv(s, buf, MAX_LINE, 0);
			buf[len] = '\0';
			
			//Check input before user is loggen in.
			int flag = checkLogin(buf, users, username);
			string str = "";
			
			switch (flag)
			{
			case 0: // 0 - successful log in
				str = "Server: " + username + " joins.";
				loggedIn = true;
				cout << username << " login." << endl;
				break;
			case 1: // 1 - user has not logged in yet
				str = "Server: Denied. Please login first.";
				break;
			case 2: // 2 - incorrect arguments
				str = "Server: Incorrect Arguments. Usage: login username password";
				break;
			case 3: // 3 - wrong password
				str = "Server: Wrong password.";
				break;
			}
			strcpy(buf, str.c_str());
			send(s, buf, strlen(buf), 0);
		}
	
		//Listen for input until user is logged out.
		while (!loggedOut)
		{
			//Receive input from socket.
			int len = recv(s, buf, MAX_LINE, 0);
			buf[len] = 0;
			string str = "";
			string messageToSend = "";

			//Get the first word in the message.
			int i = 0;
			char temp = buf[0];
			while (temp != '\0' && temp != ' ' && temp != '	')
			{
				str += temp;
				temp=buf[++i];
			}

			//Match first word with the correct operation.
			if (str == "logout")
			{
				messageToSend = "Server: " + username + " left.";
				loggedOut = true;
				cout << username << " logout.\n";
			}				
			else if (str == "newuser")
			{
				//Get second word in message.
				if (temp == '\0' || temp == '\n')//The second word does not exist.
					messageToSend = "Server: Invalid Usage: newuser username password";
				else
				{ 
					string str2 = "";
					temp = buf[++i];
					while (temp != '\0' && temp != ' ' && temp != '	' && temp != '\n')
					{
						str2 += temp;
						temp = buf[++i];
					}
					//Get third word in message.
					if (temp == '\0' || temp == '\n')//The third word does not exist.
						messageToSend = "Server: Invalid Usage: newuser username password";
					else
					{
						string str3 = "";
						temp = buf[++i];
						while (temp != '\0' && temp != ' ' && temp != '	' && temp != '\n')
						{
							str3 += temp;
							temp = buf[++i];
						}

						//There are more things after the third argument, 
						//so incorrect number of arguments.
						if (temp != '\0')
							messageToSend = "Server: Invalid Usage: newuser username pswd";
						else
						{
							int flag = newUser(str2, str3, users);
							switch (flag)
							{
							case 0://success
								messageToSend = "Created user " + users.at(users.size() - 1).username;
								cout << "Created user " + users.at(users.size() - 1).username << endl;
								break;
							case 1://Password/Username incorrect length.
								messageToSend = "New user creation fail. Password must be between ";
								messageToSend += "4 and characters, username must be less than 32 characters.";
								break;
							case 2://Username already exists.
								messageToSend = "New user creation fail. Username already exists.";
								break;
							}
						}							
					}
				}
			}
			else if( str == "login" )
			{
				messageToSend = "Server: Already logged in as " + username;
			}
			else if (str == "send")
			{
				//Send out the entirety of mesage except the "send" command.
				messageToSend = username + ": ";
				temp = buf[++i];
				while (temp != '\0' && temp != '\n')
				{
					messageToSend += temp;
					temp = buf[++i];
				}
				cout << messageToSend << endl;
			}
			else
			{
				messageToSend = "Server: Invalid Command";
			}

			strcpy(buf, messageToSend.c_str());

			send(s, buf, strlen(buf), 0);
		}
		closesocket(s);
	}
   
	closesocket(listenSocket);
}

//Checks what the user has enter when it has not yet logged in.
//				 0 - successful log in
//				 1 - user has not logged in yet
//				 2 - incorrect arguments
//				 3 - wrong password
int checkLogin(char buf[], vector<User>& users, string& user)
{
	string str = "";

	//Get the first word in the message.
	int i = 0;
	char temp = buf[0];
	while (temp != '\0' && temp != ' ' && temp != '	' && temp != '\n')//Null, space, or tab.
	{
		str += temp;
		temp = buf[++i];
	}

	//User should log in first.
	if (str != "login")
	{
		return 1;
	}
	
	//Get second word in message.
	if (temp == '\0' || temp == '\n')//The second word does not exist.
		return 2;
	string str2 = "";
	temp = buf[++i];
	while (temp != '\0' && temp != ' ' && temp != '	' && temp != '\n')//Null, space, or tab.
	{
		str2 += temp;
		temp = buf[++i];
	}
	//Get third word in message.
	if (temp == '\0' || temp == '\n')//The third word does not exist.
		return 2;
	string str3 = "";
	temp = buf[++i];
	while (temp != '\0' && temp != ' ' && temp != '	' && temp != '\n')//Null, space, or tab.
	{
		str3 += temp;
		temp = buf[++i];
	}

	if (temp != '\0')//There are more things after the third argument, so incorrect number of arguments.
		return 2;

	if (str == "newuser")
		return newUser(str2, str3, users);

	return loginStatus(str2, str3, users, user);
}

//Creates a new user
//Return codes:  0 - successfully created new user.
//				 1 - password/username lengths not compatible for new user
//				 2 - username already exists
int newUser(string str2, string str3, vector<User>& users)
{
	//Password or username length does not fall into specifications.
	if (str2.length() >= 32 || str3.length() < 4 || str3.length() > 8)
		return 1;

	for (int i = 0, j = users.size(); i < j ; i++)
	{
		//Username already exists.
		if (str2 == users.at(i).username)
			return 2;
	}

	//Write new user to file.
	FILE *fp;
	char nameBuff[35]; 
	strcpy(nameBuff, str2.c_str());
	char passBuff[12];
	strcpy(passBuff, str3.c_str());
	fp = fopen("users.txt", "a");

	fputs("\n(", fp);
	fputs(nameBuff, fp);
	fputs(", ", fp);
	fputs(passBuff, fp);
	fputc(')', fp);

	fclose(fp);

	//Add to list of known users.
	struct User u { str2, str3 };
	users.push_back(u);

	return 0;
}

//Returns whether login was successful or not.
//Return codes:	 0 - successful log in
//				 3 - wrong password
int loginStatus(string str2, string str3, vector<User>& users, string& user)
{
	//Go through each user and check if username/password matches.
	for (int i = 0, n = users.size(); i < n; i++)
	{
		if (users.at(i).username == str2)
		{
			//There is a match.
			if (users.at(i).password == str3)
			{
				user = users.at(i).username;
				return 0;
			}				

			//User entered wrong password for username
			return 3;
		}
	}

	//The username does not exist.
	return 3;
}

//Load the usernames from the users.txt file into memory.
void loadUsernames(vector<User>& users)
{
	FILE *fp;
	char nameBuff[40];
	char passBuff[15];
	fp = fopen("users.txt", "r");

	string sName, sPass;

	while (fscanf(fp, "%s %s", nameBuff, passBuff) >0)
	{
		struct User u;
		sName = nameBuff;
		sPass = passBuff;
		sName.pop_back();//Remove the comma.
		sName.replace(0, 1, "");//Remove the initial '('
		sPass.pop_back(); //Remove the ')'
		u.username = sName;
		u.password = sPass;
		users.push_back(u);
	}

	fclose(fp);
}
