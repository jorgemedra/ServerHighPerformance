
# RESUME

Using `astd::async` for each time that socket client is ready:

```c++
 void Server::newDataAvailabe(SOCKET s)
{
	
	map<SOCKET, Client*>::iterator it = m_sockets.find(s);
	if (it != m_sockets.end())
	{
		Client* c = m_sockets[s];
		
		auto cbFunction = std::bind(&Client::newDataArrived, c);
		std::async(cbFunction);
		//c->newDataArrived(); //Process the incomming message and response it.
	}
}
```

**Running 5 clients:**

```batch
Tx = 1000; Rx = 1000 Of 1000 Messages. Time (Seconds): 2.137
Tx = 1000; Rx = 1000 Of 1000 Messages. Time (Seconds): 2.113
Tx = 1000; Rx = 1000 Of 1000 Messages. Time (Seconds): 2.146
Tx = 1000; Rx = 1000 Of 1000 Messages. Time (Seconds): 2.06
Tx = 1000; Rx = 1000 Of 1000 Messages. Time (Seconds): 1.94
```

