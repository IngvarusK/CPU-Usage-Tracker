#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "readCPU/readCPU.h"


int main(){

	unsigned long long **a = readCPUfun();
	
	
	for(int i = 0; i < 9; i++){
		printf("CPU");
		if(i) printf("%d ", i-1);
		else printf(" ");
		for(int j = 0; j < 7; j++){
			printf("%llu ", a[i][j]);
		}
		printf("\n");
	}
	
	/* Free */
	for(int i = 0; i < 9; i++){
		free(a[i]);
	}
	free(a);
	return 0;
	
}
