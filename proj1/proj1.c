#include "proj1.h"

int start(int procNum, int addrSpaceSize)
{
  int i = 0, j;
  
  //search if process already running 
  int valid = validateProc(procNum, 1);
	
  if(valid < 0) return 1;
	
	//search if limit for processes reached - 
	while(procList[i] != 0){
		if(i < PROCESS_LIMIT){
			i++;
		}else{
			fprintf(stderr, "[Error] Too many processes running (max %d)\n", PROCESS_LIMIT);
			return;
		}
	}
	
	// place process in process list
	procList[i] = procNum;
	
	// create an "address space" for the process at same index as process list
	int *temp = malloc(sizeof(int) * (addrSpaceSize+1));
	pageTables[i] = temp;
	
	// set address space size
	pageTables[i][0] = addrSpaceSize;
	
	// initialize valid bits to 0
	for(j = 1; j < addrSpaceSize+1; j++){
		pageTables[i][j] = 0;
	}

  return 0;
}

int terminate(int procNum)
{
  int i = 0, j = 0, valid = validateProc(procNum, 0);
	
  if(valid < 0) return 1;
	
	while(procList[i] != procNum){
		i++;
	}
	
	//get address space size
	int addrSpaceSize = pageTables[i][0];  
	
	for(j = 1; j < addrSpaceSize; j++){
		//update free pages
		if(freePages < totalPages && pageTables[i][j]){
			freePages++;
		}
	}
	
	printf("free frames: %d\n", freePages);
	free(pageTables[i]);
	
	return 0;
}

int reference(int procNum, int  vpn)
{
  int procListIndex = validateProc(procNum, 0);
	
  if(procListIndex <  0) return 1;
	
	int addrSpaceSize = pageTables[procListIndex][0];
	int inMemory = pageTables[procListIndex][vpn];
	
	//insertNode(procNum, vpn);
	
	// if( page number is out of bounds )
	if(vpn < 1 || vpn > addrSpaceSize){
		fprintf(stderr, "[Error] Page number out of bounds\n");
		fprintf(stderr, "        Please use a valid input file\n");
		exit(1);
	}
	
	// if( page is not in memory )
	if(!inMemory){
		// if( memory is not full )
		if(freePages > 0){
			pageTables[procListIndex][vpn] = 1;
			freePages--;
		}else{
			
		}
		faults++;
	}else{
		
	}
	
	// if( page is not in memory && memory is full )
	//if(pageTables[procListIndex][vpn] != 0x1 && freePages == 00){

		/*//evict 
	if(ghHead != NULL)
		{
			int evictProcNum = validateProc(ghHead->procNum, 't');
			int evictVPN = ghHead->vpn;
			pageTables[evictProcNum][evictVPN] = 0;
			printf("vicitm proc: %d page: %d\n", ghHead->procNum, ghHead->vpn);
			//if(ghHead->next != NULL)
			ghHead = ghHead -> next;
			freePages++;
			faults++;


			//put page in mem
			pageTables[procListIndex][vpn] = 0x1;
			freePages--; 
			}*/

	//}
	
	totalRef++;
	printf("free frames: %d\n", freePages);
	printf("faults total: %d  references total: %.0f fault rate: %.4f\n\n", faults, totalRef, faults/totalRef);
	
  return 0;
}


/*------Validation----*/
int validateProc(int procNum, int mode)
{
  int i;
	
	for(i = 0; i < PROCESS_LIMIT; i++){
		if(procList[i] == procNum){
			if(mode == 0) return i;
			else break;
		}
	}
	
	if(i < PROCESS_LIMIT){
		fprintf(stderr, "[Error] Cannot start pre-existing process\n");
		fprintf(stderr, "        Please use a valid input file\n");
	}else{
		if(mode == 0){
			fprintf(stderr, "[Error] Cannot reference/terminate nonexistent process\n");
			fprintf(stderr, "        Please use a valid input file\n");
		}else{
			return 0;
		}
	}
	return -1;
}

void insertNode(int procNum, int vpn)
{/*
  //Node *current = NULL;
  if(ghHead == NULL){
      
 			printf("first\n");
			printf("%d %d\n", ghHead->procNum, ghHead->vpn);
			return;
    }

		Node *bob = ghHead;
		while(bob != NULL){
			printf("-(%d|%d)-", bob->procNum, bob->vpn);
			bob = bob->next;
		}
		printf("\n");
		*/
		return;
}

int main(int argc, char** argv)
{
	if(argc != 3){
		fprintf(stderr, "[Error] Incorrect number of arguments\n");
		fprintf(stderr, "[Usage] %s <number-of-processes> <input-file>\n", argv[0]);
		return 1;
	}
	
	totalPages = atoi(argv[1]);
  freePages = totalPages;
  
  //start program with no processes running
  int i;
  for(i = 0; i < PROCESS_LIMIT; i++){
		procList[i] = 0;
	}
	
  FILE *input = fopen(argv[2], "r");
  char commandCheck[12];                                                                                                                              
  int procNum, pageNum, addrSpaceSize;
	
	// scan input and run stuff accordingly
	while(fscanf(input, "%s", commandCheck) != EOF){
		if(!strncmp(commandCheck, "START", 2)){
			fscanf(input, "%d %d\n", &procNum, &addrSpaceSize);
			printf("Starting process %d with address size %d\n",procNum, addrSpaceSize);
			start(procNum, addrSpaceSize);
		}else if(!strncmp(commandCheck, "TERMINATE", 2)){
			fscanf(input, "%d\n", &procNum);
			printf("terminating process %d\n", procNum);
			terminate(procNum);        
		}else if(!strncmp(commandCheck, "REFERENCE", 2)){
			fscanf(input, "%d %d\n", &procNum, &pageNum);
			printf("Referencing process %d's page %d\n",procNum, pageNum);
			reference(procNum, pageNum);
		}      
	}
  close(input);
  return 0;
}