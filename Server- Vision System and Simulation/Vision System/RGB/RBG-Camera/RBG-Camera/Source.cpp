
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib") //Required for WinSock
#include <WinSock2.h> //For win sockets
#include <string> //For std::string
#include <iostream> //For std::cout, std::endl, std::cin.getline

SOCKET Connection;//This client's connection to the server
std::string rmsg;


void ClientThread()
{
	char buffer[256]; //Create buffer to hold messages up to 256 characters
	char eggchar[256] = ""; //Create array for the eggs
	//int integg[10]; // Optional

	while (true)
	{
		recv(Connection, buffer, sizeof(buffer), NULL); //receive buffer
		//std::cout << buffer << std::endl; //print out buffer
		rmsg = buffer;

		if (rmsg == "RBGStart") {
			//Calculation stuff here
			eggchar[0] = 83;
			eggchar[1] = 48;
			eggchar[2] = 49;
			eggchar[3] = 48;
			eggchar[4] = 49;
			eggchar[5] = 48;
			eggchar[6] = 49;
			eggchar[7] = 48;
			eggchar[8] = 49;
			send(Connection, "RBGDone", sizeof(buffer), NULL); //Send buffer
			send(Connection, eggchar, sizeof(buffer), NULL); //Send buffer
		}
	}
}


int main()
{
	//Winsock Startup
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0)
	{
		MessageBoxA(NULL, "Winsock startup failed", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	SOCKADDR_IN addr; //Address to be binded to our Connection socket
	int sizeofaddr = sizeof(addr); //Need sizeofaddr for the connect function
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Address = localhost (this pc)
	addr.sin_port = htons(11111); //Port = 1111
	addr.sin_family = AF_INET; //IPv4 Socket

	Connection = socket(AF_INET, SOCK_STREAM, NULL); //Set Connection socket
	if (connect(Connection, (SOCKADDR*)&addr, sizeofaddr) != 0) //If we are unable to connect...
	{
		MessageBoxA(NULL, "Failed to Connect", "Error", MB_OK | MB_ICONERROR);
		return 0; //Failed to Connect
	}

	std::cout << "Connected!" << std::endl;
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientThread, NULL, NULL, NULL); //Create the client thread that will receive any data that the server sends.


	char buffer[256] = ""; //256 char buffer to send message

	send(Connection, "RBG Camera#10101", sizeof(buffer), NULL); //Send buffer

	while (true)
	{
		//send(Connection, buffer, sizeof(buffer), NULL); //Send buffer
		Sleep(100);
	}
	return 0;
}



