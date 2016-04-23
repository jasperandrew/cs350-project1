// Jasper Andrew and Jacob Zwickler

#ifndef lab5_h
#define lab5_h

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PROCESS_LIMIT 1000
#define DBG 1

typedef struct h_node_s h_node;

struct h_node_s {
	int procNum;
	int vpn;
	int ctr;
	struct h_node_s *prev;
	struct h_node_s *next;
};

// Global variables
h_node *globalHist = NULL;
int *pageTables[PROCESS_LIMIT];
int procList[PROCESS_LIMIT];
int freePages;
int totalPages;
int numFaults = 0;
int numRefs = 0;
int policy;

// Processing functions
void start(int procNum, int addrSz);
void terminate(int procNum);
void reference(int procNum, int  vpn);

// Helper functions
int getValidProcIndex(int procNum, int mode);
int currProcCount();
void cleanUp();

int getAddrSpaceSize(int procListIdx){ return pageTables[procListIdx][0]; }
int pageInMem(int procListIdx, int vpn){ return pageTables[procListIdx][vpn]; }
void storePage(int procListIdx, int vpn){ pageTables[procListIdx][vpn] = 1; freePages--; }
void deletePage(int procListIdx, int vpn){ pageTables[procListIdx][vpn] = 0; freePages++; }

// History functions
void updateHistory(int procNum, int  vpn);
int getHistLength();

// Replacement policies
void replaceLRU();
void replaceRandom();
void replaceNFU();

#endif