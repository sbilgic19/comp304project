#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int main(int argc, char *argv[]){


	pid_t pid;
	pid = fork();
	
	if(pid < 0){
		fprintf(stderr , "Fork Failed\n");
		return 1;
	}else if(pid == 0){
		char str1[30] = "/bin/";
		strcat(str1, argv[1]);
		
		char str2[30] = "-";		
		if(argc > 2){
			strcat(str2, argv[2]);
			
			execlp(str1, str1, str2, NULL);
		}else{
			execlp(str1, str1, NULL);
		}
	}else{
		wait(NULL);
		printf("Child finished executing %s\n", argv[1]);
	}


	return 0;
}
