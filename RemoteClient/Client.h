#pragma once

#include <WinSock2.h> //This include must be first before any other include, to avoid make reference to Windows.h in another include file.
#include <Ws2tcpip.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

namespace demo_clt
{

	class Client
	{
		SOCKET client;
		string remHost;
		int remPort;

		int getHeaderSize(bool &bClosed);
		char* rx(int size, int &recived, int &errorCode, bool &bClosed);
		int tx(char* buffer, int size);

	public:
		Client(string host, int port);
		~Client();

		int openConnection();
		void closeConnection();

		bool sendData(char* buffer, int size);
		char* receiveData();

	};
}
