#include <stdlib.h>
#include<stdio.h>
#include <string.h>

#include "symbolTable.h"
#include "semantic.h"
#include "type.h"
#include "ir_generator.h"
#include "ir_structure.h"
#include "TreeNode.h"

extern TreeNode *root;
int temp_index = -1;
int label_index = -1;

int getSize(TypeP p)
{
	if(p->kind == BASIC)
		return 4;
	else if(p->kind == ARRAY)
	{
		return p->u.array.size*getSize(p->u.array.elem);
	}
	else
	{
		printf("Other types are not implemented in getSize()!\n");
		return 0;
	}
}

char* get_temp_name(int index)
{
	char* result;
	result = (char*)malloc(32);
	result[0] = 't';
	sprintf(result+1 ,"%d", index);
	return result;
}

char* get_label_name(int index)
{
	char* result;
	result = (char*)malloc(32);
	result[0] = 'L';
	sprintf(result+1, "%d", index);
	return result;
}

void generate_ir(char* filename)
{
	ir_head = translate_Program(root);
	printf("Complete translating the Program\n");
	writeInterCodes(filename);
}

InterCodes* translate_Program(TreeNode* root)
{
	if(root == NULL || strcmp(root->name, "Empty") == 0)
		return NULL;
	if(strcmp(root->name, "Program") == 0)
		return translate_Program(root->firstChild);
	if(strcmp(root->name, "ExtDefList") == 0)
		return mergeInterCodes(translate_Program(root->firstChild), translate_Program(root->firstChild->next));
	if(strcmp(root->name, "ExtDef") == 0)
	{
		TreeNode* child = root->firstChild->next;
		if(strcmp(child->name, "FunDec") == 0)
		{
			if(strcmp(child->next->name, "CompSt") != 0)
			{
				printf("Error in translate_Program()!\n");
				exit(-1);
			}
			InterCodes* fundec = translate_FunDec(child);
			InterCodes* compst = translate_CompSt(child->next);
			return mergeInterCodes(fundec, compst);
		}
		else	
			return NULL;
	}
	printf("Shouldn't reach here in translate_Program()\n");
	return NULL;
}

//FunDec --> ID LP VarList RP | ID LP RP
InterCodes* translate_FunDec(TreeNode* root)
{
	if(root == NULL || strcmp(root->name, "Empty") == 0)
	{
		printf("Error in translate_FunDec()!\n");
		exit(-1);
	}
	TreeNode* child = root->firstChild;
	InterCodes* id_code = mallocInterCodes();
	id_code->code.kind = IFUNCTION;
	Operand id_operand = mallocOperand(VARIABLE, child->data);
	id_code->code.u.func = id_operand;
	child = child->next->next;
	//VarList --> ParamDec COMMA VarList | ParamDec
	if(strcmp(child->name, "VarList") == 0)
	{
		InterCodes* tail = id_code;
		child = child->firstChild;
		while(true)
		{
			TreeNode* temp_node = child->firstChild->next;
			temp_node = temp_node->firstChild;
			while(strcmp(temp_node->name, "ID") != 0)
				temp_node = temp_node->firstChild;
			InterCodes* temp_code = mallocInterCodes();
			temp_code->prev = tail;
			tail->next = temp_code;
			temp_code->code.kind = IPARAM;
			Operand temp_operand = mallocOperand(VARIABLE, temp_node->data);
			temp_code->code.u.param = temp_operand;
			child = child->next;
			if(child == NULL)
				break;
			else
				child = child->next->firstChild;
		}
	}
	return id_code;
}

InterCodes* translate_CompSt(TreeNode* root)
{
	TreeNode* child = root->firstChild->next;
	InterCodes* code1 = translate_DefList(child);
	InterCodes* code2 = translate_StmtList(child->next);
	return mergeInterCodes(code1, code2);
}

