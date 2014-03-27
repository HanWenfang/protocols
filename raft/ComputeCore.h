
#ifndef COMUPTECORE_RAFT_H
#define COMUPTECORE_RAFT_H
#include "UniqueServerQueue.h"
#include "Connect.h"
#include "AsynCore.h"
#include "Message.h"

using namespace std;

class ComputeCore
{
private:
	UniqueServer *ranks;
	int rank;
	
	AsynCore asyncore;
	
public:
	ComputeCore(UniqueServer *rank_set, int rk):ranks(rank_set), rank(rk) { }
	~ComputeCore(){ }

	void run();
	void init();

	int getRank(){
		return rank;
	}
};


#endif

