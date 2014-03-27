#include "UniqueServer.h"
#include "ArgumentsParser.h"
#include "ComputeCore.h"
#include <vector>

using std::vector;

int main(int argc, char const *argv[])
{
	UniqueServer rank0("127.0.0.1", 16510);
	UniqueServer rank1("127.0.0.1", 16520);
	UniqueServer rank2("127.0.0.1", 16530);
	UniqueServer rank3("127.0.0.1", 16540);
	UniqueServer rank4("127.0.0.1", 16550);

	UniqueServer rankTable[5];
	rankTable[0] = rank0;
	rankTable[1] = rank1;
	rankTable[2] = rank2;
	rankTable[3] = rank3;
	rankTable[4] = rank4;
	
	int rank = ArgumentsParser::getRank(argc, argv);

	ComputeCore cc(rankTable, rank);
	cc.run();

	return 0;
}