//DefList --> Def DefList | epsilon
InterCodes* translate_DefList(TreeNode* root)
{
	if(root == NULL || strcmp(root->name, "Empty") == 0)
		return NULL;
	InterCodes* deflist_code = NULL;
	TreeNode* child = root->firstChild;
	while(true)
	{
		//Def --> Specifier DecList SEMI
		TreeNode* dec = child->firstChild->next->firstChild;
		TreeNode* specifier = child->firstChild;
		TypeP type = visitSpecifier(specifier);

		while(true)
		{
			TreeNode* vardec = dec->firstChild;
			//Dec --> VarDec ASSIGNOP Exp
			if(vardec->next != NULL)
			{
				Operand place = mallocOperand(VARIABLE, vardec->firstChild->data);
				Operand t1 = (Operand)malloc(sizeof(struct Operand_));
				InterCodes* temp_code = translate_Exp(vardec->next->next, t1);
				InterCodes* code2 = mallocInterCodes();
				code2->code.kind = IASSIGN;
				code2->code.u.assign.left = place;
				code2->code.u.assign.right = t1;
				deflist_code = mergeInterCodes(mergeInterCodes(deflist_code, temp_code), code2);
			}
			//VarDec --> VarDec LB INT RB
			else if(strcmp(vardec->firstChild->name, "ID") != 0)
			{
				TreeNode* id = vardec->firstChild->firstChild;
				TableNode* table = searchSymbolTable(id->data);
				InterCodes* temp_code = mallocInterCodes();
				temp_code->code.kind = IDEC;
				temp_code->code.u.dec.left = mallocOperand(VARIABLE, id->data);
				temp_code->code.u.dec.size = getSize(type);
				deflist_code = mergeInterCodes(deflist_code, temp_code);
			}
			if(dec->next == NULL)
				break;
			else
				dec = dec ->next->next->firstChild;
		}
		if(strcmp(child->next->name, "Empty") == 0)
			break;
		else
			child = child->next->firstChild;
	}
	return deflist_code;
}

InterCodes* translate_StmtList(TreeNode* root)
{
	if(root == NULL || strcmp(root->name, "Empty") == 0)
		return NULL;
	else
	{
		TreeNode* stmt = root->firstChild;
		return mergeInterCodes(translate_Stmt(stmt), translate_StmtList(stmt->next));
	}
}

