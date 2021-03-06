/* Serkan Berk Bilgiç - 71571
 * Ali Oktay - 72007
 */


#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h> //termios, TCSANOW, ECHO, ICANON
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <math.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
const char *sysname = "shellfyre";
// for cdh command
char arr[10][100];
int directory_counter = 9;
int arr_size = 0;

int ppid = 10;

#define MODULE "my_module"
// for filesearch command
char *directories[100];
int p = 0;
enum return_codes
{
	SUCCESS = 0,
	EXIT = 1,
	UNKNOWN = 2,
};

struct command_t
{
	char *name;
	bool background;
	bool auto_complete;
	int arg_count;
	char **args;
	char *redirects[3];		// in/out redirection
	struct command_t *next; // for piping
};

/**
 * Prints a command struct
 * @param struct command_t *
 */
void print_command(struct command_t *command)
{
	int i = 0;
	printf("Command: <%s>\n", command->name);
	printf("\tIs Background: %s\n", command->background ? "yes" : "no");
	printf("\tNeeds Auto-complete: %s\n", command->auto_complete ? "yes" : "no");
	printf("\tRedirects:\n");
	for (i = 0; i < 3; i++)
		printf("\t\t%d: %s\n", i, command->redirects[i] ? command->redirects[i] : "N/A");
	printf("\tArguments (%d):\n", command->arg_count);
	for (i = 0; i < command->arg_count; ++i)
		printf("\t\tArg %d: %s\n", i, command->args[i]);
	if (command->next)
	{
		printf("\tPiped to:\n");
		print_command(command->next);
	}
}

/**
 * Release allocated memory of a command
 * @param  command [description]
 * @return         [description]
 */
int free_command(struct command_t *command)
{
	if (command->arg_count)
	{
		for (int i = 0; i < command->arg_count; ++i)
			free(command->args[i]);
		free(command->args);
	}
	for (int i = 0; i < 3; ++i)
		if (command->redirects[i])
			free(command->redirects[i]);
	if (command->next)
	{
		free_command(command->next);
		command->next = NULL;
	}
	free(command->name);
	free(command);
	return 0;
}

/**
 * Show the command prompt
 * @return [description]
 */
int show_prompt()
{
	char cwd[1024], hostname[1024];
	gethostname(hostname, sizeof(hostname));
	getcwd(cwd, sizeof(cwd));
	printf("%s@%s:%s %s$ ", getenv("USER"), hostname, cwd, sysname);
	return 0;
}

	/**
 * Parse a command string into a command struct
 * @param  buf     [description]
 * @param  command [description]
 * @return         0
 */
int parse_command(char *buf, struct command_t *command)
{
	const char *splitters = " \t"; // split at whitespace
	int index, len;
	len = strlen(buf);
	while (len > 0 && strchr(splitters, buf[0]) != NULL) // trim left whitespace
	{
		buf++;
		len--;
	}
	while (len > 0 && strchr(splitters, buf[len - 1]) != NULL)
		buf[--len] = 0; // trim right whitespace

	if (len > 0 && buf[len - 1] == '?') // auto-complete
		command->auto_complete = true;
	if (len > 0 && buf[len - 1] == '&') // background
		command->background = true;

	char *pch = strtok(buf, splitters);
	command->name = (char *)malloc(strlen(pch) + 1);
	if (pch == NULL)
		command->name[0] = 0;
	else
		strcpy(command->name, pch);

	command->args = (char **)malloc(sizeof(char *));

	int redirect_index;
	int arg_index = 0;
	char temp_buf[1024], *arg;

	while (1)
	{
		// tokenize input on splitters
		pch = strtok(NULL, splitters);
		if (!pch)
			break;
		arg = temp_buf;
		strcpy(arg, pch);
		len = strlen(arg);

		if (len == 0)
			continue;										 // empty arg, go for next
		while (len > 0 && strchr(splitters, arg[0]) != NULL) // trim left whitespace
		{
			arg++;
			len--;
		}
		while (len > 0 && strchr(splitters, arg[len - 1]) != NULL)
			arg[--len] = 0; // trim right whitespace
		if (len == 0)
			continue; // empty arg, go for next

		// piping to another command
		if (strcmp(arg, "|") == 0)
		{
			struct command_t *c = malloc(sizeof(struct command_t));
			int l = strlen(pch);
			pch[l] = splitters[0]; // restore strtok termination
			index = 1;
			while (pch[index] == ' ' || pch[index] == '\t')
				index++; // skip whitespaces

			parse_command(pch + index, c);
			pch[l] = 0; // put back strtok termination
			command->next = c;
			continue;
		}

		// background process
		if (strcmp(arg, "&") == 0)
			continue; // handled before

		// handle input redirection
		redirect_index = -1;
		if (arg[0] == '<')
			redirect_index = 0;
		if (arg[0] == '>')
		{
			if (len > 1 && arg[1] == '>')
			{
				redirect_index = 2;
				arg++;
				len--;
			}
			else
				redirect_index = 1;
		}
		if (redirect_index != -1)
		{
			command->redirects[redirect_index] = malloc(len);
			strcpy(command->redirects[redirect_index], arg + 1);
			continue;
		}

		// normal arguments
		if (len > 2 && ((arg[0] == '"' && arg[len - 1] == '"') || (arg[0] == '\'' && arg[len - 1] == '\''))) // quote wrapped arg
		{
			arg[--len] = 0;
			arg++;
		}
		command->args = (char **)realloc(command->args, sizeof(char *) * (arg_index + 1));
		command->args[arg_index] = (char *)malloc(len + 1);
		strcpy(command->args[arg_index++], arg);
	}
	command->arg_count = arg_index;
	return 0;
}

