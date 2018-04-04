%{
	#include "stdio.h"
	#include "string.h"
	#include "stdbool.h"
	#include "stdarg.h"
	#include "stdlib.h"

	#include "TreeNode.h"
	#include "lex.yy.c"

	extern int mylineno;
	TreeNode *root;
	bool error_flag = false;
	TreeNode* createTreeNode(const char* name, int lineno, bool terminal)
	{
		TreeNode* node = (TreeNode *)malloc(sizeof(TreeNode));
		if(node==NULL)
		{
			printf("Create TreeNode Failed!\n");
			exit(-1);
		}
		strcpy(node->name, name);
		node->lineno = lineno;
		node->terminal = terminal;
		node->firstChild = NULL;
		node->next = NULL;
		return node;
	}

	void insertTreeNode(TreeNode* parent, int count, ...)
	{
		va_list arg;
		int i = 0;
		va_start(arg, count);
		TreeNode *present = NULL;
		if(parent->firstChild == NULL)
		{
			present = va_arg(arg, TreeNode*);
			parent->firstChild = present;
			i++;
		}
		TreeNode *sons = parent->firstChild;
		for(; sons->next != NULL; sons = sons->next);
		for(; i<count; i++)
		{
			present = va_arg(arg, TreeNode*);	
			sons->next = present;
			sons = sons->next;	
		}
		va_end(arg);
		return;
	}

	void printTree(TreeNode* root, int tabNum)
	{
		if(root == NULL || strcmp(root->name, "Empty")==0)
			return;
		int i = 0;
		if(strcmp(root->name, "Program")!=0)
			for(; i<tabNum-1; i++)
				printf("  ");
		if(root->terminal == false)
		{
			printf("%s (%d)\n", root->name, root->lineno);
		}
		else
		{
			if(strcmp(root->name, "ID")==0 || strcmp(root->name, "TYPE")==0 || strcmp(root->name, "INT")==0)
			{
				printf("%s: %s\n", root->name, root->data);
			}
			else if(strcmp(root->name, "FLOAT")==0)
			{
				printf("%s: %f\n", root->name, atof(root->data));
			}
			else
			{
				printf("%s\n", root->name);
			}
		}
		TreeNode *child = root->firstChild;
		for(; child != NULL; child = child->next)
			printTree(child, tabNum+1);
		return;
	}
%}

%locations
%union
{
	struct TreeNode* node;
}

/*declared tokens */
%token <node> SEMI COMMA ASSIGNOP PLUS MINUS STAR DIV RELOP
%token <node> AND OR NOT 
%token <node> LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE
%token <node> INT FLOAT
%token <node> TYPE DOT ID

/*define priority */
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LB RB LP RP DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%nonassoc SEMI COMMA STRUCT RETURN IF WHILE


/*declared non-terminals */
%type <node> Program ExtDefList ExtDef ExtDecList
%type <node> Specifier StructSpecifier OptTag Tag
%type <node> VarDec FunDec VarList ParamDec
%type <node> CompSt StmtList Stmt
%type <node> DefList Def DecList Dec
%type <node> Exp Args 

%%
/*High-level Definitions */
Program : ExtDefList {
	$$ = createTreeNode("Program", @$.first_line, false);
	insertTreeNode($$, 1, $1);
	root = $$;
};

ExtDefList: ExtDef  ExtDefList {
	$$ = createTreeNode("ExtDefList", @$.first_line, false);
	insertTreeNode($$, 2, $1, $2);
}
|  {
	$$ = createTreeNode("Empty", @$.first_line, false);
};

ExtDef : Specifier ExtDecList SEMI {
	$$ = createTreeNode("ExtDef", @$.first_line, false);
	insertTreeNode($$, 3, $1, $2, $3);
}
| Specifier SEMI {
	$$ = createTreeNode("ExtDef", @$.first_line, false);
	insertTreeNode($$, 2, $1, $2);
}
| Specifier FunDec CompSt {
	$$ = createTreeNode("ExtDef", @$.first_line, false);
	insertTreeNode($$, 3, $1, $2, $3);
}
| Specifier FunDec SEMI {
	$$ = createTreeNode("ExtDef", @$.first_line, false);
	insertTreeNode($$, 3, $1, $2, $3);
}
| error SEMI {
	;
};

ExtDecList : VarDec {
	$$ = createTreeNode("ExtDecList", @$.first_line, false);
	insertTreeNode($$, 1, $1);
}
| VarDec COMMA ExtDecList {
	$$ = createTreeNode("ExtDecList", @$.first_line, false);
	insertTreeNode($$, 3, $1, $2, $3);
};

/*Specifiers */
Specifier : TYPE {
	$$ = createTreeNode("Specifier", @$.first_line, false);
	insertTreeNode($$, 1, $1);
}
| StructSpecifier {
	$$ = createTreeNode("Specifier", @$.first_line, false);
	insertTreeNode($$, 1, $1);
};

StructSpecifier : STRUCT OptTag LC DefList RC {
	$$ = createTreeNode("StructSpecifier", @$.first_line, false);
	insertTreeNode($$, 5, $1, $2, $3, $4, $5);
}
| STRUCT Tag {
	$$ = createTreeNode("StructSpecifier", @$.first_line, false);
	insertTreeNode($$, 2, $1, $2);
};

OptTag : ID {
	$$ = createTreeNode("OptTag", @$.first_line, false);
	insertTreeNode($$, 1, $1);
}
|  {
	$$ = createTreeNode("Empty", @$.first_line, false);
};

Tag : ID {
	$$ = createTreeNode("Tag", @$.first_line, false);
	insertTreeNode($$, 1, $1);
};