InterCodes* translate_Stmt(TreeNode* root)
{
	InterCodes* result = NULL;
	TreeNode* child = root->firstChild;
	if(strcmp(child->name, "Exp") == 0)
	{
		Operand place = (Operand)malloc(sizeof(struct Operand_));
		return translate_Exp(child, place);
	}
	else if(strcmp(child->name, "CompSt") == 0)
	{
		printf("Reach CompSt!\n");
	}
	else if(strcmp(child->name, "RETURN") == 0)
	{
		Operand t1 = (Operand)malloc(sizeof(struct Operand_));
		t1->kind = TEMP;
		temp_index++;
		t1->u.temp_no = temp_index;
		InterCodes* code1 = translate_Exp(child->next, t1);
		InterCodes* code2 = mallocInterCodes();
		code2->code.kind = IRETURN;
		code2->code.u.return_val = t1;
		result = mergeInterCodes(code1, code2);
	}
	else if(strcmp(child->name, "IF") == 0)
	{
		TreeNode* stmt1 = child->next->next->next->next;
		if(stmt1->next == NULL)
		{
			InterCodes* label1 = mallocInterCodes();
			label1->code.kind = ILABEL;
			Operand temp_l1 = (Operand)malloc(sizeof(struct Operand_));
			temp_l1->kind = LABELNO;
			label_index++;
			temp_l1->u.label_no = label_index;
			label1->code.u.label = temp_l1;	

			InterCodes* label2 = mallocInterCodes();
			label2->code.kind = ILABEL;
			Operand temp_l2 = (Operand)malloc(sizeof(struct Operand_));
			temp_l2->kind = LABELNO;
			label_index++;
			temp_l2->u.label_no = label_index;
			label2->code.u.label = temp_l2;

			InterCodes* code1 = translate_Cond(child->next->next, label1, label2);
			InterCodes* code2 = translate_Stmt(stmt1);
			result = mergeInterCodes(code1, label1);
			result = mergeInterCodes(result, code2);
			result = mergeInterCodes(result, label2);
		}
		else
		{
			InterCodes* label1 = mallocInterCodes();
			label1->code.kind = ILABEL;
			Operand temp_l1 = (Operand)malloc(sizeof(struct Operand_));
			temp_l1->kind = LABELNO;
			label_index++;
			temp_l1->u.label_no = label_index;
			label1->code.u.label = temp_l1;

			InterCodes* label2 = mallocInterCodes();
			label2->code.kind = ILABEL;
			Operand temp_l2 = (Operand)malloc(sizeof(struct Operand_));
			temp_l2->kind = LABELNO;
			label_index++;
			temp_l2->u.label_no = label_index;
			label2->code.u.label = temp_l2;

			InterCodes* label3 = mallocInterCodes();	
			label3->code.kind = ILABEL;
			Operand temp_l3 = (Operand)malloc(sizeof(struct Operand_));
			temp_l3->kind = LABELNO;
			label_index++;
			temp_l3->u.label_no = label_index;
			label3->code.u.label = temp_l3;

			InterCodes* goto_code = mallocInterCodes();
			goto_code->code.kind = IGOTO;
			Operand temp_ll = (Operand)malloc(sizeof(struct Operand_));
			temp_ll->kind = LABELNO;
			temp_ll->u.label_no = label_index;
			goto_code->code.u.go_to = temp_ll;

			InterCodes* code1 = translate_Cond(child->next->next, label1, label2);
			InterCodes* code2 = translate_Stmt(stmt1);
			TreeNode* stmt2 = stmt1->next->next;
			InterCodes* code3 = translate_Stmt(stmt2);
			result = mergeInterCodes(code1, label1);
			result = mergeInterCodes(result, code2);
			result = mergeInterCodes(result, goto_code);
			result = mergeInterCodes(result, label2);
			result = mergeInterCodes(result, code3);
			result = mergeInterCodes(result, label3);
		}
	}
	else if(strcmp(child->name, "WHILE") == 0)
	{
		InterCodes* label1 = mallocInterCodes();
		label1->code.kind = ILABEL;
		Operand temp_l1 = (Operand)malloc(sizeof(struct Operand_));
		temp_l1->kind = LABELNO;
		label_index++;
		temp_l1->u.label_no = label_index;
		label1->code.u.label = temp_l1;

		InterCodes* label2 = mallocInterCodes();
		label2->code.kind = ILABEL;
		Operand temp_l2 = (Operand)malloc(sizeof(struct Operand_));
		temp_l2->kind = LABELNO;
		label_index++;
		temp_l2->u.label_no = label_index;
		label2->code.u.label = temp_l2;

		InterCodes* label3 = mallocInterCodes();
		label3->code.kind = ILABEL;
		Operand temp_l3 = (Operand)malloc(sizeof(struct Operand_));
		temp_l3->kind = LABELNO;
		label_index++;
		temp_l3->u.label_no = label_index;
		label3->code.u.label = temp_l3;

		TreeNode* exp = child->next->next;
		InterCodes* code1 = translate_Cond(exp, label2, label3);
		InterCodes* code2 = translate_Stmt(exp->next->next);

		InterCodes* goto_code = mallocInterCodes();
		goto_code->code.kind = IGOTO;
		Operand temp_ll = (Operand)malloc(sizeof(struct Operand_));
		temp_ll->kind = LABELNO;
		temp_ll->u.label_no = label_index-2;
		goto_code->code.u.go_to = temp_ll;

		result = mergeInterCodes(label1, code1);
		result = mergeInterCodes(result, label2);
		result = mergeInterCodes(result, code2);
		result = mergeInterCodes(result, goto_code);
		result = mergeInterCodes(result, label3);
	}
	else
	{
		printf("Unexpected node in translate_Stmt()!\n");
		exit(-1);
	}
	return result;
}

