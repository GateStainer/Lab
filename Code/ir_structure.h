#ifndef IR_STRUCTURE_H_
#define IR_STRUCTURE_H_

struct Operand_
{
	enum { VARIABLE, CONSTANT, ADDRESS, REFERENCE, TEMP, LABELNO} kind;
	union
	{
		char* var_name;
		int value;
		char* addr_name;
		char* refer_name;
		int temp_no;
		int label_no;
	} u;
};
typedef struct Operand_* Operand;

struct InterCode
{
	enum {ILABEL, IFUNCTION, IASSIGN, IADD, ISUB, IMUL, IDIV, IGOTO, IIFSTMT, IRETURN, IDEC, IARG, ICALL, IPARAM, IREAD, IWRITE } kind;
	union
	{
		Operand label;
		Operand func;
		struct {Operand left, right; } assign;
		struct {Operand result, op1, op2; } binop;
		Operand go_to;
		struct {Operand left, right; char relop[3]; Operand label;} ifstmt;
		Operand return_val;
		struct {Operand left; int size;} dec;
		Operand arg;
		struct {Operand left, right;} callfunc;
		Operand param;
		Operand read_val;
		Operand write_val;
	} u;
};
typedef struct InterCode InterCode;

struct InterCodes
{
	InterCode code;
	struct InterCodes* prev;
	struct InterCodes* next;
};
typedef struct InterCodes InterCodes;

extern InterCodes* head;
extern InterCodes* tail;

struct ArgListNode
{
	Operand operand;
	struct ArgListNode* next;
};
typedef struct ArgListNode ArgListNode;

Operand mallocOperand(int kind, char* name);
InterCodes* mallocInterCodes();
InterCodes* mergeInterCodes(InterCodes* code1, InterCodes* code2);
void writeInterCodes(char* filename);





#endif
