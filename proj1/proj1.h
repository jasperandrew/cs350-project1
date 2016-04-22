// Jasper Andrew and Jacob Zwickler

#ifndef lab5_h
#define lab5_h

#include <stdio.h>
#include <stdlib.h>
#include "h_node.h"

#define PROCESS_LIMIT 1000

h_node *ghHead = NULL;

int *pageTables[PROCESS_LIMIT];
int *globalHistory;
int procList[PROCESS_LIMIT];
int freePages = 0;
int totalPages = 0;
int faults = 0;
double totalRef = 0;
double fRate = 0;

int validateProc(int procNum, int mode);
void initializeProcList();
int start(int procNum, int addrSz);
int terminate(int procNum);
int reference(int procNum,int  vpn);

#endif