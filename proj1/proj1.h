// Jasper Andrew and Jacob Zwickler

#ifndef lab5_h
#define lab5_h

#include <stdio.h>
#include <stdlib.h>

#define PROCESS_LIMIT 1000

typedef struct h_node_s h_node;

struct h_node_s {
	int procNum;
	int vpn;
	struct h_node_s *prev;
	struct h_node_s *next;
};

h_node *globalHist = NULL;
int *pageTables[PROCESS_LIMIT];
int procList[PROCESS_LIMIT];
int freePages;
int totalPages;
int numFaults = 0;
int numRefs = 0;

void initializeProcList();
int validateProc(int procNum, int mode);
int start(int procNum, int addrSz);
int terminate(int procNum);
int reference(int procNum, int  vpn);
void updateHistory(int procNum, int  vpn);
void freeHistory();

#endif