/* Declarators */
VarDec : ID {
	$$ = createTreeNode("VarDec", @$.first_line, false);
	insertTreeNode($$, 1, $1);
}
| VarDec LB INT RB {
	$$ = createTreeNode("VarDec", @$.first_line, false);
	insertTreeNode($$, 4, $1, $2, $3, $4);
};

FunDec : ID LP VarList RP {
	$$ = createTreeNode("FunDec", @$.first_line, false);
	insertTreeNode($$, 4, $1, $2, $3, $4);
}
| ID LP RP {
	$$ = createTreeNode("FunDec", @$.first_line, false);
	insertTreeNode($$, 3, $1, $2, $3);
}
| error RP {
	;
};

VarList : ParamDec COMMA VarList {
	$$ = createTreeNode("VarList", @$.first_line, false);
	insertTreeNode($$, 3, $1, $2, $3);
}
| ParamDec {
	$$ = createTreeNode("VarList", @$.first_line, false);
	insertTreeNode($$, 1, $1);
};

ParamDec : Specifier VarDec {
	$$ = createTreeNode("VarList", @$.first_line, false);
	insertTreeNode($$, 2, $1, $2);
}
| error COMMA {
	;
}
| error RB {
	;
};

/*Statements */
CompSt : LC DefList StmtList RC {
	$$ = createTreeNode("CompSt", @$.first_line, false);
	insertTreeNode($$, 4, $1, $2, $3, $4);
}
| error RC {
	;
};

StmtList : Stmt StmtList {
	$$ = createTreeNode("StmtList", @$.first_line, false);
	insertTreeNode($$, 2, $1, $2);
}
|  {
	$$ = createTreeNode("Empty", @$.first_line, false);
};

Stmt : Exp SEMI {
	$$ = createTreeNode("Stmt", @$.first_line, false);
	insertTreeNode($$, 2, $1, $2);
}
| CompSt {
	$$ = createTreeNode("Stmt", @$.first_line, false);
	insertTreeNode($$, 1, $1);
}
| RETURN Exp SEMI {
	$$ = createTreeNode("Stmt", @$.first_line, false);
	insertTreeNode($$, 3, $1, $2, $3);
}
| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
	$$ = createTreeNode("Stmt", @$.first_line, false);
	insertTreeNode($$, 5, $1, $2, $3, $4, $5);
}
| IF LP Exp RP Stmt ELSE Stmt {
	$$ = createTreeNode("Stmt", @$.first_line, false);
	insertTreeNode($$, 7, $1, $2, $3, $4, $5, $6, $7);
}
| WHILE LP Exp RP Stmt {
	$$ = createTreeNode("Stmt", @$.first_line, false);
	insertTreeNode($$, 5, $1, $2, $3, $4, $5);
}
| error SEMI {
	;
};






/* Local Definitions */
DefList: Def DefList {
	$$ = createTreeNode("DefList", @$.first_line, false);
	insertTreeNode($$,2,$1,$2);
}
|  {
	$$ = createTreeNode("Empty", @$.first_line, false);
};

Def: Specifier DecList SEMI {
	$$ = createTreeNode("Def", @$.first_line, false);
	insertTreeNode($$,3,$1,$2,$3);
}
| error SEMI {
 	;
};

DecList: Dec {
	$$ = createTreeNode("DecList", @$.first_line, false);
	insertTreeNode($$,1,$1);
}
| Dec COMMA DecList {
	$$ = createTreeNode("DecList", @$.first_line, false);
	insertTreeNode($$,3,$1,$2,$3);
};

Dec: VarDec {
	$$ = createTreeNode("Dec", @$.first_line, false);
	insertTreeNode($$,1,$1);
}
| VarDec ASSIGNOP Exp {
	$$ = createTreeNode("Dec", @$.first_line, false);
	insertTreeNode($$,3,$1,$2,$3);
	}
;

/* Expressions */
Exp: Exp ASSIGNOP Exp {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,3,$1,$2,$3);
}
| Exp AND Exp {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,3,$1,$2,$3);
}
| Exp OR Exp {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,3,$1,$2,$3);
}
| Exp RELOP Exp {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,3,$1,$2,$3);
}
| Exp PLUS Exp {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,3,$1,$2,$3);
}
| Exp MINUS Exp {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,3,$1,$2,$3);
}
| Exp STAR Exp {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,3,$1,$2,$3);
}
| Exp DIV Exp {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,3,$1,$2,$3);
}
| LP Exp RP {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,3,$1,$2,$3);
}
| MINUS Exp {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,2,$1,$2);
}
| NOT Exp {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,2,$1,$2);
}
| ID LP Args RP {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,4,$1,$2,$3,$4);
}
| ID LP RP {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,3,$1,$2,$3);
}
| Exp LB Exp RB {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,4,$1,$2,$3,$4);
}
| Exp DOT ID {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,3,$1,$2,$3);
}
| ID {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,1,$1);
}
| INT {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,1,$1);
}
| FLOAT {
	$$ = createTreeNode("Exp", @$.first_line, false);
	insertTreeNode($$,1,$1);
}
| LP Exp error {
	//yyerror("Missing )");
}
| ID LP Args error {
	//yyerror("Missing )");
}
| ID LP error {
	//yyerror("Missing )");
}
| Exp LB Exp error {
	//yyerror("Missing ]");
}
;

Args: Exp COMMA Args {
	$$ = createTreeNode("Args", @$.first_line, false);
	insertTreeNode($$,3,$1,$2,$3);
}
| Exp {
	$$ = createTreeNode("Args", @$.first_line, false);
	insertTreeNode($$,1,$1);
};


%%

yyerror(char *msg)
{
	error_flag = true;
	fprintf(stderr, "Error type B at line %d:\'%s\'\n", mylineno, msg);
}