void prompt_backspace()
{
	putchar(8);	  // go back 1
	putchar(' '); // write empty over
	putchar(8);	  // go back 1 again
}

/**
 * Prompt a command from the user
 * @param  buf      [description]
 * @param  buf_size [description]
 * @return          [description]
 */
int prompt(struct command_t *command)
{
	int index = 0;
	char c;
	char buf[4096];
	static char oldbuf[4096];

	// tcgetattr gets the parameters of the current terminal
	// STDIN_FILENO will tell tcgetattr that it should write the settings
	// of stdin to oldt
	static struct termios backup_termios, new_termios;
	tcgetattr(STDIN_FILENO, &backup_termios);
	new_termios = backup_termios;
	// ICANON normally takes care that one line at a time will be processed
	// that means it will return if it sees a "\n" or an EOF or an EOL
	new_termios.c_lflag &= ~(ICANON | ECHO); // Also disable automatic echo. We manually echo each char.
	// Those new settings will be set to STDIN
	// TCSANOW tells tcsetattr to change attributes immediately.
	tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

	// FIXME: backspace is applied before printing chars
	show_prompt();
	int multicode_state = 0;
	buf[0] = 0;

	while (1)
	{
		c = getchar();
		// printf("Keycode: %u\n", c); // DEBUG: uncomment for debugging

		if (c == 9) // handle tab
		{
			buf[index++] = '?'; // autocomplete
			break;
		}

		if (c == 127) // handle backspace
		{
			if (index > 0)
			{
				prompt_backspace();
				index--;
			}
			continue;
		}
		if (c == 27 && multicode_state == 0) // handle multi-code keys
		{
			multicode_state = 1;
			continue;
		}
		if (c == 91 && multicode_state == 1)
		{
			multicode_state = 2;
			continue;
		}
		if (c == 65 && multicode_state == 2) // up arrow
		{
			int i;
			while (index > 0)
			{
				prompt_backspace();
				index--;
			}
			for (i = 0; oldbuf[i]; ++i)
			{
				putchar(oldbuf[i]);
				buf[i] = oldbuf[i];
			}
			index = i;
			continue;
		}
		else
			multicode_state = 0;

		putchar(c); // echo the character
		buf[index++] = c;
		if (index >= sizeof(buf) - 1)
			break;
		if (c == '\n') // enter key
			break;
		if (c == 4) // Ctrl+D
			return EXIT;
	}
	if (index > 0 && buf[index - 1] == '\n') // trim newline from the end
		index--;
	buf[index++] = 0; // null terminate string

	strcpy(oldbuf, buf);

	parse_command(buf, command);

	// print_command(command); // DEBUG: uncomment for debugging

	// restore the old settings
	tcsetattr(STDIN_FILENO, TCSANOW, &backup_termios);
	return SUCCESS;
}
int my_module(char *args[], int *ppid);
void print_cdh();
int cdh_command();
void addir_tracker(char *s);
int process_command(struct command_t *command);
void awesome();
void take(struct command_t *command);
char **fsearch(char *dirname, char *extension, char *filename, char *curdir);
char **nonrecursive_fsearch(char *dirname, char *extension, char *filename);
void executeToDoList(char *args[]){
        FILE *fp;
        fp = fopen("todo.txt","a");
        char *todoListArgs[] = {
                "/usr/bin/cat",
                "todo.txt",
                0

        };
        char str[150];
        char *todoDelArgs[5] = {
                "/usr/bin/sed",
                "-i",
                str,
                "todo.txt",
                0
        };

        pid_t pid = fork();
        if(pid < 0){
                printf("Fork failed");
        }
        else if(pid == 0){
                if(strcmp(args[0] , "-a") == 0) {
                        char str[100];
                        strcpy(str,args[1]);
                        strcat(str, "\n");
                        fprintf(fp,"%s", str);
                }
                else if(strcmp(args[0], "-l") == 0){
                        execv(todoListArgs[0],todoListArgs);
                        exit(0);
                }else if(strcmp(args[0], "-d") == 0){
                        strcpy(str, args[1]);
                        char str2[150];
                        strcpy(str2, "/");
                        strcat(str2, str);
                        strcat(str2, "/d");
                        strcpy(todoDelArgs[2],str2);
                        execv(todoDelArgs[0],todoDelArgs);
                        exit(0);

                }else{
                        printf("You should choose what you want to do");
                }

        }
        else
                wait(NULL);

        fclose(fp);

}


