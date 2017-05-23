#pragma once

#define FD_SETSIZE	2048		//Redefine the max socket connections that the FD_SET could be monitored.

#include <WinSock2.h> //This include must be first before any other include, to avoid make reference to Windows.h in another include file.
#include <Ws2tcpip.h>
#include <map>
#include<thread>
#include<atomic>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

namespace demo
{
	class Client;
	class Server
	{
		std::atomic<bool> bStarted;
		int _port;
		thread* p_thread;

		SOCKET server;
		map<SOCKET, Client*> m_sockets;
		std::string _srvHostName;

		int initSocketServer();

		bool bKeepRunning;
		bool debug;

		void connectionRequest(SOCKET s);
		bool newDataAvailabe(SOCKET s);
		void connectionClose(SOCKET s);
		void serverStoped();

	public:
		Server();
		~Server();

		void setDebug(bool bDebug);

		void start(int port);
		void stop();
		int Run(int port);
		

	};
}
