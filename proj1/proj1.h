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
int freeSpace = 0;
int MAXSPACE = 0;
int faults = 0;
double totalRef = 0;
double fRate = 0;

void validateArgs(char* arg1, char* arg2);
int  validateProc(int procNum, char mode);
void initializeList();
void start(int procNum, int addrSz);
void terminate(int procNum);
void refer(int procNum,int  vpn);

#endif