int main()
{
	char buf[100];
	getcwd(buf, 100);
	//addir_tracker(buf);
	while (1)
	{
		struct command_t *command = malloc(sizeof(struct command_t));
		memset(command, 0, sizeof(struct command_t)); // set all bytes to 0

		int code;
		code = prompt(command);
		if (code == EXIT)
			break;

		code = process_command(command);
		if (code == EXIT)
			break;

		free_command(command);
	}

	printf("\n");
	return 0;
}

int process_command(struct command_t *command)
{
	int r;
	if (strcmp(command->name, "") == 0)
		return SUCCESS;

	if (strcmp(command->name, "exit") == 0)
		return EXIT;

	if (strcmp(command->name, "cd") == 0)
	{
		if (command->arg_count > 0)
		{
			char b[100];
			getcwd(b,100);
			addir_tracker(b);
			r = chdir(command->args[0]);
			if (r == -1)
				printf("-%s: %s: %s\n", sysname, command->name, strerror(errno));
			return SUCCESS;
		}
	}
	if(strcmp(command->name, "awesome1") == 0){
		if(fork() == 0){
			awesome();
			exit(0);
		}else{
			if(!command->background){
				wait(0);
			}
			return SUCCESS;
		}
		return SUCCESS;
	}
	if(strcmp(command->name, "filesearch")==0){
		if(fork() == 0){
			char *s = strstr(command->args[0],"r");
			if(s != NULL){
				char curdir[250];
				getcwd(curdir,250);
				fsearch(".", command->args[0], command->args[1],curdir);
			}else{
				nonrecursive_fsearch(".", command->args[0], command->args[1]);
			}
		}else{
			if(!command->background){
				wait(NULL);
			}
			return SUCCESS;
		}
		return SUCCESS;
	}	
	
	if(strcmp(command->name, "take") == 0){
		if(fork() == 0){
			take(command);
		}else{
			if(!command->background){
				wait(0);
			}
			return SUCCESS;
		}
		return SUCCESS;
	}
	if(strcmp(command->name, "pstraverse")==0){
		/*
		char *a[] = {"make", NULL};
		int processID = 0;
		char *b[] = {"/usr/bin/sudo","insmod", "my_module.ko", processID, 0};
		char *d[] = {"/usr/bin/sudo", "./my_module.o",command->args[0], NULL};
		if(fork() == 0){

			execv("/usr/bin/make",a);
			if(fork() == 0){
				execv("/usr/bin/sudo",b);
				printf("It is done successfully.\n");
				if(fork() == 0){
					execv("/usr/bin/sudo", d);
				}else{
					wait(NULL);
				}
			}else{
				wait(NULL);
			}

		}else{
			wait(NULL);
			char *c[] = {"sudo", "rmmod", "my_module.ko",NULL};
			execv("/usr/bin/sudo", c);
				
		}
		return SUCCESS;	
		*/
		my_module(command->args, &ppid);
		return SUCCESS;
	}
	if(strcmp(command->name, "cdh")==0){
		
		
		if(fork() == 0){
			cdh_command();
		}else{
			if(!command->background){
				wait(NULL);
			}
			return SUCCESS;
		}
		return SUCCESS;
		/*
		cdh_command();
		return SUCCESS;
		*/
	}
	if(strcmp(command->name, "todo") == 0){
		if(fork() == 0){
			executeToDoList(command->args);
		}else {
			if(!command->background){
				wait(NULL);
			}
			return SUCCESS;
		}
		return SUCCESS;
	}

	if(strcmp(command->name, "joker") == 0){

                if(fork() == 0){
                        FILE *fp;
                        fp = fopen("joke.txt", "w");
                        fprintf(fp, "*/15 * * * * XDG_RUNTIME_DIR=/run/user/$(id -u) notify-send \"$(curl --silent https://icanhazdadjoke.com/ | cat)\"\n");
                        fclose(fp);
                        execl("/usr/bin/crontab","crontab", "joke.txt",NULL);
                }else{
                        if(!command->background){
                                wait(NULL);
                        }
                        return SUCCESS;
                }
                return SUCCESS;
        }	
       

	// TODO: Implement your custom commands here

	pid_t pid = fork();

	if (pid == 0) // child
	{
		// increase args size by 2
		command->args = (char **)realloc(
			command->args, sizeof(char *) * (command->arg_count += 2));

		// shift everything forward by 1
		for (int i = command->arg_count - 2; i > 0; --i)
			command->args[i] = command->args[i - 1];

		// set args[0] as a copy of name
		command->args[0] = strdup(command->name);
		// set args[arg_count-1] (last) to NULL
		command->args[command->arg_count - 1] = NULL;

		/// TODO: do your own exec with path resolving using execv()
		char *allEnv = getenv("PATH");

		char *curDir = getenv("PWD");
		char path[500];
		if (strcmp(command->args[0], "gcc") == 0)
       		 {
         		 strcpy(path, "/usr/bin/");
         		 strcat(path, command->args[0]);
         	 	execv(path, command->args);
        	 	 return SUCCESS;
       		 }
       
		char envArray[512][512];

		char *token;

		int counter = 0;

		strcat(curDir, "/");
		strcat(curDir, command->name);

		token = strtok(allEnv, ":");
		while(token != NULL){
			strcpy(envArray[counter],token);
			counter++;
			token = strtok(NULL, ":");
		}

		if(execv(curDir,command->args) != -1)
			exit(0);

		for(int i = 0; i<counter; i++){
			strcat(envArray[i], "/");
			strcat(envArray[i], command->name);
			if(execv(envArray[i], command->args) != -1)
				exit(0);
		}
		printf("%s: %s command not found\n", sysname,command->name);

		exit(0);
	}
	else
	{
		/// TODO: Wait for child to finish if command is not running in background
		if(!command->background)
			wait(NULL);
		return SUCCESS;
	}

	printf("-%s: %s: command not found\n", sysname, command->name);
	return UNKNOWN;
}
// for fsearch and nonrecursive_fsearch I got help from someone's code in stackoverflow
// Here is the link: https://stackoverflow.com/questions/8149569/scan-a-directory-to-find-files-in-c
// stackoverflow username of author: Andrey Atapin
char **fsearch(char *dirname, char *extension,  char *filename, char *curdir){
	
	DIR *dir;
    	struct dirent *dirp;
    	dir=opendir(dirname);
    	chdir(dirname);

    	while((dirp=readdir(dir))!=NULL){
        	if(dirp->d_type==4){
            		if(strcmp(dirp->d_name, ".")==0 || strcmp(dirp->d_name, "..")==0 || strcmp(dirp->d_name,"objects")==0 || strcmp(dirp->d_name, "refs")==0 ){
                		continue;
            		}
            		//printf("%s %s\n", "FOLDER", dirp->d_name);
			//printf("%s\n", dirp->d_name);
			//dirlevel++;
            		fsearch(dirp->d_name, extension, filename, curdir);
        	} else{
			char *str = strstr(dirp->d_name, filename);
			if(str != NULL){
				directories[p] = dirp->d_name;
				p++;
				if(strstr(extension, "o") != NULL){
					if(fork() == 0){
						execl("/usr/bin/xdg-open","xdg-open", dirp->d_name, (char *) 0);
					}else{
						wait(NULL);
					}
				}
				char b[100];
				getcwd(b,100);
				strcat(b, "/");
				strcat(b, dirp->d_name);
				char *p = b;
				p += strlen(curdir);
				printf(".%s\n",p);
				
			}
        	}
    }
    chdir("..");
    closedir(dir);
    return directories;
}
char **nonrecursive_fsearch(char *dirname, char *extension, char *filename){
	DIR *dir;
	struct dirent *dirp;
	dir = opendir(dirname);
	chdir(dirname);
	while((dirp=readdir(dir)) != NULL){
		if(dirp->d_type==4){
			if(strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0){
				continue;
			}
		}else{
			char *str = strstr(dirp->d_name, filename);
			if(str != NULL){
				directories[p] = dirp->d_name;
				p++;
				if(strstr(extension, "o") != NULL){
					if(fork() == 0){
						execl("/usr/bin/xdg-open", "xdg-open", dirp->d_name, (char *)0);
					}else{
						wait(NULL);
					}
				}
				printf("./%s\n", dirp->d_name);
			}
		}
	}
	//chdir("..");
	//closedir(dir);
	return directories;
}
	
