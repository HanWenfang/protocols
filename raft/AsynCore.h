

#ifndef ASYNCORE_RAFT_H
#define ASYNCORE_RAFT_H

#include "UniqueServer.h"
#include "Protocol.h"
#include "Entry.h"
#include "MessageTypes.h"
#include "Connect.h"
#include <vector>
#include <arpa/inet.h>
#include <iostream>
#include <map>
#include <string.h>
#include <Poco/Thread.h>
#include <Poco/ThreadPool.h>
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
		if(diff.totalSeconds() > 30){
			lastVoteTime = now;
			return -1;
		}

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
	int last_index;

	int current_term;
	int last_term;

	int last_committed;
	int current_committed;

	int committed;
	int voters;

	void incTerm() { ++ current_term; }

	struct sockTable
	{
		int index; // for entry log
		int sock;
	};

	sockTable sock_table[5];

	VoteRPC vRPC;

public:
	AsynCore(){
		for(int i=0; i<5; ++i)
		{
			sock_table[i].sock = -1;
			sock_table[i].index = 0;
		}
	}
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

	sockTable *getSockTable()
	{
		return sock_table;
	}

	vector<Entry> &getEntryLog()
	{
		return entryLog;
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
			vector<Entry> *entryLog = async->getEntryLog();

			vector<Message> outbox; // cleared

			int current_term = async->getCurrentTerm();
			int current_committed = async->getCurrentCommitted();

			for(int i=0; i<5; ++i)
			{
				if(i == rank) continue;
				if(socks[i] != -1){
					//HeartBeat Message
					Entry entry(current_term, current_committed, current_index, "");
					Message m(rank, i, RAFT_HEARTBEAT_MSSAGE, entry.toJSON());
					m.setSocket(socks[i]);
					outbox.push_back(m);
				}
				else
				{
					// the client didnot connect!
					// I am the leader!
					int sock;
					struct timeval time_value;
					time_value.tv_sec = 0;
					time_value.tv_usec = 300000; //300ms
					if(Connect::connectTimeout(ranks[i].ip, ranks[i].port, sock,timeval) > 0)
					{
						Entry entry(current_term, current_committed, current_index, "");
						Message m(rank, i, RAFT_HEARTBEAT_MSSAGE, entry.toJSON());
						m.setSocket(sock);
						socks[i].sock = sock; //should be thread-safe
						outbox.push_back(m);
					}

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
			if(current_committed == current_index) return;

			int *socks = async->getSocks();
			
			vector<Message> outbox; // cleared

			for(int i=0; i<5; ++i)
			{
				if(i == rank) continue;
				if(socks[i] != -1){
					//AppendEntryLog Message
					string context = combineToJson(entryLog[current_committed], entryLog[current_committed+1]);
					Message m(rank, i, RAFT_APPEND_ENTRY_MESSAGE, context);
					m.setSocket(socks[i]);
					outbox.push_back(m);
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
		// increase current term
		async->incTerm();

		sockTable *sock_table = async->getSockTable();
		vector<Message> outbox; // cleared
		vector<Message> offlineBox;

		for(int i=0; i<5; ++i)
		{
			if(i == rank) continue;
			if(sock_table[i].sock != -1){
				//Vote Message
				Entry entry(current_term, current_committed, current_index, "");
				string context = entry.combineToJSON(entryLog[current_committed]);
				Message m(rank, i, RAFT_VOTE_MESSAGE, context);
				m.setSocket(socks[i]);

				outbox.push_back(m);
			}
			else
			{
				// the client didnot connect!
				// I am the candidate!
				int sock;
				struct timeval time_value;
				time_value.tv_sec = 0;
				time_value.tv_usec = 300000; //300ms
				if(Connect::connectTimeout(ranks[i].ip, ranks[i].port, sock,timeval) > 0)
				{
					Entry entry(current_term, current_committed, current_index, "");
					string context = entry.combineToJSON(entryLog[current_committed]);
					Message m(rank, i, RAFT_VOTE_MESSAGE, context);
					m.setSocket(sock);
					socks[i].sock = sock; // should be thread-safe!
					outbox.push_back(m);
				}

			}
		}

		// asynchronous thread
		Protocol::sendMessage(outbox);
	}

private:
	AsynCore *async;

};


class CommitChecker : public Runnable
{
public:
	CommitChecker(AsynCore *ac, int c):async(ac),committed(c) {}
	virtual void run()
	{
		if(async->getStatus() == 2){
			vector<Entry> entry = async->getEntryLog();

			// strong consensus
			if(entry[committed+1].resp_counter ==5){
				// commited operation

				++commited;
			}
		}
	}

private:
	AsynCore *async;
	int committed;
};

#endif


