#include "proj1.h"

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
			if(DBG) printf("Starting process %d with address size %d\n",procNum, addrSpaceSize);
			start(procNum, addrSpaceSize);
		}else if(!strncmp(commandCheck, "TERMINATE", 2)){
			fscanf(input, "%d\n", &procNum);
			if(DBG) printf("Terminating process %d\n", procNum);
			terminate(procNum);        
		}else if(!strncmp(commandCheck, "REFERENCE", 2)){
			fscanf(input, "%d %d\n", &procNum, &pageNum);
			if(DBG) printf("Referencing process %d's page %d\n",procNum, pageNum);
			reference(procNum, pageNum);
		}      
	}
	
  close(input);
	cleanUp();
	
	printf("--- Statistics ---\n");
	printf("References: %d\nFaults: %d\nFault Rate: %.4f\n", numRefs, numFaults, (double)numFaults/numRefs);
  
	return 0;
}

/*-------------------------------------------------------------*\
|*                   	Processing Functions                     *|
\*-------------------------------------------------------------*/
void start(int procNum, int addrSpaceSize)
{
  int i = 0, j;
  
  //search if process already running 
  int valid = getValidProcIndex(procNum, 1);
	
	// check if limit for processes reached - 
	i = currProcCount();
	
	// place process in process list
	procList[i] = procNum;
	
	// create an "address space" for the process at same index as process list
	pageTables[i] = malloc(sizeof(int) * (addrSpaceSize + 1));
	
	// set address space size
	pageTables[i][0] = addrSpaceSize;
	
	// initialize valid bits to 0
	for(j = 1; j < addrSpaceSize + 1; j++){
		pageTables[i][j] = 0;
	}
}

void terminate(int procNum)
{
  int i, procListIdx = getValidProcIndex(procNum, 0);
	
	//get address space size
	int size = getAddrSpaceSize(procListIdx);  
	
	for(i = 1; i < size; i++){
		//update free pages
		if(freePages < totalPages && pageInMem(procListIdx, i)){
			deletePage(procListIdx, i);
		}
	}
	
	if(DBG) printf("free frames: %d\n", freePages);
}

void reference(int procNum, int  vpn)
{
  int procListIdx = getValidProcIndex(procNum, 0);

	// if( page number is out of bounds )
	if(vpn < 1 || vpn > getAddrSpaceSize(procListIdx)){
		fprintf(stderr, "[Error] Page number out of bounds\n");
		fprintf(stderr, "        Please use a valid input file\n");
		cleanUp();
		exit(1);
	}
		
	// if( page is not in memory )
	if(!pageInMem(procListIdx, vpn)){
		// if( memory is full )
		if(!freePages){
			// evict a page
			leastRecentlyUsed();
		}
		
		storePage(procListIdx, vpn);
		
		numFaults++;
	}
	
	updateHistory(procNum, vpn);
	
	numRefs++;
	if(DBG) printf("free frames: %d\n", freePages);
	if(DBG) printf("faults( %d ), references( %d ) -> fault rate( %.4f )\n\n", numFaults, numRefs, (double)numFaults/numRefs);
}

/*-------------------------------------------------------------*\
|*                      Helper Functions                       *|
\*-------------------------------------------------------------*/
int getValidProcIndex(int procNum, int mode)
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
	cleanUp();
	exit(1);
}

int currProcCount()
{
	int i = 0;
	while(procList[i] != 0){
		if(i < PROCESS_LIMIT){
			i++;
		}else{
			fprintf(stderr, "[Error] Too many processes running (max %d)\n", PROCESS_LIMIT);
			cleanUp();
			exit(1);
		}
	}
	return i;
}

void cleanUp()
{
	// free history nodes
	h_node *itr = globalHist, *curr;
	while((curr = itr) != NULL){
		itr = itr->next;
		free(curr);
	}
}

/*-------------------------------------------------------------*\
|*                     History Functions                       *|
\*-------------------------------------------------------------*/
void updateHistory(int procNum, int vpn)
{
  if(globalHist == NULL){
		globalHist = malloc(sizeof(h_node));
		globalHist->procNum = procNum;
		globalHist->vpn = vpn;
		globalHist->prev = NULL;
		globalHist->next = NULL;
		if(DBG) printf("-[%d|%d]-\n", globalHist->procNum, globalHist->vpn);
		return;
	}
	
	h_node *itr = globalHist, *curr;
	while((curr = itr)->next != NULL){
		int freeNode = 0;
		if(itr->procNum == procNum && itr->vpn == vpn){
			if(itr->prev != NULL) itr->prev->next = itr->next;
			else globalHist = itr->next;
			
			if(itr->next != NULL) itr->next->prev = itr->prev;
			else return;
			
			freeNode = 1;
		}
		itr = itr->next;
		if(freeNode) free(curr);
	}
	
	h_node *newNode = malloc(sizeof(h_node));
	newNode->procNum = procNum;
	newNode->vpn = vpn;
	newNode->prev = itr;
	newNode->next = NULL;
	itr->next = newNode;

	if(DBG){
		h_node *bob = globalHist;
		while(bob != NULL){
			printf("-[%d|%d]-", bob->procNum, bob->vpn);
			bob = bob->next;
		}
		printf("\n");
	}
	
	return;
}

/*-------------------------------------------------------------*\
|*                   Replacement Policies                      *|
\*-------------------------------------------------------------*/
void leastRecentlyUsed()
{
	if(DBG) printf("evicted: -[%d|%d]-", globalHist->procNum, globalHist->vpn);
	deletePage(getValidProcIndex(globalHist->procNum, 0), globalHist->vpn);
	
	h_node *tmp = globalHist;
	if(globalHist->next != NULL){
		globalHist->next->prev = NULL;
		globalHist = globalHist->next;
	}

	free(tmp);
}