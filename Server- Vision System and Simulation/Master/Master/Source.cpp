
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <string>
#include <iostream>

SOCKET Connections[100];
int ConnectionCounter = 0;
int clients[10];
std::string rmsg;
int ronce = 0;
int recwait = 0;

void ClientHandlerThread(int index) //index = the index in the SOCKET Connections array
{
	ConnectionCounter += 1; //Incremenent total # of clients that have connected
	char buffer[256] = ""; //Buffer to receive and send out messages from/to the clients

	while (true)
	{
		
		recv(Connections[index], buffer, sizeof(buffer), NULL); //get message from client
		rmsg = buffer;
		std::cout << rmsg << std::endl;

		if (rmsg == "Conveyor#10110")			{clients[0] = index;}
		if (rmsg == "RBG Camera#10101")			{clients[1] = index;}
		if (rmsg == "Thermal Camera#10011")		{clients[2] = index;}
		if (rmsg == "Robot Client#01010")		{clients[3] = index;}
	

		//Start and stop programs
		if (clients[0] != 0 & rmsg == "ConDone" & ronce == 0) {
			send(Connections[clients[1]], "RBGStart", sizeof(buffer), NULL);//send the chat message to this client
			ronce = 1;
			Sleep(10);
		}

		if (clients[1] != 0 & rmsg == "RBGDone" & ronce == 1) {
			//send(Connections[clients[2]], "ThermStart", sizeof(buffer), NULL);//send the chat message to this client
			//ronce = 2;
			recwait = 1;
			Sleep(10);
		}

		if (clients[3] != 0 & rmsg == "Robot DoneMrMaster" & ronce == 3) {
			send(Connections[clients[0]], "Conveyor Start", sizeof(buffer), NULL);//send the chat message to this client
			ronce = 0; 
			Sleep(10);
		}


		//Recieve egg array messages
		if (clients[1] != 0 & buffer[0] == 83 & ronce == 1 & recwait == 1) { //Recieve egg array with "S" in fromt
			//send(Connections[clients[2]], "ThermStart", sizeof(buffer), NULL);//send the chat message to this client
			//Sleep(10);
			send(Connections[clients[3]], buffer, sizeof(buffer), NULL);//send the chat message to this client ----- Client 2 if sent to thermal
			recwait = 0;
			ronce = 3; //2 if there is a seperate thermal camera ----- But there is not going to be so this is never gonna be changed
			Sleep(10);
		}

	}
}

int main()
{
	//Winsock Startup
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0) //If WSAStartup returns anything other than 0, then that means an error has occured in the WinSock Startup.
	{
		MessageBoxA(NULL, "WinSock startup failed", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	SOCKADDR_IN addr; //Address that we will bind our listening socket to
	int addrlen = sizeof(addr); //length of the address (required for accept call)
	addr.sin_addr.s_addr = inet_addr("172.27.31.180"); //Broadcast locally
	addr.sin_port = htons(11111); //Port
	addr.sin_family = AF_INET; //IPv4 Socket 

	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL); //Create socket to listen for new connections
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr)); //Bind the address to the socket
	listen(sListen, SOMAXCONN); //Places sListen socket in a state in which it is listening for an incoming connection. Note:SOMAXCONN = Socket Oustanding Max Connections

	SOCKET newConnection; //Socket to hold the client's connection
	int ConnectionCounter = 0; //# of client connections
	for (int i = 1; i < 100; i++)
	{
		newConnection = accept(sListen, (SOCKADDR*)&addr, &addrlen); //Accept a new connection
		if (newConnection == 0) //If accepting the client connection failed
		{
			std::cout << "Failed to accept the client's connection." << std::endl;
		}
		else //If client connection properly accepted
		{
			std::cout << "Client Connected!" << std::endl;
			char MOTD[256] = "Welcome!"; //Create buffer with message of the day
			send(newConnection, MOTD, sizeof(MOTD), NULL); //Send MOTD buffer
			Connections[i] = newConnection; //Set socket in array to be the newest connection before creating the thread to handle this client's socket.
			ConnectionCounter += 1; //Incremenent total # of clients that have connected
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandlerThread, LPVOID(i), NULL, NULL); //Create Thread to handle this client. The index in the socket array for this thread is the value (i).
		}
	}

	system("pause");
	return 0;
}