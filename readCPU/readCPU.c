#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "readCPU.h"

int readCores(void){
	/* Dynamically allocated cores */
	FILE *fp;
	
	/* user, nice, system, idle, iwowait, irq, softirq, steal */
	fp = fopen("/proc/stat", "r");
	

	if (fp != NULL) {
		/* Calculation of cores*/
		int cpuCores = 0;		
		while('c' == getc(fp)){
			cpuCores++;
			while('\n' != getc(fp)); // Wait for the next line
		}
		fclose(fp);
		return cpuCores;
	}
	else perror("Failed to read file /proc/stat");
	
	return -1;
}


ULL** readCPUfun(int cpuCores){

	FILE *fp;
	
	/* user, nice, system, idle, iwowait, irq, softirq, steal */
	fp = fopen("/proc/stat", "r");
	

	if (fp != NULL) {
		
		ULL **lineCPU = malloc(cpuCores*sizeof(ULL*));
		
		fseek( fp, 0L, SEEK_SET );
		
		for(int i = 0; i < cpuCores; i++){
			lineCPU[i] = malloc(7*sizeof(ULL));
			
			fscanf(fp, "%*s %llu %llu %llu %llu %llu %llu %llu", 
				&lineCPU[i][0],&lineCPU[i][1],&lineCPU[i][2],&lineCPU[i][3],
				&lineCPU[i][4],&lineCPU[i][5],&lineCPU[i][6]);
			while('\n' != getc(fp)); // Wait for the next line
			
			/*
			printf("%llu %llu %llu %llu %llu %llu %llu\n",
				lineCPU[i][0],lineCPU[i][1],lineCPU[i][2],lineCPU[i][3],
				lineCPU[i][4],lineCPU[i][5],lineCPU[i][6]);
			*/
					
		}	
		fclose(fp);
		return lineCPU;
	}
	else perror("Failed to read file /proc/stat");
	
	ULL **noneList = malloc(sizeof(ULL*));
	return noneList;
	
}
