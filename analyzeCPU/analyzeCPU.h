
#ifndef ANALYZECPU_H
#define ANALYZECPU_H

#define ULL unsigned long long int

struct structData{
	unsigned long long int Idle;
	unsigned long long int NonIdle;
	unsigned long long int Total;
	unsigned long long int PrevIdle;
	unsigned long long int PrevNonIdle;
	unsigned long long int PrevTotal;
};

void getValues(int *, struct structData *, unsigned long long int **);
void changeToPrev(int *, struct structData *);
float* getCpuPerc(int *, struct structData *);

#endif // ANALYZECPU_H
