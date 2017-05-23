# ServerHighPerformance for Windows (Visual Studio 2015)

**ServerHighPerformance** is a **C++11** sample sample which shows a way to create a ServerSocket application, in C++11, to attend up to 2000 remote clients connected concurrent.

This sample has:

+ 1 *VS-2015* So C++ slution named **ServerHighPerformance.sln**
+ 2 *VS-2015* C++ Proyects:
  + **ServerHighPerformance**, which has the code of the ServerHighPerformance.
  + **RemoteClient**, which has the code of the remote client.

## How it works

The main consideration of **ServerHighPerformance** is avoid the using of thread and sleeps to attend each remote connection. Instead of **ServerHighPerformance** use a single thread to monitoring each connection request and the reading status of each remote connection's socket. To monitoring the **ServerHighPerformance** use File Descriptors and `select` function, to wait until an event has happened.


``` C++
int Server::Run(int port)
{
	int err = 0;
	int max_sd = 0;
	fd_set rd_fd;
	fd_set ex_fd;
	queue<SOCKET> qClosed;

  //...

	while (bKeepRunning)
	{
    //...
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

			//...

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
```

 Mean while the **RemoteClient** is connected to the **ServerHighPerformance** to send 1000 message. Each message that is sended by **RemoteClient** needs to be answered by the **ServerHighPerformance** before to send the next message in queue.

# Testing

The folder TestPack has the kit to test the server. Each Remote client will be connected until its 1000 message are sended and answered.

To run the Server use the sintaxis:

> ServerHighPerformance.exe *PORT* *DEBUG (1 = ON/0 = OFF)*

Also, you can use the cmd file `runServer.cmd`to run the server.

To run a RemoteClient use the sintaxis:

> RemoteClient.exe *Host* *Port* *Amount of messages* *DEBUG (1 = ON/0 = OFF)*

Also you can use the files  `runXClient.cmd` to run a X amount of remote clients, for example:

To run 5 remote client use the file `run5Client.cmd` to rise client that will send 1000 message. The output will be storage at the file `output_client.log`. This file will be registered the output of each RemoteClient, as this:

```batch
Tx = 1000; Rx = 1000 Of 1000 Messages. Time (Seconds): 0.173 Total Progrma Time: 0.213
Tx = 1000; Rx = 1000 Of 1000 Messages. Time (Seconds): 0.298 Total Progrma Time: 0.334
Tx = 1000; Rx = 1000 Of 1000 Messages. Time (Seconds): 0.425 Total Progrma Time: 0.528
Tx = 1000; Rx = 1000 Of 1000 Messages. Time (Seconds): 0.407 Total Progrma Time: 0.524
Tx = 1000; Rx = 1000 Of 1000 Messages. Time (Seconds): 0.369 Total Progrma Time: 0.529
```
+ Each line says:
+ How many message has been sended, received
+ How long it take to send all the messages.
+ How long takes the RemoteClient execute all the program, from the begine of the main to the end of the main.
