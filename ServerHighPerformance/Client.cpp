
/*
Protocolo definition:

|------------|--------------|-------------------------------------------------------|
|   Field    | Size (bytes) |                     Desription                        |
|------------|--------------|-------------------------------------------------------|
|    Size    |		1       |   Size of data. MAX Size = 255 bytes                  |
|  (Header)  |              |                                                       |
|------------|--------------|-------------------------------------------------------|
|   Data     |  1 to 255    |  Data of message                                      |
|------------|--------------|-------------------------------------------------------|
*/

#include "Client.h"
#include <iostream>
#include <string>

using namespace demo;
using namespace std;

Client::Client(SOCKET s)
{
	socket = s;
	bWaitForHeader = true;
	dataSizeAwaited = 0;

	debug = false;
	_closed = false;
}


Client::~Client()
{
}

bool Client::isClosed()
{
	return _closed;
}

void Client::setDebug(bool bDebug)
{
	debug = bDebug;
}

void Client::reverseBuffer(char* buffer, int size)
{
	int iL, iR;
	bool bKeepDoing = true;
	
	iL = 0;
	iR = size - 1;
	while (bKeepDoing)
	{	
		if ((iL == iR) || (iL > iR))
			bKeepDoing = false;
		else
		{
			char cAux = buffer[iL];
			buffer[iL] = buffer[iR];
			buffer[iR] = cAux;
			iL++;
			iR--;
		}
	}//While
}

bool Client::newDataArrived()
{
	bool bDisconnected = false;

	if (bWaitForHeader)
	{
		if(debug)
			cout << "Rx:[" << socket << "] Getting Header..." << endl;
		dataSizeAwaited = getHeaderSize(bDisconnected); //Get the header which has the size of data.

		if (dataSizeAwaited > 0)
		{
			if (debug)
				cout << "Rx:[" << socket << "] The expected Size is 0, the socket will be closed." << endl;
			bWaitForHeader = false; //Change the status to WaitForData
		}
	}
	else
	{
		if (debug)
			cout << "Rx:[" << socket << "] Getting Data of " << dataSizeAwaited  << "bytes..." << endl;

		int errorCode = 0;
		int recived = 0;
		char* buffer = rx(dataSizeAwaited, recived, errorCode, bDisconnected);

		if (buffer != NULL)
		{
			if (debug)
				cout << "Rx:[" << socket << "] - Size:[" << dataSizeAwaited << "], Data[" << string(buffer) << "]" << endl;

			reverseBuffer(buffer, dataSizeAwaited);

			if (debug)
				cout << "Tx:[" << socket << "] - Size:[" << dataSizeAwaited << "], Data[" << string(buffer) << "]" << endl;
			
			char bufferSize[1] = { (char)(0x00ff & dataSizeAwaited) };

			bDisconnected = tx(bufferSize, 1); //Send Header
			bDisconnected = tx(buffer, dataSizeAwaited); //SendData
			delete[] buffer;
		}
		else
		{
			cout << "Rx:[" << socket << "] The expected Buffer is NULL, the socket will be closed." << endl;
			bDisconnected = true;
		}

		bWaitForHeader = true;
		dataSizeAwaited = 0;
	}

	_closed = bDisconnected;
	return bDisconnected;
}

int Client::tx(char* buffer, int size)
{
	int errCode = 0;
	int brec = 0;

	if (buffer == NULL || size <= 0)
		return -1;

	brec = send(socket, buffer, size, 0);

	if (brec == SOCKET_ERROR)
	{
		errCode = WSAGetLastError();
		cout << "Client [" << socket << "] Tx-Error : " << errCode << endl;
	}

	return errCode;
}

int Client::getHeaderSize(bool &bClosed)
{
	int size= 1; //The header has a size of 1 bytes
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

	recived = 0;
	if (size <= 0)
	{
		cout << "RxId[" << socket << "] - Its not possible wait for 0 bytes or less in Socket Reception process. Excpected Ammount:[" << size << "]" << endl;
		errorCode = -1;
		return NULL;
	}

	buffer = new char[size + 1];
	memset(buffer, '\0', size + 1);

	missingBytes = size - bytesRecived;

	while (missingBytes > 0)
	{
		int recived = recv(socket, (char*)(buffer + bytesRecived), missingBytes, 0);

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

	recived = bytesRecived;
	return (char*)buffer;
}



