#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <signal.h>
#include "readCPU/readCPU.h"

// Struct for producer-consumer problem
struct buffControl{
	sem_t semEmpty;
	sem_t semFull;
	pthread_mutex_t mutexBuffer;
};

struct buffControl readAnalyze;

int cores;

volatile sig_atomic_t done = 0;
 
void term(int signum)
{
    done = 1;
    printf("SIGTERM!");
}

unsigned long long **buffReadAnalyze[10];
int countReadAnalyze = 0;

void* readCPU(void* args) {

	cores = readCores();

	while (1) {
		// Add to the buffer
		//sleep(1);
		sem_wait(&readAnalyze.semEmpty);
		pthread_mutex_lock(&readAnalyze.mutexBuffer);
		
		buffReadAnalyze[countReadAnalyze] = readCPUfun(cores);
		printf("Reading...\n");
		
		countReadAnalyze++;
		pthread_mutex_unlock(&readAnalyze.mutexBuffer);
		sem_post(&readAnalyze.semFull);
		if(done) break;
    	}
}

void* analyzeCPU(void* args) {
	
    	while (1) {
		unsigned long long **y;

		// Remove from the buffer
		sem_wait(&readAnalyze.semFull);
		pthread_mutex_lock(&readAnalyze.mutexBuffer);
		
		y = buffReadAnalyze[countReadAnalyze - 1];
		
		for(int i = 0; i < cores; i++) free(y[i]);
		free(y);
		printf("Consuming... %d\n", countReadAnalyze);			
		
		countReadAnalyze--;
		pthread_mutex_unlock(&readAnalyze.mutexBuffer);
		sem_post(&readAnalyze.semEmpty);
		
		// Consume
		
		if(done){
			while(countReadAnalyze){
				y = buffReadAnalyze[countReadAnalyze - 1];
				for(int i = 0; i < cores; i++) free(y[i]);
				free(y);
				countReadAnalyze--;
			}
			break;
		}

				
		
    	}
}


int main(int argc, char* argv[]){
	struct sigaction action;
    	memset(&action, 0, sizeof(struct sigaction));
    	action.sa_handler = term;
    	sigaction(SIGTERM, &action, NULL);
    
	srand(time(NULL));
	pthread_t threadID[2];
	pthread_mutex_init(&readAnalyze.mutexBuffer, NULL);
	sem_init(&readAnalyze.semEmpty, 0, 10);
	sem_init(&readAnalyze.semFull, 0, 0);
	int i;
	for (i = 0; i < 2; i++) {
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
	for (i = 0; i < 2; i++) {
		if (pthread_join(threadID[i], NULL) != 0) {
			perror("Failed to join thread");
		}
	}
	sem_destroy(&readAnalyze.semEmpty);
	sem_destroy(&readAnalyze.semFull);
	pthread_mutex_destroy(&readAnalyze.mutexBuffer);
	
	/*
	for(int i = 0; i < 9; i++){
		printf("CPU");
		if(i) printf("%d ", i-1);
		else printf(" ");
		for(int j = 0; j < 7; j++){
			printf("%llu ", a[i][j]);
		}
		printf("\n");
	}*/
	
	return 0;
	
}
