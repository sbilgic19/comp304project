#include "queue.c"
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
int simulationTime = 120;    // simulation time
int seed = 10;               // seed for randomness
int emergencyFrequency = 40; // frequency of emergency
float p = 0.2;               // probability of a ground job (launch & assembly)
int MAX_WAIT_TIME = 20;       
FILE *fp;

pthread_mutex_t mutexCt;
pthread_mutex_t mutexLand;
pthread_mutex_t mutexAssembly;
pthread_mutex_t mutexLaunch;
pthread_mutex_t mutexPadA;
pthread_mutex_t mutexPadB;
pthread_mutex_t mcount;
pthread_mutex_t mutexEmergency;
pthread_cond_t threshold;

Queue *padA;
Queue *padB;
Queue *assembly;
Queue *launch;
Queue *land;
Queue *emergency;

long now;
int count = 1;
void* prQueue(void *arg);
void* LandingJob(void *arg); 
void* LaunchJob(void *arg);
void* EmergencyJob(void *arg); 
void* AssemblyJob(void *arg); 
void* ControlTower(void *arg); 
void* padAJob(void *arg);
void* padBJob(void *arg);
// pthread sleeper function
int pthread_sleep (int seconds)
{
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    struct timespec timetoexpire;
    if(pthread_mutex_init(&mutex,NULL))
    {
        return -1;
    }
    if(pthread_cond_init(&conditionvar,NULL))
    {
        return -1;
    }
    struct timeval tp;
    //When to expire is an absolute time, so get the current time and add it to our delay time
    gettimeofday(&tp, NULL);
    timetoexpire.tv_sec = tp.tv_sec + seconds; timetoexpire.tv_nsec = tp.tv_usec * 1000;
    
    pthread_mutex_lock (&mutex);
    int res =  pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock (&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);
    
    //Upon successful completion, a value of zero shall be returned
    return res;
}

int nth = -1;
int main(int argc,char **argv){
    // -p (float) => sets p
    // -t (int) => simulation time in seconds
    // -s (int) => change the random seed
    for(int i=1; i<argc; i++){
        if(!strcmp(argv[i], "-p")) {p = atof(argv[++i]);}
        else if(!strcmp(argv[i], "-t")) {simulationTime = atoi(argv[++i]);}
        else if(!strcmp(argv[i], "-s"))  {seed = atoi(argv[++i]);}
	else if(!strcmp(argv[i], "-n")) {nth = atoi(argv[++i]);}
    }
    
    srand(seed); // feed the seed
 	    
 
    time(&now);
    struct timeval currentTime;
    gettimeofday(&currentTime,NULL);
	
    fp = fopen("events.log", "w");
    pthread_attr_t attr;

    pthread_mutex_init(&mutexCt,NULL);
    pthread_mutex_init(&mutexAssembly,NULL);
    pthread_mutex_init(&mutexLand,NULL);
    pthread_mutex_init(&mutexLaunch,NULL);
    pthread_mutex_init(&mutexPadA,NULL);
    pthread_mutex_init(&mutexPadB,NULL);
    pthread_mutex_init(&mcount,NULL);
    pthread_mutex_init(&mutexEmergency,NULL);

    pthread_cond_init(&threshold,NULL);

       /* Queue usage example
        Queue *myQ = ConstructQueue(1000);
        Job j;
        j.ID = myID;
        j.type = 2;
        Enqueue(myQ, j);
        Job ret = Dequeue(myQ);
        DestructQueue(myQ);
    */

    // your code goes here	    
	
	
	
   

    //printf("EventId\tStatus\tRequestTime\tEndTime\tTurnaroundTime\tPad\n");
    fprintf(fp,"EventId\tStatus\tRequestTime\tEndTime\tTurnaroundTime\tPad\n");
    //printf("------------------------------------------------------------------------------\n");
    fprintf(fp, "------------------------------------------------------------------------------\n");
    padA = ConstructQueue(1000);
    padB = ConstructQueue(1000);
    assembly = ConstructQueue(1000);
    launch = ConstructQueue(1000);
    land = ConstructQueue(1000);
    emergency = ConstructQueue(1000);

    
    Job j;
    j.ID = 0;
    j.type = 1;
    j.requestTime = 0;   
    Enqueue(launch, j);

    //Enqueue(padA, j);

     pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	
    pthread_t t1;
	     pthread_t t2;
	     pthread_t t3;
	     pthread_t t4;
	     pthread_t t5;
	    pthread_t t6;
	    pthread_t t7;
	   pthread_t t8;

    pthread_create(&t1, &attr, ControlTower, NULL);
    pthread_create(&t2, &attr, padAJob, NULL);
    pthread_create(&t3, &attr, padBJob, NULL);
    pthread_create(&t4, &attr, LaunchJob, NULL);
    pthread_create(&t5, &attr, LandingJob, NULL);
    pthread_create(&t6, &attr, AssemblyJob, NULL);
    pthread_create(&t7, &attr, EmergencyJob, NULL);
    
    if(nth != -1){
    	pthread_create(&t8, &attr, prQueue, NULL);
	}

    pthread_attr_destroy(&attr);
    
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    pthread_join(t5, NULL);
    pthread_join(t6, NULL);
    pthread_join(t7, NULL);
    
    if(nth != -1){
    	pthread_join(t8, NULL);
	}
    pthread_exit(NULL);   

	
	DestructQueue(padA);
	DestructQueue(padB);
	DestructQueue(launch);
	DestructQueue(assembly);
	DestructQueue(land);
	DestructQueue(emergency);

    pthread_mutex_destroy(&mutexPadA);
    pthread_mutex_destroy(&mutexPadB);
    pthread_mutex_destroy(&mutexCt);
    pthread_mutex_destroy(&mutexLand);
    pthread_mutex_destroy(&mutexAssembly);
    pthread_mutex_destroy(&mutexLaunch);
    pthread_mutex_destroy(&mcount);
    pthread_mutex_destroy(&mutexEmergency);
    
    fclose(fp);
    return 0;
}


