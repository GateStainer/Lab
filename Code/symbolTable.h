#ifndef SYMBOLTABLE_H_
#define SYMBOLTABLE_H_

#include "type.h"

struct TableNode
{
	char name[32];
	TypeP type;
	struct TableNode* next;
};

typedef struct TableNode TableNode;

extern TableNode** symbolTable;


extern unsigned int hash(char* name);
extern void initSymbolTable();
extern TableNode* insertSymbolTable(char* name, TypeP type);
extern TableNode* searchSymbolTable(char* name);


#endif