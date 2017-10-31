#include "Client.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <fstream>
#include <future>

using namespace std;
using namespace demo_clt;


void writeResult(string data)
{
	cout << data << endl;
	
	std::fstream fs;
	fs.open("output_client.log", std::fstream::out | std::fstream::app);
	fs << data << endl;
	fs.close();
	
}

bool bRunning;
void keepRunning()
{
	system("PAUSE");
	bRunning = false;
}

int main(int argsc, char* args[])
{
	clock_t t_Pbegin, t_Pend;
	clock_t t_begin, t_end;
	
	bRunning = true;

	t_Pbegin = clock(); //Start Transmitions

	cout << "[Client] RemoteClient.exe Host Port NumMessages bDebug." << endl;
	cout << "[Client]\tIf bDebug is 0 Debug is Off, else debug is On " << endl;
	string basicData = "En general, los hombres juzgan mas por los ojos que por la inteligencia, pues todos pueden ver, pero pocos comprenden lo que ven.";
	
	cout << "[Client]\tArguments: " << args[1] << ":" << args[2]  << endl;
	cout << "[Client]\tNumbers of Messages to Send: " << args[3] << endl;

	string host = args[1];
	int port = std::atoi(args[2]);
	int messages = std::atoi(args[3]);
	bool debug = true;
	if (argsc > 4)
	{
		int dInt = std::atoi(args[4]);

		if (dInt == 0)
			debug = false;
		
	}

	cout << "[Client] \tDebug status: " << debug << endl;

	Client* c = new Client(host,port);

	int result = c->openConnection();
	if (result == 0)
	{
		cout << "Client connected to: " << host << ":" << port << endl;
		int tx = 0, rx = 0;


		auto handler = std::async(keepRunning);

		t_begin = clock(); //Start Transmitions

		//for (int i = 0; i < messages; i++)
		int i = 0;
		while(bRunning)
		{
			std::stringstream out;
			int size = 0;
			out << i << "-:" << basicData << ":-" << i;
			size = out.str().size();

			if(debug)
				cout << "TX:\t[" << out.str() << "]" << endl;

			bool result = c->sendData((char*)out.str().c_str(), size);

			if (result == true)
			{
				tx++;

				char* rxData = c->receiveData();

				if (rxData != NULL)
				{
					rx++;
					
					if (debug)
						cout << "RX:\t[" << rxData << "]" << endl;

					delete[] rxData;
				}
			}
			else
				break;

			//Sleep(10);

		} //while


		t_end = clock(); //End Transmitions

		double timetask = double(t_end - t_begin) / CLOCKS_PER_SEC;

		stringstream out;
		
		t_Pend = clock(); //Start Transmitions
		double timeProgram = double(t_Pend - t_Pbegin) / CLOCKS_PER_SEC;

		out << "Tx = " << tx << "; Rx = " << rx << " Of " << messages << " Messages. Time (Seconds): " << timetask << " Total Progrma Time: " << timeProgram;
		writeResult(out.str());

		c->closeConnection();
	} // if Open.
	else
	{
		stringstream out;
		out << "Connection Error: " << result;
		writeResult(out.str());
	}
	delete c;
	
	//if(debug)
	//	system("PAUSE");


	cout << "Client Demo has ended." << endl;
	

	return 0;
}