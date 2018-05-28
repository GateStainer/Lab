#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ir_structure.h"
#include "ir_generator.h"

InterCodes* ir_head = NULL;
InterCodes* ir_tail = NULL;


char* getOperandName(Operand operand)
{
	char* result = NULL;
	switch(operand->kind)
	{
		case VARIABLE: return operand->u.var_name;
		case CONSTANT:
			result = (char*)malloc(32);
			result[0] = '#';
			sprintf(result+1, "%d", operand->u.value);
			return result;
		case ADDRESS:
			result = (char*)malloc(32);
			result[0] = '*';
			strcpy(result+1, operand->u.addr_name);
			return result;
		case REFERENCE: 
			result = (char*)malloc(32);
			result[0] = '&';
			strcpy(result+1, operand->u.refer_name);
			return result;
		case TEMP: return get_temp_name(operand->u.temp_no);
		case LABELNO: return get_label_name(operand->u.label_no); 
		default:
			printf("Unexpected value in operand->kind in getOperandName()\n");
			exit(-1);
	}
}

Operand mallocOperand(int kind, char* name)
{
	Operand result = (Operand)malloc(sizeof(struct Operand_));
	result->kind = kind;
	switch(kind)
	{
		case VARIABLE: result->u.var_name = name; break;
		case CONSTANT: result->u.value = atoi(name); break;
		case ADDRESS: result->u.addr_name = name; break;
		case REFERENCE: result->u.refer_name = name; break;
		case TEMP: result->u.temp_no = atoi(name); break;
		case LABELNO: result->u.label_no = atoi(name); break;
		default:
			printf("Unexpected value in kind in mallocOperand()\n");
			exit(-1);
	}
	return result;
}

InterCodes* mallocInterCodes()
{
	InterCodes* temp = (InterCodes*)malloc(sizeof(InterCodes));
	temp->prev = NULL;
	temp->next = NULL;
	return temp;
}

InterCodes* mergeInterCodes(InterCodes* code1, InterCodes* code2)
{	
	if(code1 == NULL)
		return code2;
	else if(code2 == NULL)
		return code1;
	else
	{
		InterCodes* p1 = code1;
		InterCodes* p2 = code1;
		while(p2 != NULL)
		{
			p1 = p2;
			p2 = p2->next;
		}
		p1->next = code2;
		code2->prev = p1;
		return code1;
	}
}

void writeInterCodes(char* filename)
{
	InterCodes* p = ir_head;
	if(p == NULL)
	{
		printf("There is no Intermediate Representation!\n");
		return;
	}
	FILE* fp = fopen(filename, "w");
	if(fp == NULL)
	{
		printf("Can't open the file to write IR!\n");
		return;
	}
	while(p != NULL)
	{
		switch(p->code.kind)
		{
			case ILABEL:
				fprintf(fp, "LABEL %s :\n", get_label_name(p->code.u.label->u.label_no));
				break;
			case IFUNCTION:
				fprintf(fp, "FUNCTION %s :\n", getOperandName(p->code.u.func));
				break;
			case IASSIGN:
				fprintf(fp, "%s := %s\n", getOperandName(p->code.u.assign.left), getOperandName(p->code.u.assign.right));
				break;
			case IADD:
				fprintf(fp, "%s := %s + %s\n", getOperandName(p->code.u.binop.result), getOperandName(p->code.u.binop.op1), getOperandName(p->code.u.binop.op2));
				break;
			case ISUB:
				fprintf(fp, "%s := %s - %s\n", getOperandName(p->code.u.binop.result), getOperandName(p->code.u.binop.op1), getOperandName(p->code.u.binop.op2));
				break;
			case IMUL:
				fprintf(fp, "%s := %s * %s\n", getOperandName(p->code.u.binop.result), getOperandName(p->code.u.binop.op1), getOperandName(p->code.u.binop.op2));
				break;
			case IDIV:
				fprintf(fp, "%s := %s / %s\n", getOperandName(p->code.u.binop.result), getOperandName(p->code.u.binop.op1), getOperandName(p->code.u.binop.op2));
				break;
			case IGOTO:
				fprintf(fp, "GOTO %s\n", get_label_name(p->code.u.go_to->u.label_no));
				break;
			case IIFSTMT:
				fprintf(fp, "IF %s %s %s GOTO %s\n", getOperandName(p->code.u.ifstmt.left), p->code.u.ifstmt.relop, getOperandName(p->code.u.ifstmt.right), getOperandName(p->code.u.ifstmt.label));
				break;
			case IRETURN:
				fprintf(fp, "RETURN %s\n", getOperandName(p->code.u.return_val));
				break;
			case IDEC:
				fprintf(fp, "DEC %s %d\n", getOperandName(p->code.u.dec.left), p->code.u.dec.size);
				break;
			case IARG:
				fprintf(fp, "ARG %s\n", getOperandName(p->code.u.arg));
				break;
			case ICALL:
				fprintf(fp, "%s := CALL %s\n", getOperandName(p->code.u.callfunc.left), getOperandName(p->code.u.callfunc.right));
				break;
			case IPARAM:
				fprintf(fp, "PARAM %s\n", getOperandName(p->code.u.param));
			case IREAD:
				fprintf(fp, "READ %s\n", getOperandName(p->code.u.read_val));
				break;
			case IWRITE:
				fprintf(fp, "WRITE %s\n", getOperandName(p->code.u.write_val));
				break;
			default:
				printf("Unexpected kind in writeInterCodes()\n");
				return;
		}
		p = p->next;
	}
	fclose(fp);
	printf("Complete writing IR to file\n");
	return;
}








