#include "ComputeCore.h"

void startThread()
{
	

}

// make sure that all servers start!
int init()
{
	int sock;
	struct timeval time_value;
	time_value.tv_sec = 1;
	time_value.tv_usec = 0;

	for(vector<UniqueServerQueue>::iterator it=ranks.begin(); it != ranks.end(); ++it)
	{
		if((it- ranks.begin()) == rank) continue;
		
		cout << rank <<" is trying to connect rank " << it- ranks.begin() << endl;

		while( Connect::connectTimeout(it->ip, it->port, sock,timeval) < 0){
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



