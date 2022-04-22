#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
int main(int argc, char* argv[]){

	int pid;
	int level = 0;
	printf("getppid: %d, Main Process ID: %d, level: %d\n", getppid(),getpid(), level);
	int i;
	int id;
	//level += 1;
	
	/*
	for(i=0; i<3; i++){
				
		//pid = fork();
		if(level > 0){
			printf("Process ID: %d, Parent ID:%d, level:%d\n", getpid(), getppid(), level);
			
			//level += 1;
		}else{
			pid = fork();
			printf("Process ID: %d, Parent ID: %d, level: %d\n", pid, getppid(), level);
			level += 1;
			//printf("Process ID: %d, Parent ID: %d, level: %d\n", pid, getppid(), level);
		}
	}		
	//printf("Process ID: %d, Parent ID:%d, level: %d\n", getpid(), getppid(),level);
	*/
	level += 1;
	int n = atoi(argv[1]);
	for(i=0; i<n; i++){
		id = fork();
		if(id == 0){
			//level += 1;
										
			printf("Process ID:%d, Parent ID: %d, level: %d\n", getpid(), getppid(), level);
			level += 1;

		}else{
			wait(NULL);
			//level += 1;

		}
		


		
	}


		
}


