
#include "AsynCore.h"
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#define IP_PORT_SIZE sizeof(struct sockaddr_in)

// AF_INET(IPv4 Address Format) SOCK_STREAM(TCP) IPPROTO_TCP(TCP)
int AsynCore::spawnSocket()
{
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	current_socket = fd;
	return fd;
}

int AsynCore::configSocket(int option)
{
	int on = 1;
	return setsockopt(current_socket, SOL_SOCKET, option, &on, sizeof(on));
}

void AsynCore::socketAddress(struct sockaddr_in &sockAddr, in_port_t serverPort, in_addr_t serverIp)
{
	sockAddr.sin_family = AF_INET; //IPv4
	sockAddr.sin_port = serverPort;
	sockAddr.sin_addr.s_addr = serverIp;
}

int AsynCore::socketRankBind(struct sockaddr_in *socket_addr)
{
	return bind(current_socket, (struct sockaddr *)socket_addr, sizeof(*socket_addr));
}

int AsynCore::setListen(int num)
{
	return listen(current_socket, num);
}

int getRank(sockaddr_in temp)
{
	int port = temp.sin_port;
	string ip = temp..sin_addr.s_addr;

	UniqueServer temp(ip, port);

	for(int i=0; i<5; ++i)
	{
		if(ranks[i] == temp) return i;
	}
}

void AsynCore::acceptSocket()
{
	int return_value;
	struct sockaddr_in temp;
	socklen_t add_size = IP_PORT_SIZE;

	for(;;)
	{
		if((return_value = accept(current_socket, (struct sockaddr *)&temp, &add_size)) < 0)
		{
			if(errno == EINTR) continue;
			else break;
		}
		else break;
	}

	int rank = getRank(temp);
	
	if (socks[rank] != -1)
	{
		close(return_value);
	}
	else
	{
		socks[rank] = return_value;
	}

	//int nSndBuf = 1024*32;
	//int nRcvBuf = 1024*32;
	int noTCPDelay = 1;

	//set socket options
	setsockopt(return_value, IPPROTO_TCP, TCP_NODELAY, (const void *)&noTCPDelay, sizeof(noTCPDelay));
	//setsockopt(return_value, SOL_SOCKET, SO_SNDBUF, (const void *)&nSndBuf, sizeof(nSndBuf));
	//setsockopt(return_value, SOL_SOCKET, SO_SNDBUF, (const void *)&nRcvBuf, sizeof(nRcvBuf));
}

void AsynCore::setRanks(int rk, UniqueServer *rank_set)
{
	rank = rk;
	ranks = rank_set;
}

int AsynCore::initialize(int rk, UniqueServer *rank_set)
{
	/*****************
	 Status:
		 follower:	0
		 candidate: 1
		 leader:	2 
	******************/
	status = 0;
	current_term = 0;
	current_index = 0;
	last_term = 0;
	last_committed = 0;
	startHeartbeatThread();

	setRanks(rk, rank_set);

	if(spawnSocket() < 0){
		cout << "spawn socket error" << endl;
		return -1;
	}

	socks[rank] = current_socket;
	
	if(configSocket(SO_REUSEADDR) < 0){
		cout << "config socket error" << endl;
		return -1;
	}

	int nSndBuf = 1024*32;
	int nRcvBuf = 1024*32;
	int noTCPDelay = 1;

	//set socket options
	setsockopt(current_socket, IPPROTO_TCP, TCP_NODELAY, (const void *)&noTCPDelay, sizeof(noTCPDelay));
	setsockopt(current_socket, SOL_SOCKET, SO_SNDBUF, (const void *)&nSndBuf, sizeof(nSndBuf));
	setsockopt(current_socket, SOL_SOCKET, SO_SNDBUF, (const void *)&nRcvBuf, sizeof(nRcvBuf));
	
	struct sockaddr_in ServerAddr;
	memset(&ServerAddr, 0, sizeof(ServerAddr));
	socketAddress(ServerAddr, htons(ranks[rank].port), htonl(INADDR_ANY));

	if(socketRankBind(&ServerAddr) < 0){
		cout << "socket bind error" << endl;
		return -1;
	}
	if(setListen(511) < 0){
		cout << "set socket listen error" << endl;
		return -1;
	}

	return 0;
}

