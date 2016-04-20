#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <time.h>

using namespace std;

class Process {
	public:
		Process(int n, int r, int s): pid(n), tot_refs(r), rem_refs(r), size(s), min_ref(0), max_ref(s-1), phase(1) {}
		int getPID(){ return pid; }
		int getTotRefs(){ return tot_refs; }
		int getRemRefs(){ return rem_refs; }
		int getSize(){ return size; }
		int getPhase(){ return phase; }
		void incPhase(){ phase++; }
		double getPctDone(){ return ((tot_refs-rem_refs)/(double)tot_refs)*100; }
		void changeLocus(int l){
			if(l == 0){
				min_ref = 0;
				max_ref = size-1;
				return;
			}
			int locus = 0 + (rand() % (size));

			min_ref = (locus - l < 0 ? 0 : locus - l);
			max_ref = (locus + l > size-1 ? size-1 : locus + l);
		}
		int getFrame(){
			rem_refs--;
			return min_ref + (rand() % (max_ref - min_ref + 1));
		}
	private:
		int pid, rem_refs, tot_refs, size, min_ref, max_ref, phase;
};

int run(int num_procs, int min_size, int max_size, int min_refs, int max_refs, int quantum, int base_pid, int locality, int phases, char *out_file)
{
	vector<Process> proc_list;
	
	for(int i = 0; i < num_procs; i++){
		int size_rng = min_size + (rand() % (max_size - min_size + 1));
		int ref_rng = min_refs + (rand() % (max_refs - min_refs	+ 1));
		Process tmp(base_pid + i, ref_rng, size_rng);
		
		if(locality > 0){
			tmp.changeLocus(locality);
		}
		
		proc_list.push_back(tmp);
	}
	
	if(out_file != NULL){
		freopen(out_file, "w", stdout);
	}

	while(proc_list.size() > 0){
		for(int i = 0; i < proc_list.size(); i++){
			if(proc_list[i].getTotRefs() == proc_list[i].getRemRefs())
				cout << "START " << proc_list[i].getPID() << " " << proc_list[i].getSize() << endl;
			
			for(int j = 0; j < quantum; j++){
				if(proc_list[i].getRemRefs() > 0){
					if(proc_list[i].getPctDone() >= ((1.0/phases)*100) * proc_list[i].getPhase()){
						proc_list[i].changeLocus(locality);
						proc_list[i].incPhase();
					}
					
					cout << "REFERENCE " << proc_list[i].getPID() << " " << proc_list[i].getFrame() << endl;

				}else{
					cout << "TERMINATE " << proc_list[i].getPID() << endl;
					proc_list.erase(proc_list.begin()+i);
					i--;
					break;
				}
			}
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	int flag, tmp, num_procs = 2, min_size = 50, max_size = 150, min_refs = 10, max_refs = 100;
	int quantum = 4, base_pid = rand() % 100000, locality = 0, phases = 1;
	char *out_file = NULL;
	opterr = 0;
	while((flag = getopt (argc, argv, "un:s:S:r:R:q:i:l:p:o:")) != -1){
		switch(flag){
			case 'u':
				printf("Usage: %s [-u] [-n <num-procs>] [-s <min-size>] [-S <max-size>] [-r <min-refs>] [-R <max-refs>] [-q <refs-per-quantum>] [-p <base-pid>] [-l <locality>] [-o <output-file>]\n", argv[0]);
				printf("\n       u: Usage - prints this info (default values are in parentheses)");
				printf("\n       n: Number of processes (%d)", num_procs);
				printf("\n       s/S: Minimum/maximum address space size per process, in frames (%d/%d)", min_size, max_size);
				printf("\n       r/R: Minimum/maximum number of references per process (%d/%d)", min_refs, max_refs);
				printf("\n       q: Number of references per quantum of CPU time (%d)", quantum);
				printf("\n       i: Initial process ID value (random)");
				printf("\n       l: Locality - process will make references within a range of frames (none)");
				printf("\n       p: Phases - process will change its locality this many, minus one, times (%d)", phases);
				printf("\n       o: Output file path (STDOUT)\n");
				return 0;
			case 'n':
				tmp = atoi(optarg);
				if(tmp >= 1 && tmp <= 1000){
					num_procs = tmp;
				}else{
					fprintf(stderr, "[Error] Number of processes must be between 1 and 1000.\n");
					return 1;
				} break;
			case 's':
				tmp = atoi(optarg);
				if(tmp > 0){
					min_size = tmp;
				}else{
					fprintf(stderr, "[Error] Minimum address space size must be a positive value.\n");
					return 1;
				} break;
			case 'S':
				tmp = atoi(optarg);
				if(tmp > 0){
					max_size = tmp;
				}else{
					fprintf(stderr, "[Error] Maximum address space size must be a positive value.\n");
					return 1;
				} break;
			case 'r':
				tmp = atoi(optarg);
				if(tmp > 0){
					min_refs = tmp;
				}else{
					fprintf(stderr, "[Error] Minimum references per process must be a positive value.\n");
					return 1;
				} break;
			case 'R':
				tmp = atoi(optarg);
				if(tmp > 0){
					max_refs = tmp;
				}else{
					fprintf(stderr, "[Error] Maximum references per process must be a positive value.\n");
					return 1;
				} break;
			case 'q':
				tmp = atoi(optarg);
				if(tmp > 0){
					quantum = tmp;
				}else{
					fprintf(stderr, "[Error] References per quantum must be a positive value.\n");
					return 1;
				} break;
			case 'i':
				tmp = atoi(optarg);
				if(tmp >= 0){
					base_pid = tmp;
				}else{
					fprintf(stderr, "[Error] Base PID must be greater than or equal to zero.\n");
					return 1;
				} break;
			case 'l':
				tmp = atoi(optarg);
				if(tmp > 0){
					locality = tmp;
				}else{
					fprintf(stderr, "[Error] Locality must be a positive value.\n");
					return 1;
				} break;
			case 'p':
				tmp = atoi(optarg);
				if(tmp > 0){
					phases = tmp;
				}else{
					fprintf(stderr, "[Error] Number of phases must be a positive value.\n");
					return 1;
				} break;
			case 'o':
				out_file = optarg;
				break;
			case '?':
				if (optopt == 'n' || optopt == 's' || optopt == 'S' || optopt == 'r' || optopt == 'R' || optopt == 'q' || optopt == 'i' || optopt == 'l' || optopt == 'p' || optopt == 'o')
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
		if(min_refs > max_refs){
			fprintf(stderr, "[Error] Minimum references must be less than or equal to maximum.\n");
			return 1;
		}
	}
	
	if(run(num_procs, min_size, max_size, min_refs, max_refs, quantum, base_pid, locality, phases, out_file)) return 1;
	
	return 0;
}