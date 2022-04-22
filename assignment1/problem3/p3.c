#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <sys/wait.h>
int main(int argc, char *argv[]){


	pid_t pid;
	int fd[2];
	pipe(fd);
	pid = fork();
	
	
	if(pid < 0){
		fprintf(stderr , "Fork Failed\n");
		return 1;
	}else if(pid == 0){
		struct timeval t1;
		gettimeofday(&t1, NULL);
		write(fd[1], &t1, sizeof(t1));
		close(fd[1]);

		char str1[30];
		strcpy(str1,argv[1]);
		//printf("str: %s\n", str1);
		char str2[30] = "-";		
		if(argc > 2){
			strcat(str2, argv[2]);
			execlp(str1, str1, str2, NULL);
		}else{
			//printf("Else e girdi\n");
			execlp(str1, str1, NULL);
			//printf("Done\n");
		}
	}else{
		wait(NULL);
		struct timeval t2;
		gettimeofday(&t2, NULL);

		struct timeval tChild;
		//printf("Checkpoint\n");
		read(fd[0], &tChild, sizeof(tChild));
		//printf("Checkpoint 2");
		close(fd[0]);
		//printf("Child finished executing %s\n", argv[1]);
		long long elapsed = (t2.tv_sec-tChild.tv_sec)*1000000LL + t2.tv_usec-tChild.tv_usec;
		printf("Time it takes for child to complete is %llu ms\n", elapsed);
	}


	return 0;
}
