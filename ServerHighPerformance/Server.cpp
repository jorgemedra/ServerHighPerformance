
#include "Server.h"
#include "Client.h"
#include <queue>
#include <iostream>
#include <functional>
#include <future>

using namespace demo;

Server::Server()
{
	bStarted = false;
	bKeepRunning = false;
	this->_port = 0;
	p_thread = NULL;
	debug = false;
}

Server::~Server()
{
	
}

void Server::setDebug(bool bDebug)
{
	debug = bDebug;
}

void Server::start(int port)
{
	if (bStarted)
		return;
	//this->_port = port;
	this->bStarted = true;
	
	auto callBackFUnction = std::bind(&Server::Run, this, placeholders::_1);
	p_thread = new std::thread(callBackFUnction, port);
}

void Server::stop()
{
	if (bStarted)
	{
		cout << "Stoping server and closing connections..." << endl;
		bKeepRunning = false;
		
		closesocket(server); //raise the select condition to continue with thread

		if (p_thread != NULL) 
		{
			if (p_thread->joinable())
				p_thread->join();
			delete p_thread;
		}

		map<SOCKET, Client*>::iterator it;
		for (it = m_sockets.begin(); it != m_sockets.end(); it++)
		{
			SOCKET s = it->first;
			Client* c = it->second;
			
			closesocket(s);
			delete c;
		}
		m_sockets.clear();
	}


}

int Server::initSocketServer()
{
	//----------------------
	// Initialize Winsock
	WSADATA wsaData;
	sockaddr_in service;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	int res = 0;

	if (iResult != NO_ERROR) {
		return iResult;
	}

	//CREATE THE SOCKET
	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server == INVALID_SOCKET) {
		WSACleanup();
		return INVALID_SOCKET;
	}

	hostent* remoteHost;
	char hostName[255];
	hostName[0] = '\0';
	gethostname(hostName, 255);
	remoteHost = gethostbyname(hostName);
	std::string servIP = inet_ntoa(*(struct in_addr *)remoteHost->h_addr_list[0]);

	_srvHostName.append(servIP);

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *)remoteHost->h_addr_list[0]));
	service.sin_port = htons(_port);

	//Double dot means global namespaces
	res = ::bind(server, (SOCKADDR*)&service, (int)sizeof(service));

	if (res == SOCKET_ERROR) {
		int er = WSAGetLastError();
		closesocket(server);
		return er;
	}

	//----------------------
	// Listen for incoming connection requests.
	// on the created socket
	if (listen(server, 1) == SOCKET_ERROR)
		return WSAGetLastError();

	return WSAGetLastError();

}

int Server::Run(int port)
{
	int err = 0;
	int max_sd = 0;
	fd_set rd_fd;
	fd_set ex_fd;
	queue<SOCKET> qClosed;

	this->_port = port;
	err = initSocketServer();

	//Fallo el inicio de socket server.
	if (err != NO_ERROR)
	{
		cout << endl << "There was a problem to bind Native Port: " << _port << endl;
		bKeepRunning = false;
	}
	else
		bKeepRunning = true;

	cout << endl << "Native Port stablished on port: " << _port << endl;
	
	u_long iModeServer = 0;
	ioctlsocket(server, FIONBIO, &iModeServer); //Set Non-Blocking the socket server. 

	max_sd = server;

	while (bKeepRunning)
	{
		qClosed.empty();

		FD_ZERO(&rd_fd);
		FD_ZERO(&ex_fd);

		FD_SET(server, &rd_fd);
		FD_SET(server, &ex_fd);

		map<SOCKET, Client*>::iterator it;

		for (it = m_sockets.begin(); it != m_sockets.end(); it++)
		{
			SOCKET sk = it->first;
			FD_SET(sk, &rd_fd); //Add all the Sockets Clients that are connected to server.
			FD_SET(sk, &ex_fd);
		}

		int result = select(max_sd, &rd_fd, NULL, &ex_fd, NULL);	//Wait for an event of Reading or Exception

		if (result < 0)
		{
			cout << "There was a problem with the sockets reading." << endl;
			FD_ZERO(&rd_fd);
			FD_ZERO(&ex_fd);
		}

		int ss = WSAGetLastError();

		if (FD_ISSET(server, &rd_fd) && bKeepRunning)
		{
			SOCKET s = accept(server, NULL, NULL);

			if (s > 0)
				max_sd = s; //New Selector.

			//bool reuseadd = true;
			bool keepAlive = true;
			u_long iMode = 1;

			//setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseadd, sizeof(bool)); // Allows the socket to be bound to an address that is already in use
			setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (const char*)&keepAlive, sizeof(bool)); // Enable keep-alive packets for a socket connection
			 
			int iResult = ioctlsocket(s, FIONBIO, &iMode); //Set Non-Blocking the socket.

			connectionRequest(s); //Report New Connection Event
		} // Server

		  //-------------------------------
		  //Check For Clients Events.
		for (it = m_sockets.begin(); it != m_sockets.end(); it++)
		{
			SOCKET s = it->first;
			Client* c = it->second;

			bool bCloseSock = false;

			if (FD_ISSET(s, &rd_fd))
			{
				//Report Data Available, after read Data continue with the other socket.
				bCloseSock = newDataAvailabe(s); //If return False, the socket has been closed.
				
				if (bCloseSock)
					qClosed.push(s);
			}
			else if (FD_ISSET(s, &ex_fd))
				qClosed.push(s);

		}//for Clients

		 //--------------------------------------------------------
		 //Remove sockets that are closed and report the event.
		while (qClosed.size() > 0)
		{
			SOCKET s = qClosed.front();
			
			qClosed.pop();
			m_sockets.erase(s);
			connectionClose(s); //report Close event
		}


	} //while

	serverStoped();

	return 0;
}

void Server::connectionRequest(SOCKET s)
{
	m_sockets[s] = new Client(s);
	cout << "Connection client accepted with Socket ID" << (unsigned int)s << endl;
}

 bool Server::newDataAvailabe(SOCKET s)
{
	 bool bClosed = false;
	map<SOCKET, Client*>::iterator it = m_sockets.find(s);
	if (it != m_sockets.end())
	{
		Client* c = m_sockets[s];
		bClosed = c->newDataArrived(); //Process the incomming message and response it.
	}

	return bClosed;
}

void Server::connectionClose(SOCKET s)
{
	cout << "Connection client " << (unsigned int)s << " closed" << endl;

	if (m_sockets.find(s) != m_sockets.end())
	{
		Client* c = m_sockets[s];
		closesocket(s); //release the socket into the server.
		if (c != NULL)
			delete c;
	}
	closesocket(s);
}

void Server::serverStoped()
{
	this->bStarted = false;
	cout << "Server closed" << endl;
}