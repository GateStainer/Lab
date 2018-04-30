#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbolTable.h"


TableNode** symbolTable;

unsigned int hash(char* name)
{
	unsigned int val = 0, i;
	for(; *name; ++name)
	{
		val = (val << 2) + *name;
		if(i = val & ~0x3fff) val = (val ^ (i >> 12)) & 0x3fff;
	}
	return val;
}

void initSymbolTable()
{
	symbolTable = (TableNode**)malloc(sizeof(TableNode*)*0x3fff);
	memset(symbolTable, 0, sizeof(TableNode*)*0x3fff);
}

TableNode* insertSymbolTable(char* name, TypeP type)
{
	TableNode* node = (TableNode*)malloc(sizeof(TableNode));
	strcpy(node->name, name);
	node->type = type;
	unsigned int hash_num = hash(name);
	TableNode* nodePoint = symbolTable[hash_num];
	for(; nodePoint!=NULL; nodePoint=nodePoint->next)
	{
		if(strcmp(nodePoint->name, name) == 0)
			break;
	}
	if(nodePoint != NULL)
		return NULL;
	node->next = symbolTable[hash_num];
	symbolTable[hash_num] = node;
	return symbolTable[hash_num];
}

TableNode* searchSymbolTable(char* name)
{
	unsigned int hash_num = hash(name);
	TableNode* point = symbolTable[hash_num];
	while(point != NULL)
	{
		if(strcmp(point->name, name) == 0)
			return point;
		point = point->next;
	}
	return point;
}
















