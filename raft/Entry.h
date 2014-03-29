
#ifndef ENTRY_RAFT_H
#define ENTRY_RAFT_H
#include <string>
#include <Poco/NumberFormatter.h>

using namespace std;

class Entry
{
public:
	Entry(int ct, int cc, int ci, string cm):current_term(ct), current_committed(cc), \
	resp_counter(1), current_index(ci), command(cm)
	{}
	~Entry();
	string toJSON();
	string combineToJSON();

	int getCurrentTerm();
	int getCurrentCommitted();
	int getCurrentIndex();
	int getCommand();

private:
	int current_term;
	int current_committed;
	int current_index;
	int resp_counter;
	string command;
};



#endif
