#ifndef H_NODE_H
#define H_NODE_H

struct h_node_s {
	int procNum;
	int vpn;
	struct h_node_s *prev;
	struct h_node_s *next;
};

typedef struct h_node_s * h_node;

h_node makeNode(char *val);
void deleteNode(h_node node);

int getProcNum(h_node node);
int getVPN(h_node node);
h_node getPrev(h_node node);
h_node getNext(h_node node);

/* Function declarations for new h_node methods go here */

#endif