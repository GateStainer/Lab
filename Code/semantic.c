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
				visitDefList(child, true, head);
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
void visitDefList(TreeNode* root, bool inStruct, FieldListP head)
{
	if(strcmp(root->name, "Empty") == 0)
		return;
	TreeNode* child = root->firstChild;
	FieldListP field = visitDef(child, inStruct);
	if(head == NULL)
	{
		head = field;
	}
	else
	{
		FieldListP temp_head = head;
		FieldListP temp_tail = head;
		for(; temp_head!=NULL; )
		{
			temp_tail = temp_head;
			temp_head = temp_head->tail;
		}
		temp_tail->tail = field;
	}
	child = child->next;
	visitDefList(child, inStruct, head);
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
				if(exptype != NULL && sameType(type, exptype) == false)
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
		if(strcmp(child->next->name, "ASSIGNOP") == 0)
		{

		}
		else if(strcmp(child->next->name, "AND") == 0)
		{

		}
		else if(strcmp(child->next->name, "OR") == 0)
		{

		}
		else if(strcmp(child->next->name, "RELOP") == 0)
		{

		}
		else if(strcmp(child->next->name, "PLUS") == 0)
		{

		}
		else if(strcmp(child->next->name, "MINUS") == 0)
		{

		}
		else if(strcmp(child->next->name, "STAR") == 0)
		{

		}
		else if(strcmp(child->next->name, "DIV") == 0)
		{

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

		}
		else
		{
			printf("Error in visitExp NOT\n");
			exit(-1);
		}
	}
	else if(strcmp(child->name, "ID" == 0)
	{

	}
	else if(strcmp(child->name, "INT") == 0)
	{

	}
	else if(strcmp(child->name, "FLOAT") == 0)
	{

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





















