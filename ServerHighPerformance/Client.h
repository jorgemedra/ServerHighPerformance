#pragma once

#include <WinSock2.h> //This include must be first before any other include, to avoid make reference to Windows.h in another include file.
#include <Ws2tcpip.h>
#include <vector>

using namespace std;

#define MAX_COUNTER 2147483646
namespace demo
{
	class Client
	{
		SOCKET socket;
		bool bWaitForHeader;
		bool bStartToRxData;
		vector<char> bufferIn;

		int _expectedSize;
		int _bytesRecived;

		long _txCounter, _rxCounter;

		bool debug;
		bool _closed;

		void reverseBuffer();

		bool rx(int &errorCode, bool &bCompleated);
		int tx(char* buffer, int size);

	public:
		Client(SOCKET s);
		~Client();

		void startToRecAsync();
		bool readIncommingBytes();
		void setDebug(bool bDebug);
		bool isClosed();
		string getStatistics();
	};
}