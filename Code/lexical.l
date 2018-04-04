%{
	#include "syntax.tab.h"
	#include "TreeNode.h"
	/*definitions of manifest constants
	SEMI, COMMA, ASSIGNOP, PLUS, MINUS, STAR, DIV,RELOP,
	AND, OR, NOT, LP, RP, LB, RB, LC, RC, STRUCT, RETURN,
	IF, ELSE, WHILE, INT, FLOAT, TYPE, DOT, ID
	*/
	int mylineno = 1;
	extern bool error_flag;
	int yycolumn = 1;
	#define YY_USER_ACTION \
			yylloc.first_line = yylloc.last_line = mylineno; \
			yylloc.first_column = yycolumn; \
			yylloc.last_column = yycolumn + yyleng - 1; \
			yycolumn += yyleng;
	#define DEBUG 0
%}

/* regular definitions */
%option noyywrap
delim	[ \t\n]
ws	{delim}+
letter	[_A-Za-z]
digit	[0-9]
INT	([1-9]{digit}*)|[0]
FLOAT	({INT}\.{digit}+)|((({digit}+\.)|(\.{digit}+)|({digit}+\.{digit}+))[Ee][\+-]?[0-9]+) 
RELOP	>|<|(<=)|(>=)|(==)|(!=)
ID	{letter}({letter}|{digit})*

%%
[ \t\r]	{}
[\n]	{ mylineno++ ; yycolumn = 1; }	
";"	{ //printf("SEMI\n");
	 yylval.node = createTreeNode("SEMI", mylineno, true);
	 return SEMI;
	 }
","	{ //printf("COMMA\n");
	 yylval.node = createTreeNode("COMMA", mylineno, true);
	 return COMMA;
	 }
"="	{ //printf("ASSIGNOP\n");
 	yylval.node = createTreeNode("ASSIGNOP", mylineno, true);
	return ASSIGNOP;
	 }
"+"	{ //printf("PLUS\n");
	 yylval.node = createTreeNode("PLUS", mylineno, true);
	 return PLUS;
	 }
"-"	{ //printf("MINUS\n");
	 yylval.node = createTreeNode("MINUS", mylineno, true);
	 return MINUS;
	 }
"*"	{ //printf("STAR\n");
	 yylval.node = createTreeNode("STAR", mylineno, true);
	 return STAR;
	 }
"/"	{ //printf("DIV\n");
	 yylval.node = createTreeNode("DIV", mylineno, true);
	 return DIV;
	 }
{RELOP}	{ //printf("RELOP\n");
	 yylval.node = createTreeNode("RELOP", mylineno, true);
	 strcpy(yylval.node->data, yytext);
	 return RELOP;
	 }
"&&"	{ //printf("AND\n");
	 yylval.node = createTreeNode("AND", mylineno, true);
	 return AND;
	 }
"||"	{ //printf("OR\n");
	 yylval.node = createTreeNode("OR", mylineno, true);
	 return OR;
	 }
"!"	{ //printf("NOT\n");
	 yylval.node = createTreeNode("NOT", mylineno, true);
	 return NOT;
	 }
"("	{ //printf("LP\n");
	 yylval.node = createTreeNode("LP", mylineno, true);
	 return LP;
	 }
")"	{ //printf("RP\n");
	 yylval.node = createTreeNode("RP", mylineno, true);
	 return RP;
	 }
"["	{ //printf("LB\n");
	 yylval.node = createTreeNode("LB", mylineno, true);
	 return LB;
	 }
"]"	{ //printf("RB\n");
	 yylval.node = createTreeNode("RB", mylineno, true);
	 return RB;
	 }
"{"	{ //printf("LC\n");
	 yylval.node = createTreeNode("LC", mylineno, true);
	 return LC;
	 }
"}"	{ //printf("RC\n");
	 yylval.node = createTreeNode("RC", mylineno, true);
	 return RC;
	 }
"struct"	{ //printf("STRUCT\n");
		 yylval.node = createTreeNode("STRUCT", mylineno, true);
		 return STRUCT;
		 }
"return"	{ //printf("RETURN\n");
		 yylval.node = createTreeNode("RETURN", mylineno, true);
		 return RETURN;
		 }
"if"	{ //printf("IF\n");
	 yylval.node = createTreeNode("IF", mylineno, true);
	 return IF;
	 }
"else"	{ //printf("ELSE\n");
	 yylval.node = createTreeNode("ELSE", mylineno, true);
	 return ELSE;
	 }
"while"	{ //printf("WHILE\n");
	 yylval.node = createTreeNode("WHILE", mylineno, true);
	 return WHILE;
	 }
{INT}	{ //printf("INT\n"); 
	 yylval.node = createTreeNode("INT", mylineno, true);
	 strcpy(yylval.node->data, yytext);
	 return INT;
	 }
{FLOAT}	{ //printf("FLOAT\n");
 	 yylval.node = createTreeNode("FLOAT", mylineno, true);
 	 strcpy(yylval.node->data, yytext);
	 return FLOAT;
	 }
"int"	{ //printf("TYPE\n");
	 yylval.node = createTreeNode("TYPE", mylineno, true);
	 strcpy(yylval.node->data, "INT");
	 return TYPE;
	 }
"float"	{ //printf("TYPE\n");
	 yylval.node = createTreeNode("TYPE", mylineno, true);
	 strcpy(yylval.node->data, "FLOAT");
	 return TYPE;
	 }
"."	{ //printf("DOT\n");
	 yylval.node = createTreeNode("DOT", mylineno, true);
	 return DOT;
	 }
{ID}	{ //printf("ID\n");
	 yylval.node = createTreeNode("ID", mylineno, true);
	 strcpy(yylval.node->data, yytext);
	 return ID;
	 }
.	{ error_flag = true; printf("ERROR type A at Line %d: Mysterious characters \'%s\'\n", mylineno, yytext); }

%%