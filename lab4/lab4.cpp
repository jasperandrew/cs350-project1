#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <random>
#include <vector>

using namespace std;

class Process {
	public:
		Process(int n, int r, int s): pid(n), tot_refs(r), rem_refs(r), size(s) {}
		int getPID(){ return pid; }
		int getTotRefs(){ return tot_refs; }
		int getRemRefs(){ return rem_refs; }
		int getSize(){ return size; }
		double getPctDone(){ return ((tot_refs-rem_refs)/tot_refs)*100; }
		int ref(){
			rem_refs--;
			return rand() % size + 1;
		}
	private:
		int pid;
		int rem_refs;
		int tot_refs;
		int size;
};

int main(int argc, char **argv)
{
	unsigned int flag, tmp, num_procs = 1, min_size = 50, max_size = 150, quantum = 4, locality = 5, base_pid = rand() % 90000 + 10000;
	opterr = 0;
	while((flag = getopt (argc, argv, "un:l:R:")) != -1){
		switch(flag){
			case 'u':
				printf("Usage: lab4 [-u] [-n <num-procs>] [-s <min-size>] [-S <max-size>] [-q <references-per-quantum>] [-p <base-pid>] [-l <locality>]\n");
				return 0;
			case 'n':
				tmp = atoi(optarg);
				if(tmp >= 1 && tmp <= 1000){
					num_procs = tmp;
				}else{
					fprintf(stderr, "[Error] Number of processes must be between 1 and 1000.\n");
					return 1;
				}
				break;
			case 's':
				tmp = atoi(optarg);
				if(tmp > 0){
					min_size = tmp;
				}else{
					fprintf(stderr, "[Error] Minimum address space size must be a positive value.\n");
					return 1;
				}
				break;
			case 'S':
				tmp = atoi(optarg);
				if(tmp > 0){
					max_size = tmp;
				}else{
					fprintf(stderr, "[Error] Maximum address space size must be a positive value.\n");
					return 1;
				}
				break;
			case 'q':
				tmp = atoi(optarg);
				if(tmp > 0){
					quantum = tmp;
				}else{
					fprintf(stderr, "[Error] References per quantum must be a positive value.\n");
					return 1;
				}
				break;
			case 'p':
				tmp = atoi(optarg);
				if(tmp >= 0){
					base_pid = tmp;
				}else{
					fprintf(stderr, "[Error] Base PID must be greater than or equal to zero.\n");
					return 1;
				}
				break;
			case 'l':
				tmp = atoi(optarg);
				if(tmp >= 1 && tmp <= 10){
					locality = tmp;
				}else{
					fprintf(stderr, "[Error] Locality factor must be between 1 and 10.\n");
					return 1;
				}
				break;
			case '?':
				if (optopt == 'n' || optopt == 'l' || optopt == 'R')
					fprintf(stderr, "[Error] Option -%c requires an argument\n", optopt);
				else
					fprintf(stderr, "[Error] Unknown option character '%c'\n", optopt);
				fprintf(stderr, "        For usage info, run: ./lab4 -u\n");
				return 1;
			default:
				abort();
		}
		
		if(min_size > max_size){
			fprintf(stderr, "[Error] Minimum size must be less than or equal to maximum.\n");
			return 1;
		}
	}
	
	random_device rd;
	mt19937 rng(rd());
	uniform_int_distribution<int> uni(min_size, max_size);
	int size_rng = uni(rng);

	vector<Process> proc_list;
	
	for(int i = 0; i < num_procs; i++){
		int ref = rand() % 100 + 1; // temporary
		Process tmp(base_pid + i, ref, size_rng);
		proc_list.push_back(tmp);
	}
	
	freopen("out.txt", "w", stdout);
	
	while(proc_list.size() > 0){
		for(int i = 0; i < proc_list.size(); i++){
			if(proc_list[i].getTotRefs() == proc_list[i].getRemRefs())
				cout << "START " << proc_list[i].getPID() << " " << proc_list[i].getSize() << endl;
			for(int j = 0; j < quantum; j++){
				if(proc_list[i].getRemRefs() > 0){
					cout << "REFERENCE " << proc_list[i].getPID() << " " << proc_list[i].ref() << endl;
				}else{
					cout << "TERMINATE " << proc_list[i].getPID() << endl;
					proc_list.erase(proc_list.begin()+i);
					break;
				}
			}
		}
	}
	
	return 0;
}