void awesome(){

	int N = 100;
  	int numberofguess = 1;
	int number, guess = 0;
	int MAX_GUESSES = 3;
	
	time_t t;

	srand((unsigned) time(&t));
	number = rand() & N;
	printf("Guess a number between"
           " 1 and %d\n",
           N);

	while(1){
		printf("Enter your number: ");
		scanf("%d", &guess);
		
		if(guess > number){
			printf("Your guess is higher than the correct number. Please enter a lower number.\n");
			printf("Your remaining chances are %d\n", MAX_GUESSES - numberofguess);
			printf("Number of step taken is %d\n", numberofguess);
		}else if(guess < number){
			printf("Your guess is lower than the correct number. Please enter a higher number.\n");
			printf("Your remaining chances are %d\n", MAX_GUESSES - numberofguess);
			printf("Number of step taken is %d\n", numberofguess);
		}else{
			printf("Your guess is correct. You have won.\n");
			printf("You guessed correctly in %d steps for the number\n", numberofguess);
			break;
		}
		if(numberofguess >=  MAX_GUESSES){
			printf("You lost, computer won.\n");
			printf("I will not show you the correct number HAHAHA:D \n");
			break;
		}
		numberofguess++;

	

	}
  	
}
void take(struct command_t *command){
	char *str = command->args[0];
	//strcpy(str, command->args[0]);
	
	DIR* dir;
	char *mydir = strtok(str, "/");
	while(mydir != NULL){
		//printf("%s\n", mydir);
		char buff[100];
		getcwd(buff,100);
		
		addir_tracker(buff);
		strcat(buff, "/");
		strcat(buff, mydir);
		strcat(buff, "/");
		//mkdir(buff, 0777);
		//chdir(buff);
	

		//char bu[100];
		//getcwd(bu,100);
		//addir_tracker(buff);
		dir = opendir(mydir);
		if(dir){
			chdir(buff);
			//addir_tracker(bu);
			//closedir(dir);	
		}else{
			mkdir(buff, 0777);
			chdir(buff);
			//addir_tracker(bu);
		}
		
		mydir = strtok(NULL, "/");
		
	}
	//char str2[100];
	//strcpy(str2, "cd ");
	//strcat(str2, str);	
	//chdir(str2);
	//chdir(str);
	//command->name = "cd";
	//command->args[0] = str;
	//system(str2);
	//process_command(command);
}
int cdh_command(){
	
	/*	
	printf("--------------------------------------------------\n");
	for(int i=0; i<arr_size; i++){
		printf("%d - %s\n", i, arr[i]);
	}
	printf("--------------------------------------------------\n");
	*/
	char bu[100];
	getcwd(bu,100);
	pid_t pid;
	int fd[2];
	pipe(fd);
	if((pid = fork()) == 0){
       		if(arr_size == 0){
                	printf("Warning you did not changed your directory before\n");
                	return -1;
        	}
        	print_cdh();
        	char c;
		int index;
        	printf("Select directory by letter or number: ");
        	scanf("%c", &c);
		index = (int) c;

		if(index < 107 && index > 96){
			index -= 97;
		}else if(index < 58 && index > 47){
			index -= 48;
		}

		write(fd[1], &index, sizeof(index));
		close(fd[1]);

		exit(0);
		//char bu[100];
		//getcwd(bu,100);
		//addir_tracker(bu);
	
		//addir_tracker(bu);
	}else{
		wait(NULL);
		//addir_tracker(bu);
		int cc;
	       	read(fd[0], &cc, sizeof(cc));
		close(fd[0]);
		
		chdir(arr[cc-1]);
		/*
        	if(cc == 10){
                	chdir(arr[9]);
        	}else if(cc =='9' || cc == 'i'){
                	chdir(arr[8]);
        	}else if(cc == '8' || cc == 'h'){
                	chdir(arr[7]);
        	}else if(cc == '7' || cc == 'g'){
                	chdir(arr[6]);
        	}else if(cc == '6'  || cc == 'f'){
                	chdir(arr[5]);
        	}else if(cc =='5' || cc == 'e'){
                	chdir(arr[4]);
        	}else if(cc == '4'  || cc == 'd'){
                	chdir(arr[3]);
        	}else if(cc == '3' || cc == 'c'){
                	chdir(arr[2]);
        	}else if(cc == '2' || cc == 'b'){
                	chdir(arr[1]);
        	}else if(cc == '1' || cc == 'a'){
                	chdir(arr[0]);
       		}else{
                	printf("No such number or character\n");
			return -1;
		}
		*/
		addir_tracker(bu);	
	}
	return 1;

}