int maxSock()
{
	int max_fd = -1;
	for(int i=0; i<5; ++i)
	{
		if(socks[i] > max_fd) max_fd = socks[i];
	}

	return max_fd;
}

void fdSetAll(fd_set *fds)
{
	for(int i=0; i<5; ++i)
	{
		if(socks[i] != -1)  FD_SET(socks[i], fds);
	}
}

void processHeartBeatMessage(Message &m)
{

}

void processVoteMessage(Message &m)
{
	incTerm();

}

void processVoteMessageOK(Message &m)
{
	

}

void processAppendEntryMessage(Message &m)
{
	

}

void processAppendEntryMessageOK(Message &m)
{
	

}

void processMessage(vector<Message> &inbox)
{
	for(vector<Message>::iterator it=inbox.begin(); it != inbox.end(); ++it)
	{
		switch (it->getMessageTag())
		{
			case RAFT_HEARTBEAT_MSSAGE:
				processHeartBeatMessage(*it);
				break;
			case RAFT_VOTE_MESSAGE:
				processVoteMessage(*it);
				break;
			case RAFT_VOTE_MESSAGE_OK:
				processVoteMessageOK(*it);
				break;
			case RAFT_APPEND_ENTRY_MESSAGE:
				processAppendEntryMessage(*it);
				break;
			case RAFT_APPEND_ENTRY_MESSAGE_OK:
				processAppendEntryMessageOK(*it);
				break;
			default:
				cout <<"Unknown message." << endl;
		}

	}
	inbox.clear();
}

int AsynCore::select()
{
	fd_set fds;
	struct timeval time_value;
	
	FD_ZERO(&fds);
	fdSetAll(&fds);

	//timeout every 300 ms
	time_value.tv_sec = 0;
	time_value.tv_usec = 300000;

	int max_sock = maxSock();

	if(::select(max_sock+1, &fds, NULL, NULL, &time_value) > 0)
	{
		for(int i=0; i<5; ++i)
		{
			if (sock[i] == -1) continue;

			if (FD_ISSET(sock[i], &fds))
			{
				if(i != rank)
				{
					//inbox --- receive one message
					if (Protocol::receiveMessage(sock[i], inbox) == 0)
					{
						close(sock[i]);
						sock[i] = -1;
					}
				}
				else
				{
					acceptSocket();
				}
			}
		}
	}

	// processing --- status machine
	if(status == 0){
		cout << "I am a flllower!" << endl;
		// deal with communication [ leader(heartbeat), brother(VoteRPC) ]
		processMessage(inbox);

		//check leader heartbeat
		// heartbeat is not OK status ==> candidate
		if (checkHeartbeat() < 0) updateStatus(0,1);
	}

	if(status == 1){
		cout << "I am a candidate!" << endl;
		// deal with communication [ leader(heartbeat), brother(VoteRPC)]
		processMessage(inbox);

		if((status == 1) && (checkLastVoteTime() < 0)){
			Poco::LocalDateTime now;
			lastVoteTime = now;
			Poco::Thread thread;
			// pull -- new thread to elect,status must be thread-safe! [ random timeout ]
			VoteRPC vRPC;
			thread.start(vRPC);
			thread.wait();
		}
	}

	if(status == 2){
		cout << "I am a leader!" << endl;
		// deal with communication [ leader(heartbeat), brother(VoteRPC)]
		processMessage(inbox);
		// start heartbeat thread -- heatbeat timer -----2s

		//deal with client request

		//enqueue...
			
	}

	//outbox --- send

}

void AsynCore::startHeartbeatThread()
{
	Timer timer(0, 10000); //10 seconds
	HeartBeat hb(this);
	timer.start(TimerCallback<HeartBeat>(hb, &HeartBeat::run));
	cout << "Heartbeat Timer started!" << endl;
}

VoteRPC()
{
	while(select-timeout)
	{

		// status check
		!candidate => break;

		// getall response
		 ==>leader
		 break;
	}
}