//Args --> Exp COMMA Args | Exp
InterCodes* translate_Args(TreeNode* root, ArgListNode* arg_list)
{
	TreeNode* child = root->firstChild;
	temp_index++;
	Operand t1 = (Operand)malloc(sizeof(struct Operand_));
	t1->kind = TEMP;
	t1->u.temp_no = temp_index;
	InterCodes* code1 = translate_Exp(child, t1);
	ArgListNode* new_arg = (ArgListNode*)malloc(sizeof(ArgListNode));
	new_arg->operand = t1;
	new_arg->next = arg_list->next;
	arg_list->next = new_arg;

	if(child->next == NULL)
		return code1;
	else
	{
		InterCodes* code2 = translate_Args(child->next->next, arg_list);
		return mergeInterCodes(code1, code2);
	}
}

InterCodes* translate_Exp(TreeNode* root, Operand place)
{
	TreeNode* child = root->firstChild;
	TreeNode* op = child->next;
	InterCodes* exp_code = mallocInterCodes();
	//Exp --> LP Exp RP
	if(strcmp(child->name, "LP") == 0)
	{
		return translate_Exp(child->next, place);
	}
	//Exp --> INT
	else if(strcmp(child->name, "INT") == 0)
	{
		place->kind = CONSTANT;
		place->u.value = atoi(child->data);
		return NULL;
	}
	//Exp --> ID | ID LP Args RP | ID LP RP
	else if(strcmp(child->name, "ID") == 0)
	{
		if(child->next == NULL)
		{
			TableNode* table = searchSymbolTable(child->data);
			if(table->type->kind == BASIC)
			{	
				place->kind = VARIABLE;
				place->u.var_name = child->data;
			}
			else
			{
				place->kind = REFERENCE;
				place->u.refer_name = child->data;
			}
			return NULL;
		}
		else
		{
			TreeNode* args = child->next->next;
			temp_index++;
			Operand new_temp = (Operand)malloc(sizeof(struct Operand_));
			new_temp->kind = TEMP;
			new_temp->u.temp_no = temp_index;
			if(strcmp(args->name, "Args") == 0)
			{
				ArgListNode* arg_list = (ArgListNode*)malloc(sizeof(ArgListNode));
				arg_list->next = NULL;
				InterCodes* code1 = translate_Args(args, arg_list);
				if(args->next == NULL)
				{
					printf("Error in translate_Exp()!\n");
					exit(-1);
				}
				if(strcmp(child->data, "write") == 0)
				{
					/*InterCodes* mid_code = mallocInterCodes();
					mid_code->code.kind = IASSIGN;
					Operand temp = (Operand)malloc(sizeof(struct Operand_));
					temp->kind = VARIABLE;
					char* temp_var = (char*)malloc(32);
					temp_var[0] = 'v';
					temp_var[1] = 'v';
					temp_var[2] = 'v';
					temp_var[3] = '\0';
					temp->u.var_name = temp_var;
					mid_code->code.u.assign.left = temp;
					mid_code->code.u.assign.right = arg_list->next->operand;*/
					exp_code->code.kind = IWRITE;
					exp_code->code.u.write_val = arg_list->next->operand;
					return mergeInterCodes(code1, exp_code);
				}
				else
				{
					ArgListNode* arg_node = arg_list->next;
					InterCodes* code2 = NULL;
					while(arg_node != NULL)
					{
						InterCodes* code_arg = (InterCodes*)malloc(sizeof(InterCodes));
						code_arg->prev = NULL;
						code_arg->next = NULL;
						code_arg->code.kind = IARG;
						code_arg->code.u.arg = arg_node->operand;
						code2 = mergeInterCodes(code2, code_arg);
						arg_node = arg_node->next;
					}
					InterCodes* code_temp = mallocInterCodes();
					code_temp->code.kind = ICALL;
					code_temp->code.u.callfunc.left = new_temp;
					Operand callfunc_right = mallocOperand(VARIABLE, child->data);
					code_temp->code.u.callfunc.right = callfunc_right;

					InterCodes* code3 = mallocInterCodes();
					code3->code.kind = IASSIGN;
					code3->code.u.assign.left = place;
					code3->code.u.assign.right = new_temp;
					code3 = mergeInterCodes(code_temp, code3);
					return mergeInterCodes(mergeInterCodes(code1, code2), code3);
				}
			}
			else
			{
				if(strcmp(child->data, "read") == 0)
				{
					exp_code->code.kind = IREAD;
					exp_code->code.u.read_val = new_temp;
				}
				else
				{
					exp_code->code.kind = ICALL;
					exp_code->code.u.callfunc.left = new_temp;
					Operand func_name = mallocOperand(VARIABLE, child->data);
					exp_code->code.u.callfunc.right = func_name;
				}
				InterCodes* code2 = mallocInterCodes();
				code2->code.kind = IASSIGN;
				code2->code.u.assign.left = place;
				code2->code.u.assign.right = new_temp;
				return mergeInterCodes(exp_code, code2);
			}
		}
	}
	else if(strcmp(child->name, "FLOAT") == 0)
	{
		printf("There shouldn't be float in translate_Exp()!\n");
		exit(-1);
	}
	//Exp --> Exp ASSIGNOP Exp
	else if(strcmp(op->name, "ASSIGNOP") == 0)
	{
		InterCodes* code1 = translate_Exp(child, place);
		Operand temp_p = (Operand)malloc(sizeof(struct Operand_));
		temp_p->kind = TEMP;
		temp_index++;
		temp_p->u.temp_no = temp_index;
		InterCodes* code2 = translate_Exp(op->next, temp_p);
		InterCodes* code3 = mallocInterCodes();
		code3->code.kind = IASSIGN;
		code3->code.u.assign.left = place;
		code3->code.u.assign.right = temp_p;
		return mergeInterCodes(mergeInterCodes(code1, code2), code3);
	}
	//Exp -->Exp PLUS Exp | Exp MINUS Exp | Exp STAR Exp | Exp DIV Exp
	else if(strcmp(op->name, "PLUS") == 0 || strcmp(op->name, "MINUS") == 0 || strcmp(op->name, "STAR") == 0 || strcmp(op->name, "DIV") == 0)
	{
		Operand p1 = (Operand)malloc(sizeof(struct Operand_));
		p1->kind = TEMP;
		temp_index++;
		p1->u.temp_no = temp_index;
		Operand p2 = (Operand)malloc(sizeof(struct Operand_));
		p2->kind = TEMP;
		temp_index++;
		p2->u.temp_no = temp_index;
		InterCodes* code1 = translate_Exp(child, p1);
		InterCodes* code2 = translate_Exp(op->next, p2);
		InterCodes* code3 = mallocInterCodes();
		if(strcmp(op->name, "PLUS") == 0)
			code3->code.kind = IADD;
		else if(strcmp(op->name, "MINUS") == 0)
			code3->code.kind = ISUB;
		else if(strcmp(op->name, "STAR") == 0)
			code3->code.kind = IMUL;
		else
			code3->code.kind = IDIV;
		code3->code.u.binop.result = place;
		code3->code.u.binop.op1 = p1;
		code3->code.u.binop.op2 = p2;
		return mergeInterCodes(mergeInterCodes(code1, code2), code3);
	}
	//Exp --> MINUS Exp
	else if(strcmp(child->name, "MINUS") == 0)
	{
		Operand new_temp = (Operand)malloc(sizeof(struct Operand_));
		new_temp->kind = TEMP;
		temp_index++;
		new_temp->u.temp_no = temp_index;
		Operand p1 = (Operand)malloc(sizeof(struct Operand_));
		p1->kind = CONSTANT;
		p1->u.value = 0;
		InterCodes* code1 = translate_Exp(child->next, new_temp);
		InterCodes* code2 = mallocInterCodes();
		code2->code.kind = ISUB;
		code2->code.u.binop.result = place;
		code2->code.u.binop.op1 = p1;
		code2->code.u.binop.op2 = new_temp;
		return mergeInterCodes(code1, code2);

	}
	//Exp --> NOT Exp | Exp AND Exp | Exp OR Exp | Exp RELOP Exp
	else if(strcmp(child->name, "NOT") == 0 || strcmp(op->name, "RELOP") == 0 || strcmp(op->name, "AND") == 0 || strcmp(op->name, "OR") == 0)
	{
		InterCodes* label1 = mallocInterCodes();
		InterCodes* label2 = mallocInterCodes();
		label1->code.kind = ILABEL;
		Operand temp_l1 = (Operand)malloc(sizeof(struct Operand_));
		temp_l1->kind = ILABEL;
		label_index++;
		temp_l1->u.label_no = label_index;
		Operand temp_l2 = (Operand)malloc(sizeof(struct Operand_));
		temp_l2->kind = LABELNO;
		label_index++;
		temp_l2->u.label_no = label_index;

		Operand p1 = (Operand)malloc(sizeof(struct Operand_));
		p1->kind = CONSTANT;
		p1->u.value = 0;
		InterCodes* code0 = mallocInterCodes();
		code0->code.kind = IASSIGN;
		code0->code.u.assign.left = place;
		code0->code.u.assign.right = p1;
		InterCodes* code1 = translate_Cond(root, label1, label2);
		Operand p2 = (Operand)malloc(sizeof(struct Operand_));
		p2->kind = CONSTANT;
		p2->u.value = 1;
		InterCodes* code_temp = mallocInterCodes();
		code_temp->code.kind = IASSIGN;
		code_temp->code.u.assign.left = place;
		code_temp->code.u.assign.right = p2;
		InterCodes* code2 = mergeInterCodes(label1, code_temp);
		return mergeInterCodes(mergeInterCodes(mergeInterCodes(code0, code1), code2), label2);
	}
	//Exp --> Exp LB Exp RB
	else if(strcmp(child->name, "Exp") == 0 && strcmp(op->name, "LB") == 0)
	{
		TreeNode* exp1 = child->firstChild;
		InterCodes* code1 = NULL;
		Operand p1 = NULL;
		if(strcmp(exp1->name, "ID") ==0)
			p1 = mallocOperand(REFERENCE, exp1->data);
		else
		{
			p1 = (Operand)malloc(sizeof(struct Operand_));
			p1->kind = TEMP;
			temp_index++;
			p1->u.temp_no = temp_index;
			code1 = translate_Exp(child, p1);
		}

		Operand p2 = (Operand)malloc(sizeof(struct Operand_));
		InterCodes* code2 = translate_Exp(op->next, p2);

		TypeP root_type = visitExp(root);
		InterCodes* offset_code = mallocInterCodes();
		offset_code->code.kind = IMUL;
		Operand temp_result = (Operand)malloc(sizeof(struct Operand_));
		temp_result->kind = TEMP;
		temp_index++;
		temp_result->u.temp_no = temp_index;
		offset_code->code.u.binop.result = temp_result;
		Operand temp_op1 = (Operand)malloc(sizeof(struct Operand_));
		temp_op1->kind = CONSTANT;
		temp_op1->u.value = getSize(root_type);
		offset_code->code.u.binop.op1 = temp_op1;
		offset_code->code.u.binop.op2 = p2;

		InterCodes* code3 = mallocInterCodes();
		code3->code.kind = IADD;
		if(code1 == NULL)
			code3->code.u.binop.op1 = p1;
		else
		{
			Operand temp_op3 = (Operand)malloc(sizeof(struct Operand_));
			temp_op3->kind = TEMP;
			temp_op3->u.temp_no = p1->u.temp_no;
			code3->code.u.binop.op1 = temp_op3;
		}
		code3->code.u.binop.op2 = offset_code->code.u.binop.result;
		temp_index++;
		place->u.temp_no = temp_index;
		Operand temp_op4 = (Operand)malloc(sizeof(struct Operand_));
		temp_op4->kind = TEMP;
		temp_op4->u.temp_no = place->u.temp_no;
		code3->code.u.binop.result = temp_op4;
		place->kind = ADDRESS;
		return mergeInterCodes(mergeInterCodes(code1, code2), mergeInterCodes(offset_code, code3));
	}
	//Exp --> Exp DOT ID
	else if(strcmp(op->name, "DOT") == 0)
	{
		printf("Struct is not supported in translate_Exp()!\n");
		exit(-1);
	}
	else
	{
		printf("Unexpected expression in translate_Exp()!\n");
		return NULL;
	}
	return exp_code;
}

