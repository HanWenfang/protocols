#include "ComputeCore.h"

// make sure that all servers start!
int init()
{
	int sock;
	struct timeval time_value;
	time_value.tv_sec = 1;
	time_value.tv_usec = 0;

	for(int i=0; i<5; ++i)
	{
		if(i == rank) continue;
		
		cout << rank <<" is trying to connect rank " << i << endl;

		while( Connect::connectTimeout(ranks[i].ip, ranks[i].port, sock,timeval) < 0){
			sleep(5);
		}

		close(sock);
	}

	if(asyncore.initialize(rank, ranks) < 0)
	{
		cout << "initialize asyncore error." << endl;
		return;
	}

	return 0;
}

void ComputeCore::run()
{
	init();

	//begin to select - timeout-------300ms
	for(;;)  
	{
		asyncore.select();

	}
}



