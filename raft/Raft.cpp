#include "UniqueServer.h"
#include "ArgumentsParser.h"
#include <vector>

using std::vector;


int main(int argc, char const *argv[])
{
	UniqueServer rank0("127.0.0.1", 16510);
	UniqueServer rank1("127.0.0.1", 16520);
	UniqueServer rank2("127.0.0.1", 16530);
	UniqueServer rank3("127.0.0.1", 16540);
	UniqueServer rank4("127.0.0.1", 16550);

	vector<UniqueServer> rankTable;
	rankTable.push_back(rank0);
	rankTable.push_back(rank1);
	rankTable.push_back(rank2);
	rankTable.push_back(rank3);
	rankTable.push_back(rank4);
	
	int rank = ArgumentsParser::getRank(argc, argv);




	return 0;
}