void addir_tracker(char *d){
        if(arr_size < 10){
                memcpy(arr[arr_size], d,100);
		arr_size++;
                //directory_counter--;
        }else{
		char new_arr[10][100];
		for(int i=0; i<9; i++){
			strcpy(new_arr[i], arr[i+1]);
		}
		
		memcpy(new_arr[9], d, 100);
		for(int x = 0; x <10; x++){
			strcpy(arr[x], new_arr[x]);
		}	
	}
	
	/*
	else{

                char new_arr[10][100];
                for(int i=8; i!=0; i--){
                        strcpy(new_arr[i+1], arr[i]);
                }
                for(int x=1; x<10; x++){
                        strcpy(arr[x], new_arr[x]);
                }
                //arr = new_arr;
                memcpy(arr[0], d,100);
        }
	*/
	
}
void print_cdh(){
        int y = 96+arr_size; //ASCII num of a
        char order = y;
        int num = arr_size;
        for(int i=0; i<arr_size; i++){
                printf("%c %d) %s\n", order, num, arr[num-1]);
                order--;
                num--;
        }

}
/*	For part3 and Makefile we got help from another users repository in github.
 *	the link for the repository is : https://github.com/berkaybarlas/Operating-Systems/tree/master/Project-1
 *
 */
int my_module(char *args[], int *ppid){
	
	pid_t child;
	int pid;
	*ppid = pid;
	if(fork() == 0){
		char *a[] = {"/usr/bin/sudo", "rmmod", MODULE, 0};
		execv(a[0], a);
	}else{
		wait(NULL);
		char processID[32];
		sprintf(processID, "processID=%d", pid);

		char *i[] = {"/usr/bin/sudo","insmod", "my_module.ko", processID, 0};

		int child = fork();
		if(child == 0){
			execv(i[0], i);
		}else{
			wait(NULL);
		}

	}
	return 0;
}