// the function that creates plane threads for landing
void* LandingJob(void *arg){
        struct timeval time1;
        long time2;
        gettimeofday(&time1,NULL);

        long currentTime = time1.tv_sec;
        time2 = now + simulationTime;
	
        while(currentTime < time2){
		
		pthread_sleep(2);
		float random = (float) rand() / RAND_MAX;
                float probLand = 1-p;
                if(random< probLand){

		     Job j;
                     
		     pthread_mutex_lock(&mcount);
                     j.ID = count;
		     count++;
		     pthread_mutex_unlock(&mcount);
                     
		     j.type = 3;   // land
                     j.requestTime = (int)currentTime-now; 
		     

                     pthread_mutex_lock(&mutexLand);
		     Enqueue(land, j);
                     pthread_mutex_unlock(&mutexLand);
			
		    
		}

		gettimeofday(&time1, NULL);
		currentTime = time1.tv_sec;
	}
	
	
}

// the function that creates plane threads for departure
void* LaunchJob(void *arg){
	
	struct timeval time1;
        long time2;
        gettimeofday(&time1,NULL);

        long currentTime = time1.tv_sec;
        time2 = now + simulationTime;

        while(currentTime < time2){

		pthread_sleep(2);
                float random = (float)rand()/RAND_MAX;
                float prob = p/2;
                if(random< prob){

                     Job j; 
                     
		     pthread_mutex_lock(&mcount);
                     j.ID = count;
		     count++;
		     pthread_mutex_unlock(&mcount);

		     
		     j.type = 1;   // launch
                     j.requestTime = (int)currentTime-now; 
                     

                     pthread_mutex_lock(&mutexLaunch);
		     Enqueue(launch, j);
                     pthread_mutex_unlock(&mutexLaunch);
                
		}

		gettimeofday(&time1, NULL);
		currentTime = time1.tv_sec;


        }
	
}


