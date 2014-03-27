
#ifndef ENTRY_RAFT_H
#define ENTRY_RAFT_H
#include <string>

using namespace std;

class Entry
{
public:
	Entry(int ct, int cc, int rc, string cm):current_term(ct), current_committed(cc), resp_counter(rc), command(cm)
	{}
	~Entry();
	string toJSON();
	int getCurrentTerm();
	int getCurrentCommitted();
	int getCommand();

private:
	int current_term;
	int current_committed;
	int resp_counter;
	string command;
};



#endif
