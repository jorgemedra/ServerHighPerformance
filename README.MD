# ServerHighPerformance for Windows (Visual Studio 2015)

**ServerHighPerformance** is a **C++11** sample sample which shows a way to create a ServerSocket application, in C++11, to attend up to 2000 remote clients connected concurrent.

This sample has:

+ 1 *VS-2015* So C++ slution named **ServerHighPerformance.sln**
+ 2 *VS-2015* C++ Proyects:
  + **ServerHighPerformance**, which has the code of the ServerHighPerformance.
  + **RemoteClient**, which has the code of the remote client.

## How it works

The main consideration of **ServerHighPerformance** is avoid the using of thread and sleeps to attend each remote connection. Instead of **ServerHighPerformance** use a single thread to monitoring each connection request and the reading status of each remote connection's socket. To monitoring the **ServerHighPerformance** use File Descriptors and `select` function, to wait until an event has happened.


```Matlab

% Initialize Socket Server server.
% Initialize skt_m map.
% Initialize qClosed queue.
% Add server to rd_fd.
% tout = X;

while Service is Running

	% Add all socket of server and skt_m to rd_fd. This step has a runtime of O(n).

	% Wait for an event of Request connection o Data ready for read.
	% k = select(dummy, &rd_fd, NULL, NULL, NULL, &tout);

	if k < 0
		% Find the sockets that trigger an error on select and add them to qClosed.
	elseif k == 0
		% Timeout detected, it’s depend of each requirement to know how to handle each time out it happens.
	else

		% For each socket s that raise an event on rd_fd to accept an Connection Request
		% or read data. This step has a runtime of O(k).
		for i=0 to k-1
			s = rd_fd(i);
			if s == server
				% Accept socket and add it to skt_m.
			else			
				% Read data from socket s.
				% If socket s trigger an error to read, then close the socket
				% adding it to qClosed.
			end
		end
	end

	while qClosed is not empty.
		sc = Pop from qClosed.
		% Close sc.
	end
end

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