// the function that creates plane threads for emergency landing
void* EmergencyJob(void *arg){
        struct timeval time1;
        long time2;
        gettimeofday(&time1,NULL);

        long currentTime = time1.tv_sec;
        time2 = now + simulationTime;

        while(currentTime < time2){

		pthread_sleep(2);

                if((currentTime-now) % emergencyFrequency == 0 && (currentTime-now) != 0){
			
                     pthread_mutex_lock(&mutexEmergency);
                     Job j1;
                     
		     pthread_mutex_lock(&mcount);
                     j1.ID = count;
		     count++;
		     pthread_mutex_unlock(&mcount);

                     j1.type = 4;   // emergency
                     j1.requestTime = (int)currentTime - now;
                     
                     Enqueue(emergency, j1);
                   	

		     Job j2;
                     
		     pthread_mutex_lock(&mcount);
                     j2.ID = count;
		     count++;
		     pthread_mutex_unlock(&mcount);

		     j2.type = 4;   // emergency
                     j2.requestTime = (int)currentTime - now;

		     Enqueue(emergency, j2);	
                     pthread_mutex_unlock(&mutexEmergency);
			
                }

                gettimeofday(&time1, NULL);
                currentTime = time1.tv_sec;
        }

}
// the function that creates plane threads for emergency landing
void* AssemblyJob(void *arg){

	struct timeval time1;
        long time2;
        gettimeofday(&time1,NULL);

        long currentTime = time1.tv_sec;
        time2 = now + simulationTime;

        while(currentTime < time2){
		pthread_sleep(2);
                float random = (float)rand()/RAND_MAX;
                float prob = p/2;
                if(random< prob){

                     Job j;
			
		     pthread_mutex_lock(&mcount);
                     j.ID = count;
		     count++;
		     pthread_mutex_unlock(&mcount);


                     j.type = 2;   // assembly
		     j.requestTime = (int)currentTime - now;


                     pthread_mutex_lock(&mutexAssembly);
		     Enqueue(assembly, j);
                     pthread_mutex_unlock(&mutexAssembly);
                	
		}

		gettimeofday(&time1, NULL);
		currentTime = time1.tv_sec;
        }

}

// the function that controls the air traffic
void* ControlTower(void *arg){
	struct timeval time1;
	long time2;
	gettimeofday(&time1,NULL);
	
	long currentTime = time1.tv_sec;
	time2 = now + simulationTime;
	
	int evenORodd = 0;
	int counter = 0;
	while(currentTime < time2){
	

		if(!isEmpty(emergency)){

			pthread_mutex_lock(&mutexEmergency);
			Job ret1 = Dequeue(emergency);
			Job ret2 = Dequeue(emergency);
			pthread_mutex_unlock(&mutexEmergency);

			pthread_mutex_lock(&mutexPadA);
			NODE *emergent = (NODE*) malloc(sizeof(NODE));
			//NODE *emergent;
			emergent->data = ret1;
			emergent->prev = padA->head;
			padA->head = emergent;
			padA->size++;
			pthread_mutex_unlock(&mutexPadA);

			pthread_mutex_lock(&mutexPadB);
                        NODE *emergent2 = (NODE*) malloc(sizeof (NODE));
                        //NODE *emergent2;
			emergent2->data = ret2;
                        emergent2->prev = padB->head;
                        padB->head = emergent2;
			padB->size++;
			pthread_mutex_unlock(&mutexPadB);

		}else{
			int launchTime = 0;
			int assemblyTime = 0;
			if(!isEmpty(launch)){
				Job launchItem;
				launchItem = launch->head->data;
				launchTime = currentTime - launchItem.requestTime;
			}
			if(!isEmpty(assembly)){
				Job assemblyItem;
				assemblyItem = assembly->head->data;
				assemblyTime = currentTime - assemblyItem.requestTime;
			}
			if((isEmpty(land) || launch->size >= 3 || assembly->size >= 3 || launchTime >= MAX_WAIT_TIME || assemblyTime >= MAX_WAIT_TIME) &&  counter < 20){
				if(!isEmpty(launch)){	
				
					pthread_mutex_lock(&mutexLaunch);
					Job ret = Dequeue(launch);
					pthread_mutex_unlock(&mutexLaunch);
			
					pthread_mutex_lock(&mutexPadA);
					Enqueue(padA, ret);
					pthread_mutex_unlock(&mutexPadA);
					counter++;
		    		}
                    		if(!isEmpty(assembly)){

                       			pthread_mutex_lock(&mutexAssembly);
                        		Job reta = Dequeue(assembly);
					pthread_mutex_unlock(&mutexAssembly);
			
                        		pthread_mutex_lock(&mutexPadB);
					Enqueue(padB, reta);
					pthread_mutex_unlock(&mutexPadB);
					counter++;
                    		}		
		    
			}else{
				counter=0;
	   	    		if(evenORodd % 4 == 0){
			
                        		pthread_mutex_lock(&mutexLand);
                        		Job retl = Dequeue(land);
                        		pthread_mutex_unlock(&mutexLand);
			
                        		pthread_mutex_lock(&mutexPadB);
					Enqueue(padB, retl);
                        		pthread_mutex_unlock(&mutexPadB);
		    		}else{
			
					pthread_mutex_lock(&mutexLand);
                        		Job retla = Dequeue(land);
                        		pthread_mutex_unlock(&mutexLand);
			
                        		pthread_mutex_lock(&mutexPadA);
					Enqueue(padA, retla);
                        		pthread_mutex_unlock(&mutexPadA);

		    		}
		    		evenORodd++;
			}
	
		}
		gettimeofday(&time1, NULL);
		currentTime = time1.tv_sec;
		pthread_sleep(2);
	}

}

