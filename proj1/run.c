// Jasper Andrew and Jacob Zwickler
#include "run.h"

int main(int argc, char** argv)
{
	if(argc != 5){
		fprintf(stderr, "[Error] Incorrect number of arguments\n");
		printUsage(argv[0]);
		exit(1);
	}
	
	totalPages = atoi(argv[1]);
	if(totalPages < 1){
		fprintf(stderr, "[Error] Number of pages in memory must be a positive value\n");
		printUsage(argv[0]);
		exit(1);
	}
  freePages = totalPages;

	policy = (strcmp(argv[3], "lru") == 0 ? 0 : (strcmp(argv[3], "rnd") == 0 ? 1 : (strcmp(argv[3], "nfu") == 0 ? 2 : -1 )));
	if(policy == -1){
		fprintf(stderr, "[Error] Unrecognized replacement policy option (%s)\n", argv[3]);
		printUsage(argv[0]);
		exit(1);
	}
	
	scope = (strcmp(argv[4], "global") == 0 ? 0 : (strcmp(argv[4], "local") == 0 ? 1 : -1 ));
	if(scope == -1){
		fprintf(stderr, "[Error] Unrecognized replacement type option (%s)\n", argv[4]);
		printUsage(argv[0]);
		exit(1);
	}
	int i;
	if(scope == 1){
		localHists = malloc(sizeof(h_node*) * numProcesses);
		for(i = 0; i < numProcesses; i++) localHists[i] = NULL;
	}
	
  FILE *input = fopen(argv[2], "r");
  char commandCheck[12];                                                                                                                              
	
	// Count the number of processes
	while(fscanf(input, "%s", commandCheck) != EOF){
		if(!strncmp(commandCheck, "START", 2)) numProcesses++;
	}
	
	// Allocate for number of processes
	procList = malloc(sizeof(int) * numProcesses);
	pageTables = malloc(sizeof(int*) * numProcesses);
	
	// Start program with no processes running
  for(i = 0; i < numProcesses; i++) procList[i] = 0;
	
	rewind(input);
	
	// Scan input and run stuff accordingly
	int procNum, pageNum, addrSpaceSize;
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

//---------------------------------------------------------------
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
	
	if(DBG) printf("Free pages: %d\n", freePages);
}

//---------------------------------------------------------------
void reference(int procNum, int  vpn)
{
  currProcIdx = getValidProcIndex(procNum, 0);

	// if( page number is out of bounds )
	if(vpn < 1 || vpn > getAddrSpaceSize(currProcIdx)){
		fprintf(stderr, "[Error] Page number out of bounds\n");
		fprintf(stderr, "        Please use a valid input file\n");
		cleanUp();
		exit(1);
	}
	
	// if( page is not in memory )
	if(!pageInMem(currProcIdx, vpn)){
		// if( memory is full )
		if(!freePages){
			// evict a page
			switch(policy){
				case 0: replaceLRU(); break;
				case 1: replaceRandom(); break;
				case 2: replaceNFU();
			}
		}
		
		storePage(currProcIdx, vpn);
		
		numFaults++;
	}
	
	updateHistory(procNum, vpn);
	
	numRefs++;
	if(DBG) printf("Free frames: %d\n", freePages);
	if(DBG) printf("Faults( %d ), References( %d ) -> Fault rate( %.4f )\n\n", numFaults, numRefs, (double)numFaults/numRefs);
}


