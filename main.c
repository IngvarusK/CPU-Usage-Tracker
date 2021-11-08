#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <ncurses.h> // sudo apt-get install libncurses5-dev
#include <time.h>
#include "readCPU/readCPU.h"
#include "analyzeCPU/analyzeCPU.h"

#define us_wait 1000000 // sampling delay in us (micro seconds)

// Struct for dealing with producer-consumer problem
struct buffControl{
	sem_t semEmpty;
	sem_t semFull;
	pthread_mutex_t mutexBuffer;
};

struct buffControl readAnalyze, analyzeShow;

int cores = 0;

// Read-Analyze communication
unsigned long long int **buffReadAnalyze[10];
int countReadAnalyze = 0;

// Analyze-Show communication
float *buffAnalyzeShow[10];
int countAnalyzeShow = 0;

// Sigterm signal handler
volatile sig_atomic_t done = 0;
void term(int signum)
{
    done = 1;
    printf("SIGTERM!\n");
}

// For Watchdog purpose
time_t seconds[4];

void* readCPU(void* args) {

	cores = readCores();

	while (1) {
		seconds[0] = time(NULL);
		// Add to the buffer
		usleep(us_wait);
		sem_wait(&readAnalyze.semEmpty);
		pthread_mutex_lock(&readAnalyze.mutexBuffer);
		
		buffReadAnalyze[countReadAnalyze] = readCPUfun(cores);
		//printf("Reading...%d\n", countReadAnalyze);
		
		countReadAnalyze++;
		pthread_mutex_unlock(&readAnalyze.mutexBuffer);
		sem_post(&readAnalyze.semFull);
		
		//SIGTERM event
		if(done){
			//printf("SIGTERM EXIT1!\n");
			return NULL;
		}
		
    	}
}

void* analyzeCPU(void* args) {
	while(!cores) usleep(1000); // wait for core info
	
	// Keeping 8 variables per core from /proc/sys
	struct structData structDataVal[cores];
	struct structData *structDataPoint = structDataVal;
	
	/*-----------------Receiving raw data-----------------*/
	sem_wait(&readAnalyze.semFull);
	pthread_mutex_lock(&readAnalyze.mutexBuffer);
		
	getValues(&cores, structDataPoint, buffReadAnalyze[countReadAnalyze - 1]);
	changeToPrev(&cores, structDataPoint);
	
	countReadAnalyze--;
	pthread_mutex_unlock(&readAnalyze.mutexBuffer);
	sem_post(&readAnalyze.semEmpty);
	/*----------------------------------------------------*/	
	
    	while (1) {
    		seconds[1] = time(NULL);
		/*-----------------Receiving raw data-----------------*/
		sem_wait(&readAnalyze.semFull);
		pthread_mutex_lock(&readAnalyze.mutexBuffer);
		
		getValues(&cores, structDataPoint, buffReadAnalyze[countReadAnalyze - 1]);	
		
			/*--------------------Calucations--------------------*/
			sem_wait(&analyzeShow.semEmpty);
			pthread_mutex_lock(&analyzeShow.mutexBuffer);
			
			buffAnalyzeShow[countAnalyzeShow] = getCpuPerc(&cores, structDataPoint);
			changeToPrev(&cores, structDataPoint);
			//printf("Sending to Show...%d\n", countAnalyzeShow);
			
			countAnalyzeShow++;
			pthread_mutex_unlock(&analyzeShow.mutexBuffer);
			sem_post(&analyzeShow.semFull);
			/*----------------------------------------------------*/					
		
		countReadAnalyze--;
		pthread_mutex_unlock(&readAnalyze.mutexBuffer);
		sem_post(&readAnalyze.semEmpty);
		/*----------------------------------------------------*/		
		
		//SIGTERM event
		if(done){
			//printf("SIGTERM EXIT2!\n");
			return NULL;
		}	
		
    	}
}

