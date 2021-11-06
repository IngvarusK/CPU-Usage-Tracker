#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <signal.h>
#include "readCPU/readCPU.h"
#include "analyzeCPU/analyzeCPU.h"

// Struct for dealing with producer-consumer problem
struct buffControl{
	sem_t semEmpty;
	sem_t semFull;
	pthread_mutex_t mutexBuffer;
};

struct buffControl readAnalyze;

int cores = 0;

unsigned long long int **buffReadAnalyze[10];
int countReadAnalyze = 0;

volatile sig_atomic_t done = 0;
void term(int signum)
{
    done = 1;
    printf("SIGTERM!\n");
}

void* readCPU(void* args) {

	cores = readCores();

	while (1) {
		// Add to the buffer
		sleep(1);
		sem_wait(&readAnalyze.semEmpty);
		pthread_mutex_lock(&readAnalyze.mutexBuffer);
		
		buffReadAnalyze[countReadAnalyze] = readCPUfun(cores);
		printf("Reading...%d\n", countReadAnalyze);
		
		countReadAnalyze++;
		pthread_mutex_unlock(&readAnalyze.mutexBuffer);
		sem_post(&readAnalyze.semFull);
		
		//SIGTERM event
		if(done) {printf("JESTEM1!\n"); return NULL;	}
		
    	}
}

void* analyzeCPU(void* args) {
	while(!cores) usleep(1000); // wait for core info
	
	struct structData structDataVal[cores];
	struct structData *structDataPoint = structDataVal;
	
	unsigned long long int **y;
	
	/*-----------------Receiving raw data-----------------*/
	sem_wait(&readAnalyze.semFull);
	pthread_mutex_lock(&readAnalyze.mutexBuffer);
		
	y = buffReadAnalyze[countReadAnalyze - 1];
	getValues(&cores, structDataPoint, y);
	changeToPrev(&cores, structDataPoint);
	
	countReadAnalyze--;
	pthread_mutex_unlock(&readAnalyze.mutexBuffer);
	sem_post(&readAnalyze.semEmpty);
	/*----------------------------------------------------*/	
	
    	while (1) {
		/*-----------------Receiving raw data-----------------*/
		sem_wait(&readAnalyze.semFull);
		pthread_mutex_lock(&readAnalyze.mutexBuffer);
		
		y = buffReadAnalyze[countReadAnalyze - 1];
		getValues(&cores, structDataPoint, y);						
		
		countReadAnalyze--;
		pthread_mutex_unlock(&readAnalyze.mutexBuffer);
		sem_post(&readAnalyze.semEmpty);
		/*----------------------------------------------------*/
		
		if(done) {printf("JESTEM2!\n"); return NULL;	}
	
		/*--------------------Calucations--------------------*/
		float* a = getCpuPerc(&cores, structDataPoint);
		for(int i = 0; i < cores; i++) printf("%.2f ", a[i]);
		changeToPrev(&cores, structDataPoint);
		free(a);
		/*----------------------------------------------------*/
		printf("\n");
	
		//SIGTERM event
		if(done) {printf("JESTEM3!\n"); return NULL;	}	
		
    	}
}


int main(int argc, char* argv[]){
	struct sigaction action;
    	memset(&action, 0, sizeof(struct sigaction));
    	action.sa_handler = term;
    	sigaction(SIGTERM, &action, NULL);
    
	pthread_t threadID[2];
	
	pthread_mutex_init(&readAnalyze.mutexBuffer, NULL);
	sem_init(&readAnalyze.semEmpty, 0, 10);
	sem_init(&readAnalyze.semFull, 0, 0);
	
	for (int i = 0; i < 2; i++) {
		if ((i % 2) == 0) {
			if (pthread_create(&threadID[i], NULL, &readCPU, NULL) != 0) {
			perror("Failed to create thread");
			}
		} else if ((i % 2) == 1) {
			if (pthread_create(&threadID[i], NULL, &analyzeCPU, NULL) != 0) {
			perror("Failed to create thread");
			}
		}
	}
	for (int i = 0; i < 2; i++) {
		if (pthread_join(threadID[i], NULL) != 0) {
			perror("Failed to join thread");
		}
	}
	
	unsigned long long int **pointFree1;
	while(countReadAnalyze){
		pointFree1 = buffReadAnalyze[countReadAnalyze - 1];
		for(int i = 0; i < cores; i++) free(pointFree1[i]);
		free(pointFree1);
		countReadAnalyze--;
	}
	
	sem_destroy(&readAnalyze.semEmpty);
	sem_destroy(&readAnalyze.semFull);
	pthread_mutex_destroy(&readAnalyze.mutexBuffer);
	
	return 0;
	
}
