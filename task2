#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>

void CurrTime();

int main()
{
	pid_t child_pid;
	
	printf("Parent: PID %d Parent PID %d ", getpid(), getppid());
	CurrTime();
	
	for(int i = 1; i <= 2; i++)
	{
		child_pid = fork();
		
		switch(child_pid)
		{
			case 0:
				printf("Child %d: PID %d Parent PID %d ", i, getpid(), getppid());
				CurrTime();
				exit(EXIT_SUCCESS);
				
			case -1: 
				fprintf(stderr, "An error occurred, when creating the child process\n");
				exit(EXIT_FAILURE);
			
			default:
				break;		
		}
	}
			 
	if(system("ps -x") == -1)
		perror("system");
		
	while(wait(NULL) != -1);	
	
	
	return 0;
}

void CurrTime()
{
	struct timeval time;
	
	if(gettimeofday(&time, NULL) == -1)
	{
		fprintf(stderr, "An error occurred, when calculating the current time\n");
	}
	else
	{
		int msec = time.tv_usec / 1000;
		int sec = time.tv_sec % 60;
		int min = (time.tv_sec / 60) % 60;
		int hours = (time.tv_sec / 3600 + 3) % 24;
		
		printf("Time is: %02d:%02d:%02d:%03d\n\n", hours, min, sec, msec);
	}
}
