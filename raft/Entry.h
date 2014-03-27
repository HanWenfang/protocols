
#ifndef ENTRY_RAFT_H
#define ENTRY_RAFT_H
#include <string>

using namespace std;

class Entry
{
public:
	Entry();
	~Entry();
	string toJSON();

private:
	int current_term;
	int current_index;
	string command;
};



#endif