void* showCPU(void* args) {

	while(!cores) usleep(1000); // wait for core info
	
	// ncurses.h window terminal
	struct winsize size;
	
	WINDOW * mainwin = initscr();		/* Start curses mode 	*/
	printw("Cpu Tracker loading...");	/* Print Hello World	*/
	
	int width = 0;
	int width_load;	
	
	while(1){
		seconds[2] = time(NULL);
		sem_wait(&analyzeShow.semFull);
		pthread_mutex_lock(&analyzeShow.mutexBuffer);
		
		//printf("Showing...%d\n", countAnalyzeShow-1);
		
		refresh();			/* Print it on to the real screen */
		clear();
		ioctl( 0, TIOCGWINSZ, (char *) &size );
		move(0,(int)((size.ws_col-21)/2));
		printw("CPU Usage Tracker IgorK");	/* Print Hello World	*/
		width = size.ws_col - 11;
		//printw( "Rows: %u\nCols: %u\n", size.ws_row, size.ws_col );
		move(2,0);
		printw(" CPU Total Load: %.2f%\n\n", buffAnalyzeShow[countAnalyzeShow - 1][0]);
		for(int i = 1; i < cores; i++){
			printw(" %d[", i-1);
			width_load = (int)((buffAnalyzeShow[countAnalyzeShow - 1][i] / 100)*width);
			for(int i = 0; i < width_load; i++) printw("|");
			for(int i = 0; i < (width-width_load); i++) printw(" ");
			printw("] ");
			if(buffAnalyzeShow[countAnalyzeShow - 1][i] < 10) printw(" ");
			printw("%.1f% \n", buffAnalyzeShow[countAnalyzeShow - 1][i]);
		}
		//printw("%d\n",width);
		free(buffAnalyzeShow[countAnalyzeShow - 1]);

		countAnalyzeShow--;
		pthread_mutex_unlock(&analyzeShow.mutexBuffer);
		sem_post(&analyzeShow.semEmpty);
	
		
	
		//SIGTERM event
		if(done){
			// Close the window
			delwin(mainwin);
			endwin();
			refresh();
			//printf("SIGTERM EXIT3!\n");
			return NULL;
		}
	}
	
}

void* watchDog(void* args) {

	/* Program saves the last time the begin of each infinite loop is procceded
		then it's compared to stable watchdog thread function		*/
	while(1){
		sleep(2);
		seconds[4] = time(NULL);
		if((seconds[4]-seconds[0]) >= 2) perror("Program stopped working properly readCPU() fault");
		if((seconds[4]-seconds[1]) >= 2) perror("Program stopped working properly analyzeCPU() fault");
		if((seconds[4]-seconds[2]) >= 2) perror("Program stopped working properly showCPU() fault");
		if(done){
			//printf("SIGTERM EXIT1!\n");
			return NULL;
		}
	}
}


int main(int argc, char* argv[]){

	//Sigterm handler
	struct sigaction action;
    	memset(&action, 0, sizeof(struct sigaction));
    	action.sa_handler = term;
    	sigaction(SIGTERM, &action, NULL);
    
	pthread_t threadID[4];
	
	// Read-Analyze communication
	pthread_mutex_init(&readAnalyze.mutexBuffer, NULL);
	sem_init(&readAnalyze.semEmpty, 0, 10);
	sem_init(&readAnalyze.semFull, 0, 0);
	// Analyze-Show communication
	pthread_mutex_init(&analyzeShow.mutexBuffer, NULL);
	sem_init(&analyzeShow.semEmpty, 0, 10);
	sem_init(&analyzeShow.semFull, 0, 0);
	
	for (int i = 0; i < 4; i++) {
		if ((i % 4) == 0) {
			if (pthread_create(&threadID[i], NULL, &readCPU, NULL) != 0) {
			perror("Failed to create thread");
			}
		} else if ((i % 4) == 1) {
			if (pthread_create(&threadID[i], NULL, &analyzeCPU, NULL) != 0) {
			perror("Failed to create thread");
			}
		} else if ((i % 4) == 2) {
			if (pthread_create(&threadID[i], NULL, &showCPU, NULL) != 0) {
			perror("Failed to create thread");
			}
		} else if ((i % 4) == 3) {
			if (pthread_create(&threadID[i], NULL, &watchDog, NULL) != 0) {
			perror("Failed to create thread");
			}
		}
	}
	for (int i = 0; i < 4; i++) {
		if (pthread_join(threadID[i], NULL) != 0) {
			perror("Failed to join thread");
		}
	}
	
	// Delete all variables made by malloc() (** unsigned long long int and *long long int and)
	while(countReadAnalyze){
		for(int i = 0; i < cores; i++) free(buffReadAnalyze[countReadAnalyze - 1][i]);
		free(buffReadAnalyze[countReadAnalyze - 1]);
		countReadAnalyze--;
	}
	// Delete all variables made by malloc() (* float)
	while(countAnalyzeShow){
		free(buffAnalyzeShow[countAnalyzeShow - 1]);
		countAnalyzeShow--;
	}
	
	sem_destroy(&readAnalyze.semEmpty);
	sem_destroy(&readAnalyze.semFull);
	pthread_mutex_destroy(&readAnalyze.mutexBuffer);
	
	sem_destroy(&analyzeShow.semEmpty);
	sem_destroy(&analyzeShow.semFull);
	pthread_mutex_destroy(&analyzeShow.mutexBuffer);
	
	return 0;
	
}