void* padAJob(void* arg){
	struct timeval time;
	long time2;

	gettimeofday(&time, NULL);
	long currentTime = time.tv_sec;
        time2 = now + simulationTime;

	while(currentTime < time2){
		
		if(isEmpty(padA)){
			pthread_sleep(2);
		}else{
			char c;
			pthread_mutex_lock(&mutexPadA);
			Job ret = Dequeue(padA);
			ret.endTime = (int)currentTime - now;
			pthread_mutex_unlock(&mutexPadA);
			if(ret.type == 1){
				c = 'D';
				pthread_sleep(2);
				fprintf(fp,"\t%d\t%c\t%d\t%d\t%d\tA\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
				//printf("%d\t%c\t%d\t%d\t%d\tA\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
			}else if(ret.type == 2){
				c = 'A';
				pthread_sleep(6);
				fprintf(fp,"\t%d\t%c\t%d\t%d\t%d\tA\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
				//printf("%d\t%c\t%d\t%d\t%d\tA\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
			}else if(ret.type == 4){
				c = 'E';
				pthread_sleep(1);
				fprintf(fp,"\t%d\t%c\t%d\t%d\t%d\tA\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
				//printf("%d\t%c\t%d\t%d\t%d\tA\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
			}else{
				c = 'L';
				pthread_sleep(1);
				fprintf(fp,"\t%d\t%c\t%d\t%d\t%d\tA\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
				//printf("%d\t%c\t%d\t%d\t%d\tA\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
			}

		}

		gettimeofday(&time, NULL);
		currentTime = time.tv_sec;

	}	
}

void* padBJob(void *arg){
	struct timeval time;
	long time2;
	
	gettimeofday(&time, NULL);
	long currentTime = time.tv_sec;
        time2 = now + simulationTime;

	while(currentTime < time2){
		if(isEmpty(padB)){
			pthread_sleep(2);
		}else{
			char c;
			pthread_mutex_lock(&mutexPadB);
			Job ret = Dequeue(padB);
			ret.endTime = (int)currentTime - now;
			pthread_mutex_unlock(&mutexPadB);
			if(ret.type == 1){
				c = 'D';
				pthread_sleep(2);
				fprintf(fp,"\t%d\t%c\t%d\t%d\t%d\tB\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
				//printf("%d\t%c\t%d\t%d\t%d\tB\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
			}else if(ret.type == 2){
				c = 'A';
				pthread_sleep(6);
				fprintf(fp,"\t%d\t%c\t%d\t%d\t%d\tB\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
				//printf("%d\t%c\t%d\t%d\t%d\tB\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
			}else if(ret.type == 4){
				c = 'E';
				pthread_sleep(1);
				fprintf(fp,"\t%d\t%c\t%d\t%d\t%d\tB\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
				//printf("%d\t%c\t%d\t%d\t%d\tB\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
			}else{
				c = 'L';
				pthread_sleep(1);	
				fprintf(fp,"\t%d\t%c\t%d\t%d\t%d\tB\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
				//printf("%d\t%c\t%d\t%d\t%d\tB\n",ret.ID, c, ret.requestTime, ret.endTime, ret.endTime-ret.requestTime);
			}


	
		}
		gettimeofday(&time, NULL);
		currentTime = time.tv_sec;

	}	
}

void* prQueue(void *arg){
	struct timeval time;
	long time2;

	gettimeofday(&time, NULL);
	long currentTime = time.tv_sec;

	time2 = now + simulationTime;
	while(currentTime < time2){
		if((currentTime-now >= nth) && (nth != -1)){
				//printQueue
				printf("At %ld sec landing : ", currentTime-now);
				printQueue(land);
				printf("\n");

				printf("At %ld sec launching : ", currentTime-now);
				printQueue(launch);
				printf("\n");

				printf("At %ld sec assembly : ", currentTime-now);
				printQueue(assembly);
				printf("\n");
				
				printf("\n");

		}
		pthread_sleep(1);
		gettimeofday(&time, NULL);
		currentTime = time.tv_sec;	
	}
}
