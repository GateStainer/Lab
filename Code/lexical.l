%{
//#include "syntax.tab.h"
%}

/* regular definitions */
%option noyywrap
%option yylineno
delim	[ \t\n]
ws	{delim}+
letter	[_A-Za-z]
digit	[0-9]
INT	([1-9]{digit}*)|[0]
FLOAT	{INT}\.{digit}+
EFLOAT	(({digit}+\.)|(\.{digit}+)|({digit}+\.{digit}+))[Ee][\+-]?[0-9]+
ID	{letter}({letter}|{digit})*

%%
[ \t\r]	{}
\n	{ yylineno++; }
";"	{ printf("SEMI\n");
	 //return SEMI;
	 }
","	{ printf("COMMA\n");
	 //return COMMA;
	 }
"="	{ printf("ASSIGNOP\n");
	//return ASSIGNOP;
	 }
"+"	{ printf("PLUS\n");
	 //return PLUS;
	 }
"-"	{ printf("MINUS\n");
	 //return MINUS;
	 }
"*"	{ printf("STAR\n");
	//return STAR;
	 }
"/"	{ printf("DIV\n");
	//return DIV;
	 }
"<"	{ printf("RELOP\n");
	//yylval = LT; return RELOP;
	 }
"<="	{ printf("Relop\n");
	//yylval = LE; return RELOP;
	 }
"=="	{ printf("RELOP\n");
	//yylval = EQ; return RELOP;
	 }
">"	{ printf("RELOP\n");
	//yylval = GT; return RELOP;
	 }
">="	{ printf("RELOP\n");
	//yylval = GE; return RELOP;
	 }
"!="	{ printf("RELOP\n");
	//yylval = NE; return RELOP;
	 }
"&&"	{ printf("AND\n");
	//return AND;
	 }
"||"	{ printf("OR\n");
	//return OR;
	 }
"."	{ printf("DOT\n");
	//return DOT;
	 }
"!"	{ printf("NOT\n");
	//return NOT;
	 }
"("	{ printf("LP\n");
	//return LP;
	 }
")"	{ printf("RP\n");
	//return RP;
	 }
"["	{ printf("LB\n");
	//return LB;
	 }
"]"	{ printf("RB\n");
	//return RB;
	 }
"{"	{ printf("LC\n");
	//return LC;
	 }
"}"	{ printf("RC\n");
	//return RC;
	 }
"struct"	{ printf("STRUCT\n");
		//return STRUCT;
		 }
"return"	{ printf("RETURN\n");
		//return RETURN;
		 }
"if"	{ printf("IF\n");
	//return IF;
	 }
"else"	{ printf("ELSE\n");
	//return ELSE;
	 }
"while"	{ printf("WHILE\n");
	//return WHILE;
	 }
{INT}	{ printf("INT\n"); 
	 //return INT;
	 }
{FLOAT}	{ printf("FLOAT\n");
	//yylval = atof(yytext); return FLOAT;
	 }
{EFLOAT}	{printf("EFLOAT\n");
	 }
"int"	{ printf("TYPE\n");
	//return TYPE;
	 }
"float"	{ printf("TYPE\n");
	//return TYPE;
	 }
{ID}	{ printf("ID\n");
	//return ID;
	 }
.	{ printf("ERROR type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext); }

%%
