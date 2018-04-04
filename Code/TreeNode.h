#ifndef TREENODE_H_
#define TREENODE_H_

#include "stdbool.h"
//Define syntax tree
struct TreeNode
{
	char name[32];	//name of the node, like ELSE, TYPE, PROGRAM, etc.
	int lineno;
	char data[32];	//attribute data
	bool terminal;	//whether a terminal or non-terminal
	bool visited;
	struct TreeNode *firstChild;
	struct TreeNode *next;
};

typedef struct TreeNode TreeNode;
extern TreeNode* root;
extern TreeNode* createTreeNode(const char* name, int lineno, bool terminal);
extern void insertTreeNode(TreeNode* parent, int count, ...);
extern void printTree(TreeNode* root, int tabNum);
extern TreeNode *root;

#endif