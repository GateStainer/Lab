#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "type.h"
#include "semantic.h"

void semanticTest(TreeNode* root)
{
	visitProgram(root);
}

void visitProgram(TreeNode* root)
{
	TreeNode* child = root->firstChild;
	//Program --> ExtDefList
	visitExtDefList(child);
}

void visitExtDefList(TreeNode* root)
{
	//ExtDefList --> epsilon
	if(strcmp(root->name, "Empty") == 0)
		return;
	TreeNode* child = root->firstChild;
	//ExtDefList --> ExtDef ExtDefList
	visitExtDef(child);
	visitExtDefList(child->next);
}

//
void visitExtDef(TreeNode* root)
{
	TreeNode* child = root->firstChild;
	TypeP specifier = visitSpecifier(child);
	if(specifier == NULL)
		return;
	child = child->next;
	//ExtDef --> Specifier ExtDecList SEMI
	if(strcmp(child->name, "ExtDecList") == 0)
	{
		visitExtDecList(child, specifier);
	}
	//ExtDef --> Specifier SEMI
	else if(strcmp(child->name, "SEMI") == 0)
	{
		return;
	}
	//ExtDef --> Specifier FunDec CompSt
	else if(strcmp(child->name, "FunDec") == 0)
	{
		TypeP type = visitFunDec(child, specifier);
		if(type == NULL)
			return;
		TableNode* temp_node = searchSymbolTable(type->u.structure->name);
		if(temp_node != NULL)
		{
			printf("Error type 4 at Line %d: Redefined function \"%s\".\n", child->lineno, temp_node->name);
			return;
		}
		else
		{
			insertSymbolTable(type->u.structure->name, type);
			child = child->next;
			visitCompSt(child, specifier);
		}

	}
	else
	{
		printf("Error in visitExtDef\n");
		exit(-1);
	}

}

TypeP visitFunDec(TreeNode* root, TypeP lasttype)
{
	TypeP type = (TypeP)malloc(sizeof(Type_));
	TreeNode* child = root->firstChild;
	type->kind = FUNCTION;
	type->u.structure = (FieldListP)malloc(sizeof(FieldList_));
	type->u.structure->name = child->data;
	type->u.structure->type = lasttype;
	FieldListP field = type->u.structure;
	child = child->next->next;
	//FunDec --> ID LP RP
	if(strcmp(child->name, "RP") == 0)
	{
		field->tail = NULL;
	}
	//FunDec --> ID LP VarList RP
	else
	{
		for(child = child->firstChild; ; )
		{
			TypeP point = visitSpecifier(child->firstChild);
			if(point != NULL)
			{
				FieldListP tempfield = visitVarDec(child->firstChild->next, point, false);
				field->tail = tempfield;
				field = tempfield;
			}
			child = child->next;
			if(child == NULL)
				break;
			else
				child = child->next->firstChild;
		}
	}
	return type;
}

//CompSt --> LC DefList StmtList RC
void visitCompSt(TreeNode* root, TypeP returntype)
{
	TreeNode* child = root->firstChild->next;
	FieldListP head = visitDefList(child, false);
	child = child->next;
	while(true)
	{
		if(strcmp(child->name, "Empty") == 0)
			break;
		else
		{
			child = child->firstChild;
			visitStmt(child, returntype);
			child = child->next;
		}
	}
}

void visitStmt(TreeNode* root, TypeP returntype)
{
	TreeNode* child = root->firstChild;
	//Stmt --> Exp SEMI
	if(strcmp(child->name, "Exp") == 0)
		visitExp(child);
	//Stmt --> CompSt
	else if(strcmp(child->name, "CompSt") == 0)
	{
		visitCompSt(child, returntype);
	}
	//Stmt --> RETURN Exp SEMI
	else if(strcmp(child->name, "RETURN") == 0)
	{
		TypeP exptype = visitExp(child->next);
		if(sameType(returntype, exptype) == false)
		{
			printf("Error type 8 at Line %d: Type mismatched for return.\n", child->lineno);
			return;
		}
	}
	//Stmt --> IF LP Exp RP Stmt | IF LP Exp RP Stmt ELSE Stmt
	else if(strcmp(child->name, "IF") == 0)
	{
		child = child->next->next;
		TypeP point = visitExp(child);
		if(point==NULL)
			return;
		if(!(point->kind==BASIC && point->u.basic==INT))
		{
			printf("Error type 7 at Line %d: Type mismatched for if.\n", child->lineno);
			return;
		}
		child = child->next->next;
		visitStmt(child, returntype);
		child = child->next;
		if(child != NULL)
			visitStmt(child->next, returntype);
	}
	//Stmt --> WHILE LP Exp RP Stmt
	else if(strcmp(child->name, "WHILE") == 0)
	{
		child = child->next->next;
		TypeP point = visitExp(child);
		if(point==NULL)
			return;
		if(!(point->kind==BASIC && point->u.basic==INT))
		{
			printf("Error type 7 at Line %d: Type mismatched for while.\n",child->lineno);
			return;
		}
		child = child->next->next;
		visitStmt(child, returntype);
	}
	else
	{
		printf("Error in visitStmt\n");
		exit(-1);
	}
}

