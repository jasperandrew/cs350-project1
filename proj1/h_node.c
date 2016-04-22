#include "h_node.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

h_node makeNode(int procNum, int vpn) {
	h_node node = (h_node)malloc(sizeof(struct h_node_s));
	assert(node); // exit if we can't get space for a node
	node->procNum = procNum;
	node->vpn = 1;
	node->prev = NULL;
	node->next = NULL;
	return node;
}

void deleteNode(h_node node) {
	if (node->procNum) free(node->procNum);
	if (node->vpn) free(node->vpn);
	free(node);
}

int getProcNum(h_node node) {
	return node->procNum;
}

int getVPN(h_node node) {
	return node->vpn;
}

h_node getPrev(h_node node) {
	return node->prev;
}

h_node getNext(h_node node) {
	return node->next;
}

/* Function definitions for new h_node methods go here */