/*-------------------------------------------------------------*\
|*                      Helper Functions                       *|
\*-------------------------------------------------------------*/
int getValidProcIndex(int procNum, int mode)
{
  int i;
	for(i = 0; i < numProcesses; i++){
		if(procList[i] == procNum){
			if(mode == 0) return i;
			else break;
		}
	}
	
	if(i < numProcesses){
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

//---------------------------------------------------------------
int currProcCount()
{
	int i = 0;
	while(procList[i] != 0){
		if(i < numProcesses){
			i++;
		}else{
			fprintf(stderr, "[Error] Too many processes running (max %d)\n", numProcesses);
			cleanUp();
			exit(1);
		}
	}
	return i;
}

//---------------------------------------------------------------
void cleanUp()
{
	// free procList
	free(procList);
	
	// free pageTables
	int i;
	for(i = 0; i < numProcesses; i++) free(pageTables[i]);
	free(pageTables);
	
	// free history nodes
	h_node *itr = globalHist, *curr;
	while((curr = itr) != NULL){
		itr = itr->next;
		free(curr);
	}
	
	// free local history nodes
	if(localHists != NULL){
		for(i = 0; i < numProcesses; i++){
			itr = localHists[i];
			while((curr = itr) != NULL){
				itr = itr->next;
				if(curr != NULL) free(curr);
			}
		}
		free(localHists);
	}
}

//---------------------------------------------------------------
void printUsage(char *arg){
	fprintf(stderr, "[Usage] %s <pages-in-memory> <input-file> <policy> <scope>\n", arg);
	fprintf(stderr, "\n        Note: <policy> can be:\n");
	fprintf(stderr, "                - lru (Least Recently Used)\n");
	fprintf(stderr, "                - rnd (Random)\n");
	fprintf(stderr, "                - nfu (Not Frequently Used)\n");
	fprintf(stderr, "              <scope> can be:\n");
	fprintf(stderr, "                - global (Global Replacement)\n");
	fprintf(stderr, "                - local (Local Replacement)\n");
}


/*-------------------------------------------------------------*\
|*                     History Functions                       *|
\*-------------------------------------------------------------*/
void updateHistory(int procNum, int vpn)
{
	h_node *hist = ( scope == 0 ? globalHist : localHists[currProcIdx] );
	
  if(hist == NULL){
		hist = malloc(sizeof(h_node));
		hist->procNum = procNum;
		hist->vpn = vpn;
		hist->ctr = 0;
		hist->prev = NULL;
		hist->next = NULL;
		if(DBG) printf("+-[%d|%d](%d)-\n", hist->procNum, hist->vpn, hist->ctr);
		
		if(scope == 0) globalHist = hist;
		else localHists[currProcIdx] = hist;
		return;
	}

	h_node *itr = hist, *curr = NULL;
	while(itr->next != NULL){
		if(itr->procNum == procNum && itr->vpn == vpn){
			curr = itr;
			if(itr->prev != NULL) itr->prev->next = itr->next;
			else hist = itr->next;
			
			itr->next->prev = itr->prev;
		}
		itr = itr->next;
		if(itr->procNum == procNum && itr->vpn == vpn) curr = itr;
	}
	
	if(curr == NULL){
		h_node *newNode = malloc(sizeof(h_node));
		newNode->procNum = procNum;
		newNode->vpn = vpn;
		newNode->ctr = 0;
		newNode->prev = itr;
		newNode->next = NULL;
		itr->next = newNode;
	}else{
		if(curr->next != NULL){
			itr->next = curr;
			curr->prev = itr;
			curr->next = NULL;
		}
		curr->ctr++;
	}
	
	if(DBG){
		h_node *bob = hist;
		while(bob != NULL){
			printf("-[%d|%d](%d)-", bob->procNum, bob->vpn, bob->ctr);
			bob = bob->next;
		}
		printf("\n");
	}
	
	if(scope == 0) globalHist = hist;
	else localHists[currProcIdx] = hist;
}

//---------------------------------------------------------------
int getHistLength()
{
	h_node *hist = ( scope == 0 ? globalHist : localHists[currProcIdx] );

	int n = 0;
	h_node *itr = globalHist;
	while(itr != NULL){
		n++;
		itr = itr->next;
	}
	return n;
}


/*-------------------------------------------------------------*\
|*                   Replacement Policies                      *|
\*-------------------------------------------------------------*/
void replaceLRU() // Least Recently Used Replacement
{
	h_node *hist = ( scope == 0 ? globalHist : localHists[currProcIdx] );

	if(DBG) printf("evicted LRU: -[%d|%d](%d)-\n", hist->procNum, hist->vpn, hist->ctr);
	deletePage(getValidProcIndex(hist->procNum, 0), hist->vpn);
	
	h_node *tmp = hist;
	if(hist->next != NULL){
		hist->next->prev = NULL;
		hist = hist->next;
	}

	free(tmp);
	
	if(scope == 0) globalHist = hist;
	else localHists[currProcIdx] = hist;
}

//---------------------------------------------------------------
void replaceRandom() // Random Replacement
{
	h_node *hist = ( scope == 0 ? globalHist : localHists[currProcIdx] );
	
	srand(time(NULL));
	int randomIdx = rand() % getHistLength();
	
	h_node *itr = hist;
	while(randomIdx > 0){
		randomIdx--;
		itr = itr->next;
	}
	if(DBG) printf("evicted random: -[%d|%d](%d)-\n", itr->procNum, itr->vpn, itr->ctr);
	deletePage(getValidProcIndex(itr->procNum, 0), itr->vpn);
	
	if(itr->prev != NULL) itr->prev->next = itr->next;
	else hist = itr->next;
	
	if(itr->next != NULL) itr->next->prev = itr->prev;

	free(itr);

	if(scope == 0) globalHist = hist;
	else localHists[currProcIdx] = hist;
}

//---------------------------------------------------------------
void replaceNFU() // Not Frequently Used Replacement
{
	h_node *hist = ( scope == 0 ? globalHist : localHists[currProcIdx] );
	
	int min = -1;
	h_node *itr = hist, *minNode;
	while(itr != NULL){
		if(itr->ctr < min || min == -1){
			min = itr->ctr;
			minNode = itr;
		}
		itr = itr->next;
	}
	
	if(DBG) printf("evicted NFU: -[%d|%d](%d)-\n", minNode->procNum, minNode->vpn, minNode->ctr);
	deletePage(getValidProcIndex(minNode->procNum, 0), minNode->vpn);
	
	if(minNode->prev != NULL) minNode->prev->next = minNode->next;
	else hist = minNode->next;
	
	if(minNode->next != NULL) minNode->next->prev = minNode->prev;

	free(minNode);	

	if(scope == 0) globalHist = hist;
	else localHists[currProcIdx] = hist;
}