void visitExtDecList(TreeNode* root, TypeP type)
{
	TreeNode* child = root->firstChild;
	while(true)
	{
		visitVarDec(child, type, false);
		child = child->next;
		if(child == NULL)
			break;
		else
		{
			child = child->next->firstChild;
		}
	}
}

TypeP visitSpecifier(TreeNode* root)
{
	TreeNode* child = root->firstChild;
	//Specifier --> TYPE
	if(strcmp(child->name, "TYPE")==0)
	{
		TypeP type = (TypeP)malloc(sizeof(Type_));
		type->kind = BASIC;
		if(strcmp(child->data, "INT")==0)
			type->u.basic = INT;
		else if(strcmp(child->data, "FLOAT")==0)
			type->u.basic = FLOAT;
		return type;
	}
	//Specifier --> StructSpecifier
	else if(strcmp(child->name, "StructSpecifier")==0)
	{
		child = child->firstChild->next;
		//StructSpecifier --> STRUCT OptTag LC DefList RC
		if(strcmp(child->name, "OptTag")==0 || strcmp(child->name, "Empty")==0)
		{
			char* name = NULL;
			//OptTag --> ID
			if(strcmp(child->name, "OptTag")==0)
			{
				name = child->firstChild->data;
				TableNode* temp_node = searchSymbolTable(name);
				if(temp_node != NULL)
				{
					printf("Error type 16 at Line %d: Duplicated name \"%s\".\n", child->lineno, name);
					return NULL;
				}
			}
			child = child->next->next;
			FieldListP head = NULL;
			//DefList --> Def DefList
			if(strcmp(child->name, "Empty") != 0)
			{
				head = visitDefList(child, true);
			}
			TypeP type = (TypeP)malloc(sizeof(Type_));
			type->kind = STRUCTURE;
			type->u.structure = head;
			if(name != NULL)
				insertSymbolTable(name, type);
			return type;
		}
		//StructSpecifier --> STRUCT Tag
		else if(strcmp(child->name, "Tag")==0)
		{
			child = child->firstChild;
			char* temp_name = child->data;
			TableNode* temp_node = searchSymbolTable(temp_name);
			if(temp_node == NULL)
			{
				printf("Error type 17 at Line %d: Undefined structure \"%s\".\n", child->lineno, child->data);
				return NULL;
			}	
			else if(temp_node->type->kind != STRUCTURE)
			{
				printf("Error type X at line %d: \"%s\" is not a struct.\n", child->lineno, child->name);
				return NULL;
			}
			else
			{
				return temp_node->type;
			}
		}
		else
		{
			printf("Error in StructSpecifier\n");
			exit(-1);
		}
	}
	else
	{
		printf("Error in visitSpecifier\n");
		exit(-1);
	}
}

//DefList --> Def DefList
FieldListP visitDefList(TreeNode* root, bool inStruct)
{
	if(strcmp(root->name, "Empty") == 0)
		return NULL;
	FieldListP head = NULL;
	TreeNode* child = root->firstChild;
	FieldListP field = visitDef(child, inStruct);
	head = field;
	
	child = child->next;
	FieldListP nexthead = visitDefList(child, inStruct);
	FieldListP temp_head = head;
	FieldListP temp_tail = head;
	for(; temp_head!=NULL; )
	{
		temp_tail = temp_head;
		temp_head = temp_head->tail;
	}
	if(temp_tail == NULL)
		head = nexthead;
	else
		temp_tail->tail = nexthead;
	return head;
}

