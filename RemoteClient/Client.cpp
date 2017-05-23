	

#include "Client.h"
#include <iostream>
#include <sstream>

using namespace demo_clt;
using namespace std;

Client::Client(string host, int port)
{
	remHost = host;
	remPort = port;
}

Client::~Client()
{

}

int Client::openConnection()
{
	hostent* remoteHost;
	hostent* lclHost;
	unsigned int addr;
	char server_name[512];
	int err = 0;


	if (remHost.compare("") == 0 || remPort <= 0)
		return -1;


	memset(server_name, '\0', 512);


	///*********************************************************************
#ifdef _WIN32 // _WIN32 is defined by all Windows 32 and 64 bit compilers, but not by others.
	///*********************************************************************
	//----------------------
	// Initialize Winsock
	WSADATA wsaData;
	err = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (err != 0)
		return err;

	///*********************************************************************
#endif
	///*********************************************************************


	//CREA EL SOKET
	WSAGetLastError(); //Clean the error flag.
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (client == INVALID_SOCKET)
	{
		err = WSAGetLastError();
		return err;
	}


	//Establece el HostName o IP Address.
	if (isalpha(remHost.c_str()[0]))     // host address is a name
	{
		remoteHost = gethostbyname(remHost.c_str());
	}
	else
	{
		addr = inet_addr(remHost.c_str());
		remoteHost = gethostbyaddr((char *)&addr, 4, AF_INET);
	}

	err = WSAGetLastError();
	if (err != 0)
		return err;

	char hostName[255];
	char hostNameAux[255];
	hostName[0] = '\0';
	hostNameAux[0] = '\0';
	gethostname(hostName, 255);

	strcpy(server_name, inet_ntoa(*(struct in_addr *)remoteHost->h_addr_list[0]));
	sockaddr_in sockC;
	sockC.sin_family = AF_INET;
	sockC.sin_addr.s_addr = htonl(INADDR_ANY);
	sockC.sin_addr.s_addr = inet_addr(server_name);
	sockC.sin_port = htons((u_short)remPort);


	if ((connect(client, (SOCKADDR*)&sockC, sizeof(sockC)) == -1))
	{
		err = WSAGetLastError();
		return err;
	}

	u_long argp = 1L; //non-blocking mode is enabled.
	if (ioctlsocket(client, FIONBIO, &argp) != 0)
		return WSAGetLastError();

	bool reuseadd = true;
	bool keepAlive = true;

	setsockopt(client, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseadd, sizeof(bool));
	setsockopt(client, SOL_SOCKET, SO_KEEPALIVE, (const char*)&keepAlive, sizeof(bool));

	return 0;
}

void Client::closeConnection()
{
	closesocket(client);
	cout << "Connection closed" << endl;
}

int Client::tx(char* buffer, int size)
{
	int errCode = 0;
	int brec = 0;

	if (buffer == NULL || size <= 0)
		return -1;

	timeval to;
	to.tv_sec = 1;
	to.tv_usec = 0;
	fd_set wr_fd;

	FD_ZERO(&wr_fd);
	FD_SET(client, &wr_fd);

	//cout << "Client [" << socket << "] Wait for Tx." << endl;
	//int res = select(client, NULL, &wr_fd, NULL, &to);
	int res = select(client, NULL, &wr_fd, NULL, NULL);

	if(res != 1)
		cout << "Client [" << socket << "] Select ERROR: " << res << endl;
	
	if (FD_ISSET(client, &wr_fd))
	{

		brec = send(client, buffer, size, 0);

		if (brec == SOCKET_ERROR)
		{
			errCode = WSAGetLastError();
			cout << "Client [" << socket << "] Tx-Error : " << errCode << endl;
		}
		//else
		//	cout << "Client [" << socket << "] Tx-OK." << endl;
	}
	else
	{
		cout << "Client [" << socket << "] Tx-Error By Select : " << WSAGetLastError() << endl;
	}
	return errCode;
}

int Client::getHeaderSize(bool &bClosed)
{
	int size = 1; //The header has a size of 1 bytes
	int recived = 0;
	int errorCode = 0;

	char* buffer = rx(size, recived, errorCode, bClosed);

	if (errorCode == 0 && !bClosed && recived > 0)
	{
		size = (int)(0x00ff & buffer[0]);
		delete buffer;
	}
	else
		size = 0;

	return size;
}

char* Client::rx(int size, int &recived, int &errorCode, bool &bClosed)
{
	char* buffer = NULL;
	int bytesRecived = 0;
	int missingBytes = 0;

	timeval to;
	to.tv_sec = 1;
	to.tv_usec = 0;

	fd_set rd_set;
	FD_ZERO(&rd_set);
	FD_SET(client, &rd_set);

	recived = 0;
	if (size <= 0)
	{
		cout << "RxId[" << socket << "] - Its not possible wait for 0 bytes or less in Socket Reception process. Excpected Ammount:[" << size << "]";
		errorCode = -1;
		return NULL;
	}

	buffer = new char[size + 1];
	memset(buffer, '\0', size + 1);

	missingBytes = size - bytesRecived;

	//select(client, &rd_set, NULL, NULL, &to);
	select(client, &rd_set, NULL, NULL, NULL);

	if (FD_ISSET(client, &rd_set))
	{
		while (missingBytes > 0)
		{
			int recived = recv(client, (char*)(buffer + bytesRecived), missingBytes, 0);

			if (recived <= 0)
			{
				errorCode = WSAGetLastError();
				bClosed = true;

				if (buffer != NULL)
					delete[] buffer;

				return NULL;
			}

			bytesRecived = bytesRecived + recived;
			missingBytes = size - bytesRecived;
		}
	}
	else
	{
		cout << "RxId[" << socket << "] - TIME OUT RX" << endl;
		bytesRecived = 0;
		if(buffer != NULL)
			delete[] buffer;
	}
	recived = bytesRecived;
	return (char*)buffer;
}

bool Client::sendData(char* buffer, int size)
{
	bool result = true;

	char buffAck[1];

	buffAck[0] = (char)(0x000ff & size);

	//Si hay fatos que enviar
	if (size > 0 && buffer != NULL)
	{
		int brec = 0;

		if (buffer == NULL || size <= 0)
			return -1;

		int result = tx(buffAck, 1); //Header
		if (result == 0)
		{
			result = tx(buffer, size); //Data
			if (result != 0)
				result = false;
		}
		else
			result = false;

	}
	else
	{
		result = false;
	}


	return result;
}

char* Client::receiveData()
{
	bool bClosed = false;
	char* buffer = NULL;
	int received = 0;
	int erroCode = 0;
	
	int headerSize = getHeaderSize(bClosed);

	if (headerSize > 0)
	{
		buffer = rx(headerSize, received, erroCode, bClosed);

		if (received != headerSize && buffer != NULL)
		{
			delete[] buffer;
			buffer = NULL;
		}
	}
	return buffer;
}
