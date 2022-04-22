#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main(){


	pid_t pid;
	pid = fork();
		
	if(pid < 0){
		fprintf(stderr, "Fork Failed\n");
	}else if(pid == 0){
		exit(0);
	}else{
		sleep(100);
	}

	return 0;
}
