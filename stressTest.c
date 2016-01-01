#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
int main(int argc, char* argv[])
{
	//There will be 2^counter threads (all repeatedly asking for simultaneous connections).
	int counter;
	for(counter = 0; counter < 5; counter++)
	{
		fork();
	}
	while(1)
	{
		int returned = system("cat request.txt | nc 127.0.1.1 8000");
		if(WIFSIGNALED(returned))
			if(WTERMSIG(returned) == SIGINT || WTERMSIG(returned) == SIGQUIT)
				break;
	}
}
