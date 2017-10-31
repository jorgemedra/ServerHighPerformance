
#include <iostream>
#include "Server.h"

using namespace std;
using namespace demo;

int main(int argsc, char* args[])
{
	cout << "[Serve] ServerHighPerformance.exe IdPort bDebug."<< endl;
	cout << "[Serve]\tIf bDebug is 0 Debug is Off, otherwise debug is On " << endl;
	cout << "[Serve]\tPort argument received: " << args[1] << endl;
	int port = std::atoi(args[1]);
	bool debug = false;

	if (argsc > 2)
	{
		int dBand = std::atoi(args[2]);
		if (dBand == 0)
			debug = false;
		else
			debug = true;
	}

	cout << "[Serve]\tDebug status: " << debug << endl;

	Server server;
	server.setDebug(debug);
	server.start(port);

	system("PAUSE");

	server.stop();
	
	cout << "Server Demo has ended." << endl;
	system("PAUSE");

	return 0;
}