#include "stdio.h"
#include "TreeNode.h"
#include "symbolTable.h"
#include "semantic.h"
#include "ir_generator.h"

extern FILE* yyin;
extern void yyrestart(FILE *input);
extern int yyparse();
extern bool error_flag;
extern int mylineno;

int main(int argc, char** argv)
{
	if(argc <= 1) return 1;
	FILE* f = fopen(argv[1], "r");
	if(!f)
	{
		perror(argv[1]);
		return 1;
	}
	yyrestart(f);
	mylineno = 1;
	yyparse();
	if(!error_flag)
	{
		//printTree(root, 2);
		initSymbolTable();
		semanticTest(root);
		//printf("Begin ir generation\n");
		if(argc >= 3)
			generate_ir(argv[2]);

	}
	return 0;
}