InterCodes* translate_Cond(TreeNode* root, InterCodes* label1, InterCodes* label2)
{
	TreeNode* exp1 = root->firstChild;
	TreeNode* op = exp1->next;
	if(strcmp(op->name, "RELOP") == 0)
	{
		Operand t1 = (Operand)malloc(sizeof(struct Operand_));
		t1->kind = TEMP;
		temp_index++;
		t1->u.temp_no = temp_index;
		Operand t2 = (Operand)malloc(sizeof(struct Operand_));
		t2->kind = TEMP;
		temp_index++;
		t2->u.temp_no = temp_index;

		InterCodes* code1 = translate_Exp(exp1, t1);
		InterCodes* code2 = translate_Exp(op->next, t2);

		InterCodes* code3 = mallocInterCodes();
		code3->code.kind = IIFSTMT;
		strcpy(code3->code.u.ifstmt.relop, op->data);
		code3->code.u.ifstmt.left = t1;
		code3->code.u.ifstmt.right = t2;
		code3->code.u.ifstmt.label = label1->code.u.label;

		InterCodes* code4 = mallocInterCodes();
		code4->code.kind = IGOTO;
		code4->code.u.go_to = label2->code.u.label;

		return mergeInterCodes(mergeInterCodes(mergeInterCodes(code1, code2), code3), code4);
	}
	else if(strcmp(exp1->name, "NOT") == 0)
	{
		return translate_Cond(exp1->next, label2, label1);
	}
	else if(strcmp(op->name, "AND") == 0)
	{
		InterCodes* temp_label = mallocInterCodes();
		temp_label->code.kind = ILABEL;
		label_index++;
		Operand temp_ll = (Operand)malloc(sizeof(struct Operand_));
		temp_ll->kind = LABELNO;
		temp_ll->u.label_no = label_index;
		temp_label->code.u.label = temp_ll;
		InterCodes* code1 = translate_Cond(exp1, temp_label, label2);
		InterCodes* code2 = translate_Cond(op->next, label1, label2);
		return mergeInterCodes(mergeInterCodes(code1, temp_label), code2);
	}
	else if(strcmp(op->name, "OR") == 0)
	{
		InterCodes* temp_label = mallocInterCodes();
		temp_label->code.kind = ILABEL;
		label_index++;
		Operand temp_ll = (Operand)malloc(sizeof(struct Operand_));
		temp_ll->kind = LABELNO;
		temp_ll->u.label_no = label_index;
		temp_label->code.u.label = temp_ll;
		InterCodes* code1 = translate_Cond(exp1, label1, temp_label);
		InterCodes* code2 = translate_Cond(op->next, label1, label2);
		return mergeInterCodes(mergeInterCodes(code1, temp_label), code2);
	}
	else
	{
		Operand t1 = (Operand)malloc(sizeof(struct Operand_));
		t1->kind = TEMP;
		temp_index++;
		t1->u.temp_no = temp_index;
		Operand t2 = (Operand)malloc(sizeof(struct Operand_));
		t2->kind = CONSTANT;
		t2->u.value = 0;

		InterCodes* code1 = translate_Exp(root, t1);
		InterCodes* code2 = mallocInterCodes();
		code2->code.kind = IIFSTMT;
		code2->code.u.ifstmt.relop[0] = '!';
		code2->code.u.ifstmt.relop[1] = '=';
		code2->code.u.ifstmt.relop[2] = '\0';
		code2->code.u.ifstmt.left = t1;
		code2->code.u.ifstmt.right = t2;
		Operand temp_ll = (Operand)malloc(sizeof(struct Operand_));
		temp_ll->kind = LABELNO;
		temp_ll->u.label_no = label1->code.u.label->u.label_no;;
		code2->code.u.ifstmt.label = temp_ll;

		InterCodes* code3 = mallocInterCodes();
		code3->code.kind = IGOTO;
		code3->code.u.go_to = label2->code.u.label;
		return mergeInterCodes(mergeInterCodes(code1, code2), code3);
 
	}
}