//Def --> Specifier DecList SEMI
FieldListP visitDef(TreeNode* root, bool inStruct)
{
	TreeNode* child = root->firstChild;
	TypeP type = visitSpecifier(child);
	if(type == NULL)
		return NULL;
	FieldListP field = NULL;
	//DecList --> Dec | Dec COMMA DecList
	child = child->next->firstChild;
	while(true)
	{
		//Dec --> VarDec | VarDec ASSIGNOP Exp
		TreeNode* varDec = child->firstChild;
		FieldListP result = visitVarDec(varDec, type, inStruct);
		//Dec --> VarDec ASSIGNOP Exp
		if(varDec->next != NULL)
		{
			if(inStruct == true)
			{
				printf("Error type 15 ar Line %d: Illegal initialization in structure.\n", varDec->lineno);
			}
			else
			{
				TypeP exptype = visitExp(varDec->next->next);
				if(sameType(type, exptype) == false)
				{
					printf("Error type 5 at Line %d: Type mismatched for assignment.\n", varDec->lineno);
				}
			}
		}
		if(result != NULL)
		{
			FieldListP newField = (FieldListP)malloc(sizeof(FieldList_));
			newField->name = result->name;
			newField->type = result->type;
			newField->tail = field;
			field = newField;
		}
		child = child->next;
		if(child != NULL)
			child = child->next->firstChild;
		else
			break;

	}
	return field;
}

//VarDec --> ID | VarDec LB INT RB
FieldListP visitVarDec(TreeNode* root, TypeP type, bool inStruct)
{
	TypeP lasttype = type;
	TreeNode* child = root->firstChild;
	FieldListP result = (FieldListP)malloc(sizeof(FieldList_));
	TreeNode* next;
	while(strcmp(child->name, "ID") != 0)
	{
		TypeP newtype = (TypeP)malloc(sizeof(Type_));
		newtype->kind = ARRAY;
		next = child->next->next;
		newtype->u.array.size = atoi(next->data);
		newtype->u.array.elem = lasttype;
		lasttype = newtype;
		next = next->next;
		child = child->firstChild;
	}

	result->name = child->data;
	result->type = lasttype;
	result->tail = NULL;

	TableNode* temp_node = insertSymbolTable(result->name, result->type);
	if(temp_node == NULL)
	{
		if(inStruct)
			printf("Error type 15 at Line %d: Redefined field \"%s\".\n", child->lineno, result->name);
		else
			printf("Error type 3 at Line %d: Redefined variable \"%s\".\n", child->lineno, result->name);
	}
	return result;
}

