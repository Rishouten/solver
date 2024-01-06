#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <malloc.h>


# define max_size 1024
# define T 1000
# define ET 1e-5
# define DELTA 0.95
# define OLOOP 10
# define ILOOP 200000
# define LIMIT 180000
# define QueueSize 10
# define TRUE 1
# define False 0


typedef struct CirQueue {
	int* data;
	int front;
	int rear;
}*Queue;

Queue InitQueue() {
	Queue q;
	q = (Queue)malloc(sizeof(Queue));
	q->data = (int*)malloc(sizeof(int) * QueueSize);
	q->front = q->rear = 0;
	return q;
}

int QueueFull(Queue q) {
	if (q == NULL) {
		return False;
	}
	return(q->rear + 1) % QueueSize == q->front;
 }

int QueueEmpty(Queue q) {
	if (q == NULL) {
		return False;
	}
	return q->front == q->rear;
}

int QueueLength(Queue q) {
	if (q == NULL) {
		return False;
	}
	return(q->rear - q->front + QueueSize) % QueueSize;
}

int EnQueue(Queue q, int data) {
	if (QueueFull(q)) {
		return False;
	}
	q->data[q->rear] = data;
	q->rear = (q->rear + 1) % QueueSize;
	return TRUE;
}

int DeQueue(Queue q, int* val) {
	if (QueueEmpty(q)) {
		return False;
	}
	*val = q->data[q->front];
	q->front = (q->front + 1) % QueueSize;
	return TRUE;
}

int SearchDupNum(int* array, int num) {
	int i;
	for (i = 0; i < QueueSize; i++) {
		if (num == array[i]) {
			return TRUE;
		}
	}
	return False;
}

int get_row(char* fname) {
	char line[max_size];
	int row = 0;
	FILE* stream = fopen(fname, "r");
	while (fgets(line, max_size, stream)) {
		row++;
	}
	fclose(stream);
	return row;
}

void read_QUBO(char* line, int** data, char* fname) {
	FILE* stream = fopen(fname, "r");
	int i = 0;
	while (fgets(line, max_size, stream)) {
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
	char energy_str[max_size];
	char bit_str[max_size] = { '\0' };
	char str[100] = { '\0' };
	sprintf(energy_str, "%d\n", energy);
	for (int i = 0; i < nbit; i++) {
		sprintf(str, "%d", bit[i]);
		strcat(bit_str, str);
	}
	FILE* stream = fopen(fname, "w+");
	fputs(energy_str, stream);
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

int get_bit(int nbit, int old_Idx) {
	int new_Idx;
	while (1) {
		new_Idx = rand() % nbit;
		printf("%d\n", new_Idx);
		if (new_Idx != old_Idx) {
			break;
		}
	}
	return new_Idx;
}


int flipbit(int flipIdx, int nbit, int energy, int* dE, int* bit, int** elist, int** W) {
	int j, k;
	int dir = 2 * bit[flipIdx] - 1;
	energy += dE[flipIdx];
	dE[flipIdx] = -dE[flipIdx];
	bit[flipIdx] = 1 - bit[flipIdx];
	printf("%d\n", flipIdx);
	for (j = 0; j < nbit; j++) {
		k = elist[flipIdx][j];
		if (k != -1) {
			dE[k] += W[flipIdx][k] * dir * (2 * bit[k] - 1);
		}
	}
	for (j = 0; j < nbit; j++) {
		printf("%d,", dE[j]);
	}
	printf("\n");

	return energy;
}

int** find_nonzero(int** matrix, int row, int col) {
	int i, j;
	int** pos;
	pos = (int**)malloc(sizeof(int*) * row);
	for (i = 0; i < row; i++) {
		pos[i] = (int*)malloc(sizeof(int) * col);
		memset(pos[i], -1, row * sizeof(int));
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

int judge(int dE, double t) {
	int res;
	double num;
	if (dE < 0) {
		res = 1;
	}
	else {
		
		num = rand() / (RAND_MAX + 1.0);
		double eps = exp(-dE / t);
		//printf("%f\n", eps);
		if (eps > num && eps < 1) {
			res = 2;
			//Sleep(1);
			printf("%f,", eps);
			printf("%f\n", num);

		}
		else {
			//Sleep(1);
			res = 0;
			printf("%f,", eps);
			printf("%f\n", num);

		}
	}
	return res;
}


int main(int argc, char* argv[]) {
	int i, j;
	char fname[max_size] = "r256.csv";
	char result_fname[max_size] = "r256-result.txt";
	char line[max_size];
	int** data;
	int nbit;
	int** W;

	//printf("Please enter the input path:");
	//scanf("%s", fname);
	//printf("Please enter the output path:");
	//scanf("%s", result_fname);

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
	for (i = 0; i < nbit; i++) {
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
	int flipIdx = 0;

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
		cnt = 0;
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

	Queue q = InitQueue();


	srand((unsigned)time(NULL));
	int old_Idx = 0;
	int res;
	double t = T;
	int val;
	int* all_flipIdx;
	int count = ILOOP * 10;
	all_flipIdx = (int*)malloc(sizeof(int) * count);
	if (all_flipIdx == NULL) {
		printf("False");
	}
	else {
		for (i = 0; i < count; i++) {
			all_flipIdx[i] = -1;                      //Initial values of bit are 0
		}
	}

	while (t > ET) {
		int unc = 0;
		int Idx = 0;
		for (j = 0; j < ILOOP; j++) {
			flipIdx = get_bit(nbit, old_Idx);
			res = judge(dE[flipIdx], t);
			if (res == 1) {
				energy = flipbit(flipIdx, nbit, energy, dE, bit, elist, W);	
                if (!QueueFull(q)) {
						EnQueue(q, flipIdx);
					}
					else {
						DeQueue(q, &val);
						EnQueue(q, flipIdx);
					}
			
				
				//for (i = 0; i < QueueSize; i++) {
					//printf("%d,", q->data[i]);
				//}
				//printf("\n");
				old_Idx = flipIdx;
				all_flipIdx[Idx] = flipIdx;
				Idx += 1;
				unc = 0;
				
			}
            else if (res == 2){
                if (!SearchDupNum(q->data, flipIdx)) {
				    energy = flipbit(flipIdx, nbit, energy, dE, bit, elist, W);
					if (!QueueFull(q)) {
						EnQueue(q, flipIdx);
					}
					else {
						DeQueue(q, &val);
						EnQueue(q, flipIdx);
					}
                    old_Idx = flipIdx;
				    all_flipIdx[Idx] = flipIdx;
				    Idx += 1;
				    unc = 0;
				}
            }
			else if (res == 0){
				unc++;
				//printf("%d\n", unc);
				if (unc > LIMIT) {
					break;
				}
			}
		}	
		t = t * DELTA;
		//printf("%f\n", t);
	}

	printf("energy = %d", energy);
	printf("\nEnding T = %f", t);
	printf("\nbit = ");
	for (i = 0; i < nbit; i++) {
		printf("%d", bit[i]);
	}
	printf("\n");

	//for (i = 0; i < count; i++) {
		//printf("%d, ", all_flipIdx[i]);
	//}
	printf("\n");
	write_QUBO(energy, nbit, bit, result_fname);
	return(0);
}
