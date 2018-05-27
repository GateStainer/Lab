#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbolTable.h"
#include "type.h"
#include "semantic.h"


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

	TypeP p1 = (TypeP)malloc(sizeof(Type_));
	p1->kind = FUNCTION;
	FieldListP q1 = (FieldListP)malloc(sizeof(FieldList_));
	q1->name = "read";
	q1->type = (TypeP)malloc(sizeof(Type_));
	q1->type->kind = BASIC;
	q1->type->u.basic = INT;
	q1->tail = NULL;
	p1->u.structure = q1;
	insertSymbolTable("read", p1);

	TypeP p2 = (TypeP)malloc(sizeof(Type_));
	p2->kind = FUNCTION;
	FieldListP q2 = (FieldListP)malloc(sizeof(FieldList_));
	q2->name = "write";
	q2->type = (TypeP)malloc(sizeof(Type_));
	q2->type->kind = BASIC;
	q2->type->u.basic = INT;
	FieldListP q3 = (FieldListP)malloc(sizeof(FieldList_));
	q3->name = NULL;
	q3->type = (TypeP)malloc(sizeof(Type_));
	q3->type->kind = BASIC;
	q3->type->u.basic = INT;
	q3->tail = NULL;
	q2->tail = q3;
	p2->u.structure = q2;
	insertSymbolTable("write", p2);
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
















