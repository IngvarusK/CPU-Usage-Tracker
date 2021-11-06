#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "analyzeCPU.h"

void getValues(int *cpuCores, struct structData *structDataVal, unsigned long long int **Data){
	for(int i = 0; i < *cpuCores; i++){
		structDataVal[i].Idle = Data[i][3]+Data[i][4];
		structDataVal[i].NonIdle = Data[i][0]+Data[i][1]+Data[i][2]+Data[i][5]+Data[i][6]+Data[i][7];
		structDataVal[i].Total = structDataVal[i].Idle+structDataVal[i].NonIdle;
		free(Data[i]);
	}	
	free(Data);
}

void changeToPrev(int *cpuCores, struct structData *structDataVal){
	for(int i = 0; i < *cpuCores; i++){
		structDataVal[i].PrevIdle = structDataVal[i].Idle;
		structDataVal[i].PrevNonIdle = structDataVal[i].NonIdle;
		structDataVal[i].PrevTotal = structDataVal[i].Total;
	}
}

float* getCpuPerc(int *cpuCores, struct structData *structDataVal){

	float *percCPU = (float*) malloc(*cpuCores*sizeof(float));
	unsigned long long int totald[*cpuCores];
	unsigned long long int idled[*cpuCores];

	for(int i = 0; i < *cpuCores; i++){
		totald[i] = structDataVal[i].Total-structDataVal[i].PrevTotal;
		idled[i] = structDataVal[i].Idle-structDataVal[i].PrevIdle;
	
		percCPU[i] = (float)(totald[i]-idled[i])/totald[i]*100;		
	}
	return percCPU;
}
