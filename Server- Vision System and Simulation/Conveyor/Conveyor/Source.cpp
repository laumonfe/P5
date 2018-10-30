
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib") //Required for WinSock
#include <WinSock2.h> //For win sockets
#include <string> //For std::string
#include <iostream> //For std::cout, std::endl, std::cin.getline

SOCKET Connection;//This client's connection to the server

void ClientThread()
{
	char buffer[256]; //Create buffer to hold messages up to 256 characters
	while (true)
	{
		recv(Connection, buffer, sizeof(buffer), NULL); //receive buffer
		std::cout << buffer << std::endl; //print out buffer
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
	addr.sin_addr.s_addr = inet_addr("172.17.168.237"); //Address = localhost (this pc)
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


	char buffer[256]; //256 char buffer to send message

	send(Connection, "Conveyor#10110", sizeof(buffer), NULL); //Send buffer

	while (true)
	{
		std::cin.getline(buffer, sizeof(buffer)); //Get line if user presses enter and fill the buffer

		send(Connection, buffer, sizeof(buffer), NULL); //Send buffer
		Sleep(100);
	}
	return 0;
}



