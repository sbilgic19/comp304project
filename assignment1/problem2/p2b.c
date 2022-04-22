#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
int main (int argc, char *argv[]){
	

	int n = atoi(argv[1]);
	int carNum = 1;
	while (n >= carNum){
		printf("Starting washing car %d\n", carNum);
	
		int fd4[2];		
		pipe(fd4);
		
		int fd[2];
		pipe(fd);
		write(fd[1], &carNum, sizeof(int));
		close(fd[1]);
		pid_t pid = fork();
		if(pid == 0){
			sleep(4);
			int a;
			read(fd[0], &a, sizeof(int));
			printf("Washing the windows of car %d\n", a);
			close(fd[0]);
		
			int fd2[2];
			pipe(fd2);
		
			write(fd2[1], &a, sizeof(int));
			close(fd2[1]);
			int id = fork();
			if(id == 0){
				sleep(4);
				int b;
				read(fd2[0], &b, sizeof(int));
				printf("Washing the interior of car %d\n", b);
				close(fd2[0]);
			
				int fd3[2];
				pipe(fd3);
				write(fd3[1], &b, sizeof(int));
				close(fd3[1]);
				int PID = fork();
				if(PID == 0){
					sleep(4);
					int x;
					read(fd3[0], &x, sizeof(int));
					printf("Washing the wheel of car %d\n", x);
					close(fd3[0]);
				
					write(fd4[1], &x, sizeof(int));
					close(fd4[1]);
					
					sleep(4);
					kill(getpid(), SIGKILL);
				}else{
					kill(getpid(), SIGKILL);
				}
			}else{
				kill(getpid(), SIGKILL);
			}
		

		}else{
			wait(NULL);
			int p;
			read(fd4[0], &p, sizeof(int));
			sleep(4);
			printf("Finished washing car: %d\n", p);
			close(fd4[0]);
			sleep(1);
			carNum +=1;
			//write(fd[1], &carNum, sizeof(int));
			//close(fd[1]);
		}	


	}








	
	return 0;
}
