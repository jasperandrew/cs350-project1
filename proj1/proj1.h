#ifndef lab5_h
#define lab5_h

#include <stdio.h>
#include <stdlib.h>
#include "h_node.h"

h_node *ghHead = NULL;

int* pageTables[1000];
int *globalHistory;
int procList[1000];
int freeSpace = 0;
int  MAXSPACE = 0; 
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