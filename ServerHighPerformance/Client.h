#pragma once

#include <WinSock2.h> //This include must be first before any other include, to avoid make reference to Windows.h in another include file.
#include <Ws2tcpip.h>

namespace demo
{
	class Client
	{
		SOCKET socket;
		bool bWaitForHeader;
		int dataSizeAwaited;

		bool debug;
		bool _closed;

		void reverseBuffer(char* buffer, int size);
		int getHeaderSize(bool &bClosed);
		char* rx(int size, int &recived, int &errorCode, bool &bClosed);
		int tx(char* buffer, int size);

	public:
		Client(SOCKET s);
		~Client();

		void setDebug(bool bDebug);
		bool newDataArrived();

		bool isClosed();
	};
}