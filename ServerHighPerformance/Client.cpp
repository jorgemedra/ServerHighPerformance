
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
#include <sstream>

using namespace demo;
using namespace std;

#define HEADER_SIZE		1

Client::Client(SOCKET s):
	socket(s),
	bWaitForHeader(true),
	bStartToRxData(false),
	_expectedSize(HEADER_SIZE),
	_bytesRecived(0),
	debug(false),
	_closed(false),
	bufferIn(0),
	_txCounter(0),
	_rxCounter(0)
{
	
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

void Client::reverseBuffer()
{
	int iL, iR;
	bool bKeepDoing = true;
	
	iL = HEADER_SIZE; //Skip the header byte.
	iR = _expectedSize - 1; //Calcualte the size of data.

	while (bKeepDoing)
	{	
		if ((iL == iR) || (iL > iR))
			bKeepDoing = false;
		else
		{
			char cAux = bufferIn[iL];
			bufferIn[iL] = bufferIn[iR];
			bufferIn[iR] = cAux;
			iL++;
			iR--;
		}
	}//While
}


void Client::startToRecAsync()
{
	bufferIn.clear();	
	bufferIn.resize(HEADER_SIZE, 0x00);
	bWaitForHeader = true;
	_expectedSize = HEADER_SIZE;
	_bytesRecived = 0;

	if (debug)
		cout << "Rx:[" << socket << "] Wait for Header..." << endl;

}

bool Client::readIncommingBytes()
{
	int errorCode = 0;
	bool bConxOpened = false;
	bool bCompleted = false;

	bConxOpened = rx(errorCode, bCompleted);

	if (!bConxOpened)
	{
		cout << "Rx:[" << socket << "] The remote endpoint closed the connection. Error Code: " << errorCode << endl;
		return false;
	}
	else if (bCompleted)
	{
		if (bWaitForHeader)
		{
			int size = (int)(0x00ff & bufferIn[0]);
			
			if (size == 0)
			{
				cout << "Rx:[" << socket << "] Header with Data size equal to CERO, the connection will be closed, according to protocol." << endl;
				return false;
			}
			else
				if (debug)
					cout << "Rx:[" << socket << "] Header with Data size = " << size << "." << endl;

			_expectedSize += size; //Increase the size of bytes expected.
			bWaitForHeader = false;
			bufferIn.resize(_expectedSize); //resize the buffer.

		}
		else //Data completed.
		{
			_rxCounter++;
			if (debug)
				cout << "Rx:[" << socket << "] - Data[" << &bufferIn[1] << "]" << endl;
			
			int size = (int)(0x00ff & bufferIn[0]);
			reverseBuffer();
			bufferIn[0] = (char)(size & 0x00ff);

			if(tx(&bufferIn[0],_expectedSize) == 0)
				_txCounter++;
			
			bWaitForHeader = true;
			startToRecAsync(); //Reinitialize the reception cycle.

			if (_rxCounter == MAX_COUNTER)
				_rxCounter = 0;
			if (_txCounter == MAX_COUNTER)
				_txCounter = 0;
		}
	}

	return bConxOpened;

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


/**
* errorCode: Output parameter to indicated if there was an error to read the buffer.
* bCompleated: Output parameter to indicated if has been readed all the expected bytes.
* this method Return: 
* True if the reception was successful, false if the connection was closed.
*/
bool Client::rx(int &errorCode, bool &bCompleated)
{	
	int missingBytes = 0;

	missingBytes = _expectedSize - _bytesRecived;

	if (missingBytes <= 0)
	{
		cout << "RxId[" << socket << "] - Its not possible wait for 0 bytes or less in Socket Reception process. Excpected Ammount:[" << missingBytes << "]" << endl;
		errorCode = -1;
		return true;
	}

	int recived = recv(socket, &bufferIn[_bytesRecived], missingBytes, 0);

	if (recived <= 0)
	{
		errorCode = WSAGetLastError();
		bCompleated = false;
		bufferIn.clear();
		return false;
	}

	_bytesRecived = _bytesRecived + recived;

	if (_bytesRecived == _expectedSize)
		bCompleated = true;

	return true;
}


string Client::getStatistics()
{
	stringstream out;
	out << "Rx: " << _rxCounter << " - Tx: " << _txCounter;
	return out.str();
}
