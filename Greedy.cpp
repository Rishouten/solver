#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_row(char* fname) {
	char line[1024];
	int row = 0;
	FILE* stream = fopen(fname, "r");
	while (fgets(line, 1024, stream)) {
		row++;
	}
	fclose(stream);
	return row;
}

void read_QUBO(char* line, int** data, char* fname) {
	FILE* stream = fopen(fname, "r");
	int i = 0;
	while (fgets(line, 1024, stream)) {
		int j = 0;
		char* tok;
		for (tok = strtok(line, ","); tok && *tok; j++, tok = strtok(NULL, ",\n")) {
			data[i][j] = atoi(tok);
		}
		i++;
	}
	fclose(stream);
}

void write_QUBO(int energy, int nbit, int* bit, char* fname) {
	char energy_str[1024];
	char bit_str[1024] = {'\0'};
	char str[100] = { '\0' };
	sprintf(energy_str, "%d\n", energy);
	for (int i = 0; i < nbit; i++) {
		sprintf(str, "%d", bit[i]);
		strcat(bit_str, str);
	}	
	FILE* stream = fopen(fname, "w+");
	fputs(energy_str,stream);
	fputs(bit_str, stream);
	fclose(stream);
}

void symmetric(int** matrix, int nbit) {
	int i, j;
	for (i = 0; i < nbit; i++) {
		for (j = 0; j < nbit; j++) {
			if (i != j) {
				matrix[i][j] += matrix[j][i];
			}
		}
	}
}

int find_min(int* array, int len) {
	int i;
	int* minP = array;
	for (i = 1; i < len; i++) {
		minP = (*minP <= *(array + i) ? minP : (array + i));
	}
	int minIdx = (minP - array);
	return minIdx;
}

int greedy_flip(int* dE, int nbit) {
	int minIdx = find_min(dE, nbit);
	if (dE[minIdx] < 0) {

		return minIdx;
	}
	else {
		return -1;  //  No negative value of dE.
	}
}

int flipbit(int minIdx,int nbit, int energy, int* dE, int* bit, int** elist, int** W) {
	int j,k;
	int dir = 2 * bit[minIdx] - 1;
	energy += dE[minIdx];
	dE[minIdx] = -dE[minIdx];
	bit[minIdx] = 1 - bit[minIdx];
	for (j = 0; j < nbit; j++) {
		k = elist[minIdx][j];
		if (k != -1) {
			dE[k] += W[minIdx][k] * dir * (2 * bit[k] - 1);	
		}
	}
	return energy;
}

int** find_nonzero(int** matrix,int row,int col) {
	int i, j;
	int** pos;
	pos = (int**)malloc(sizeof(int*) * row);  
	for (i = 0; i < row; i++) {
		pos[i] = (int*)malloc(sizeof(int) * col);
		memset(pos[i],-1, row * sizeof(int));       
	}
	for (i = 0; i < row; i++) {
		for (j = 0; j < col; j++) {
			if (matrix[i][j] != 0 && i != j) {
				pos[i][j] = j;
			}
		}
	}
	return pos;
}

int main(int argc, char* argv[]) {
	int i, j;
	char fname[1024];
	char result_fname[1024];
	char line[1024];
	int** data;
	int nbit;
	int** W;

	printf("Please enter the input path:");
	scanf("%s", fname);
	printf("Please enter the output path:");
	scanf("%s", result_fname);

	int row = get_row(fname);
	data = (int**)malloc(row * sizeof(int*));
	for (int i = 0; i < row; i++) {
		data[i] = (int*)malloc(row * sizeof(int));
	}
	read_QUBO(line, data, fname);
	nbit = *data[0];
	
	W = (int**)malloc(sizeof(int*) * nbit);
	for (i = 0; i < nbit; i++) {
		W[i] = (int*)malloc(sizeof(int) * nbit);
		memset(W[i], 0, nbit * sizeof(int));
	}
	int L2 = nbit - 1;
	for (i = 0; i < nbit ; i++) {
		int L1 = nbit - 1;
		for (j = L2; j >= 0; j--) {
			W[i][L1] = data[i + 1][j];   
			L1--;
		}
		L2--;
	}
	symmetric(W, nbit); //Build QUBO matrix

	int energy = 0;
	int* dE;
	int* bit;
	int** elist = find_nonzero(W, nbit, nbit); // -1 represents no path
	int* nedge;
	int cnt;
	int minIdx;

	nedge = (int*)malloc(sizeof(int) * nbit);   //Memory allocation
	if (nedge == NULL) {
		printf("False");
	}
	else {
		for (i = 0; i < nbit; i++) {
			cnt = 0;
			for (j = 0; j < nbit; j++) {
				if (elist[i][j] != -1) {
					cnt++;
				}
			}
			nedge[i] = cnt;
		}
	}
	dE = (int*)malloc(sizeof(int) * nbit);   
	if (dE == NULL) {
		printf("False");
	}
	else {
		for (i = 0; i < nbit; i++) {
			dE[i] = W[i][i];                  //Initial values of dE are diagonal elements 
		}
	}

	bit = (int*)malloc(sizeof(int) * nbit); 
	if (bit == NULL) {
		printf("False");
	}
	else {
		for (i = 0; i < nbit; i++) {
			bit[i] = 0;                      //Initial values of bit are 0
		}
	}
	minIdx = greedy_flip(dE, nbit);
	while (minIdx != -1) {	
		energy = flipbit(minIdx, nbit, energy, dE, bit, elist, W);
		minIdx = greedy_flip(dE, nbit);
	}
	printf("energy = %d", energy);
	printf("\nbit = ");
	for (i = 0; i < nbit; i++) {
		printf("%d", bit[i]);
	}
	printf("\n");	
	write_QUBO(energy, nbit, bit, result_fname);
	return(0);
}
