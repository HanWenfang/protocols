

#ifndef ASYNCORE_RAFT_H
#define ASYNCORE_RAFT_H

#include "UniqueServer.h"
#include "Protocol.h"
#include "Entry.h"
#include "MessageTypes.h"
#include <vector>
#include <arpa/inet.h>
#include <iostream>
#include <map>
#include <string.h>
#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include <Poco/LocalDateTime.h>
#include <Poco/Mutex.h>
#include <Poco/Timer.h>

using namespace std;
using Poco::Timer;

class AsynCore
{
private:
	int current_socket;

	int rank;
	UniqueServer *ranks;
	int socks[5];

	vector<Message> inbox;
	vector<Message> outbox;

	int status;
	Poco::Mutex mutex;
	void updateStatus(int olds, int news)
	{
		Poco::Mutex::ScopedLock lock(mutex);
		if(status != olds) return;
		status = news;
	}
	Poco::LocalDateTime lastVoteTime;
	int checkLastVoteTime()
	{
		Poco::LocalDateTime now;
		Timespan diff = now - lastVoteTime;
		
		//30 seconds votetime out-of-date:no random
		if(diff.totalSeconds() > 30) return -1;

		return 0;
	}

	Poco::LocalDateTime lastHeartbeat;
	int checkHeartbeat()
	{
		Poco::LocalDateTime now;
		Timespan diff = now - lastHeartbeat;
		
		//60 seconds heartbeat out-of-date
		if(diff.totalSeconds() > 60) return -1;

		return 0;
	}
	void startHeartbeatThread();
	void processMessage(vector<Message> &inbox);
	void processVoteMessage(Message &m);
	void processVoteMessageOK(Message &m);
	void processAppendEntryMessage(Message &m);
	void processAppendEntryMessageOK(Message &m);
	void processHeartBeatMessage(Message &m);
	
	vector<Entry> entryLog;
	int current_index;
	int current_term;
	int last_term;
	int last_committed;

	void incTerm() { ++ current_term; }


public:
	AsynCore(){ memset(socks, -1, sizeof(socks)); }
	int initialize(int rk, UniqueServer *rank_set);
	void setRanks(int rk, UniqueServer *rank_set);
	int spawnSocket();
	int configSocket(int option); // redis
	void socketAddress(struct sockaddr_in &sockAddr, in_port_t serverPort, in_addr_t serverIp);
	int socketRankBind(struct sockaddr_in *socket_addr);
	int setListen(int num);
	void acceptSocket();
	int select();

	int getStatus()
	{
		Poco::Mutex::ScopedLock lock(mutex);
		return status;
	}

	int *getSocks()
	{
		return socks;
	}
};

class HeartBeat
{
public:
	HeartBeat(AsynCore *ac):async(ac) {}
	void run(Timer &timer)
	{
		if(async->getStatus() == 2){
			int *socks = async->getSocks();
			vector<Message> outbox; // cleared

			for(int i=0; i<5; ++i)
			{
				if(i == rank) continue;
				if(socks[i] != -1){
					//HeartBeat Message

					
				}
			}

		
			Protocol::sendMessage(outbox);
		}

	}

private:
	AsynCore *async;

};

class AppendEntry: public Poco::Runnable
{
public:
	AppendEntry(AsynCore *ac):async(ac) {}
	virtual void run()
	{
		if(async->getStatus() == 2){
			int *socks = async->getSocks();
			vector<Message> outbox; // cleared

			for(int i=0; i<5; ++i)
			{
				if(i == rank) continue;
				if(socks[i] != -1){
					//HeartBeat Message

					
				}
			}

		
			Protocol::sendMessage(outbox);
		}

	}

private:
	AsynCore *async;

};

// random timeout
class VoteRPC: public Poco::Runnable
{
public:
	VoteRPC(AsynCore *ac):async(ac) {}
	virtual void run()
	{
		async->incTerm();
		int *socks = async->getSocks();
		vector<Message> outbox; // cleared

		for(int i=0; i<5; ++i)
		{
			if(i == rank) continue;
			if(socks[i] != -1){
				//Vote Message
				Entry entry();
				Message m(rank, i, RAFT_VOTE_MESSAGE, entry.toJSON());
				m.setSocket(socks[i]);

				outbox.push_back(m);
			}
		}

		// asynchronous thread
		Protocol::sendMessage(outbox);
	}

private:
	AsynCore *async;

};

#endif