TypeP visitExp(TreeNode* root)
{
	TreeNode* child = root->firstChild;
	if(strcmp(child->name, "Exp") == 0)
	{
		//Exp --> Exp ASSIGNOP Exp
		if(strcmp(child->next->name, "ASSIGNOP") == 0)
		{
			TreeNode* node = child->firstChild;
			if(!((strcmp(node->name, "ID")==0 && node->next == NULL) || (strcmp(node->name, "Exp")==0 && strcmp(node->next->name, "DOT")==0) \
				|| (strcmp(node->name, "Exp")==0 && strcmp(node->next->name, "LB")==0 && strcmp(node->next->next->name, "Exp")==0)))
			{
				printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", child->lineno);
				return NULL;
			}
			TypeP type1 = visitExp(child);
			TypeP type2 = visitExp(child->next->next);
			if(sameType(type1,type2) == false)
			{
				printf("Error type 5 at Line %d: Type mismatched for assignment.\n", child->lineno);
				return NULL;
			}
			return type1;
		}
		//Exp --> Exp AND Exp | Exp OR Exp
		else if(strcmp(child->next->name, "AND") == 0 || strcmp(child->next->name, "OR") == 0)
		{
			TypeP type1 = visitExp(child);
			TypeP type2 = visitExp(child->next->next);
			if(!(sameType(type1, type2) && type1->kind==BASIC && type1->u.basic==INT))
			{
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", child->lineno);
				return NULL;
			}
			return type1;
		}
		//Exp --> Exp RELOP Exp
		else if(strcmp(child->next->name, "RELOP") == 0)
		{
			TypeP type1 = visitExp(child);
			TypeP type2 = visitExp(child->next->next);
			if(!(sameType(type1, type2) && type1->kind==BASIC && (type1->u.basic==INT || type1->u.basic==FLOAT)))
			{
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", child->lineno);
				return NULL;
			}
			TypeP newtype = (TypeP)malloc(sizeof(Type_));
			newtype->kind = BASIC;
			newtype->u.basic = INT;
			return newtype;
		}
		//Exp --> Exp PLUS Exp | Exp MINUS Exp | Exp STAR Exp | Exp DIV Exp
		else if(strcmp(child->next->name, "PLUS") == 0 || strcmp(child->next->name, "MINUS") == 0 \
			|| strcmp(child->next->name, "STAR") == 0 || strcmp(child->next->name, "DIV") == 0)
		{
			TypeP type1 = visitExp(child);
			TypeP type2 = visitExp(child->next->next);
			if(!(sameType(type1, type2) && type1->kind==BASIC && (type1->u.basic==INT || type1->u.basic==FLOAT)))
			{
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", child->lineno);
				return NULL;
			}
			return type1;
		}
		//Exp --> Exp LB Exp RB
		else if(strcmp(child->next->name, "LB") == 0)
		{
			TypeP type1 = visitExp(child);
			if(type1 == NULL)
				return NULL;
			else if(type1->kind != ARRAY)
			{
				printf("Error type 10 at Line %d: \"%s\" is not an array.\n", child->lineno, child->data);
				return NULL;
			}
			else
			{
				child = child->next->next;
				TypeP type2 = visitExp(child);
				if(type2 == NULL)
					return NULL;
				else if(!(type2->kind == BASIC && type2->u.basic == INT))
				{
					printf("Error type 12 at Line %d: \"%s\" is not an integer.\n", child->lineno, child->data);
					return NULL;
				}
				else
					return type1->u.array.elem;
			}
		}
		//Exp --> Exp DOT ID
		else if(strcmp(child->next->name, "DOT") == 0)
		{
			TypeP type1 = visitExp(child);
			if(type1 == NULL)
				return NULL;
			else if(type1->kind != STRUCTURE)
			{
				printf("Error type 13 at Line %d: Illegal use of \".\".\n", child->lineno);
				return NULL;
			}
			else
			{
				child = child->next->next;
				FieldListP field = type1->u.structure;
				for(; field!=NULL; field=field->tail)
					if(strcmp(field->name, child->data) == 0)
						break;
				if(field == NULL)
				{
					printf("Error type 14 at Line %d: Non-existent field \"%s\".\n", child->lineno, child->data);
					return NULL;					
				}
				else
				{
					return field->type;
				}

			}
		}
		else
		{
			printf("Error in visitExp Exp\n");
			exit(-1);
		}

	}
	else if(strcmp(child->name, "LP") == 0)
	{
		if(strcmp(child->next->name, "Exp") == 0)
		{
			return visitExp(child->next);
		}
		else
		{
			printf("Error in visitExp LP\n");
			exit(-1);
		}
	}
	else if(strcmp(child->name, "MINUS") == 0)
	{
		if(strcmp(child->next->name, "Exp") == 0)
		{
			TypeP temp_type = visitExp(child->next);
			if(temp_type==NULL)
				return NULL;
			if(temp_type->kind != BASIC)
			{
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", child->lineno);
				return NULL;
			}
			return temp_type;
		}
		else
		{
			printf("Error in visitExp MINUS\n");
			exit(-1);
		}
	}
	else if(strcmp(child->name, "NOT") == 0)
	{
		if(strcmp(child->next->name, "Exp") == 0)
		{
			TypeP temp_type = visitExp(child->next);
			if(temp_type==NULL)
				return NULL;
			if(temp_type->kind != BASIC || temp_type->u.basic != INT)
			{
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", child->lineno);
				return NULL;
			}
			TypeP newtype = (TypeP)malloc(sizeof(Type_));
			newtype->kind = BASIC;
			newtype->u.basic = INT;
			return newtype;
		}
		else
		{
			printf("Error in visitExp NOT\n");
			exit(-1);
		}
	}
	else if(strcmp(child->name, "ID") == 0)
	{
		//Exp --> ID LP RP | ID LP Args RP
		if(child->next != NULL)
		{
			TableNode* temp_node = searchSymbolTable(child->data);
			if(temp_node == NULL)
			{
				printf("Error type 2 at Line %d: Undefined function \"%s\".\n", child->lineno, child->data);
				return NULL;
			}
			else if(temp_node->type->kind != FUNCTION)
			{
				printf("Error type 11 at Line %d: \"%s\" is not a function.\n", child->lineno, child->data);
				return NULL;
			}
			else
			{
				TypeP temp_type = temp_node->type;
				//Exp --> ID LP RP
				if(strcmp(child->next->next->name, "RP")==0)
				{
					if(temp_type->u.structure->tail == NULL)
					{
						return temp_type->u.structure->type;
					}
					else
					{
						printf("Error type 9 at Line %d: Too few arguments for function \"%s\".\n",child->lineno, child->data);
						return NULL;
					}
				}
				else
				{
					FieldListP args = temp_type->u.structure->tail;
					if(args == NULL)
					{
						printf("Error type 9 at Line %d: Too many arguments for function \"%s\".\n", child->lineno, child->data);
						return NULL;
					}
					char* function_name = child->data;
					for(child = child->next->next->firstChild; ; )
					{
						TypeP child_type = visitExp(child);
						if(child_type == NULL)
							return NULL;
						if(sameType(child_type, args->type) == false)
						{
							printf("Error type 9 at Line %d: Arguments type mismatch in function \"%s\".\n", child->lineno, function_name);
							return NULL;
						}
						args = args->tail;
						if(args == NULL && child->next != NULL)
						{
							printf("Error type 9 at Line %d: Too many arguments for function \"%s\".\n", child->lineno, function_name);
							return NULL;
						}
						if(args != NULL && child->next == NULL)
						{
							printf("Error type 9 at Line %d: Too feq arguments for function \"%s\".\n", child->lineno, function_name);
							return NULL;
						}
						if(args == NULL && child->next == NULL)
							return temp_node->type->u.structure->type;
						child = child->next->next->firstChild;
					}

				}
			}
		}
		//Exp --> ID
		else
		{
			TableNode* temp_node = searchSymbolTable(child->data);
			if(temp_node == NULL)
			{
				printf("Error type 1 at Line %d: Undefined variable \"%s\".\n", child->lineno, child->data);
				return NULL;
			}
			else
			{
				return temp_node->type;
			}
		}
	}
	else if(strcmp(child->name, "INT") == 0)
	{
		TypeP newtype = (TypeP)malloc(sizeof(Type_));
		newtype->kind = BASIC;
		newtype->u.basic = INT;
		return newtype;
	}
	else if(strcmp(child->name, "FLOAT") == 0)
	{
		TypeP newtype = (TypeP)malloc(sizeof(Type_));
		newtype->kind = BASIC;
		newtype->u.basic = FLOAT;
		return newtype;
	}
	else
	{
		printf("Error in visitExp\n");
		exit(-1);
	}
}

bool sameType(TypeP type1, TypeP type2)
{
	if(type1 == NULL || type2 == NULL)
		return false;
	if(type1->kind == type2->kind)
	{
		if(type1->kind == BASIC)
		{
			if(type1->u.basic == type2->u.basic)
				return true;
			else
				return false;
		}
		else if(type1->kind == ARRAY)
			return sameType(type1->u.array.elem, type2->u.array.elem);
		else if(type1->kind == STRUCTURE)
		{
			FieldListP p1 = type1->u.structure;
			FieldListP p2 = type2->u.structure;
			while(p1 != NULL && p2 != NULL)
			{
				if(!sameType(p1->type, p1->type))
					return false;
				p1 = p1->tail;
				p2 = p2->tail;
			}
			if(p1 != NULL || p2 != NULL)
				return false;
			return true;
		}
		else if(type1->kind == FUNCTION)
		{
			FieldListP p1 = type1->u.structure;
			FieldListP p2 = type2->u.structure;
			while(p1 != NULL && p2 != NULL)
			{
				if(!sameType(p1->type, p2->type))
					return false;
				p1 = p1->tail;
				p2 = p2->tail;
			}
			if(p1 != NULL || p2 != NULL)
				return false;
			return true;
		}

	}
	else
		return false;
}







