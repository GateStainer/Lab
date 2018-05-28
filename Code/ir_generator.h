#ifndef IR_GENERATOR_H_
#define IR_GENERATOR_H_
#include "TreeNode.h"
#include "type.h"
#include "ir_structure.h"

int getSize(TypeP p);
char* get_temp_name(int index);
char* get_label_name(int index);
void generate_ir(char* filename);
InterCodes* translate_Program(TreeNode* root);
InterCodes* translate_FunDec(TreeNode* root);
InterCodes* translate_CompSt(TreeNode* root);
InterCodes* translate_DefList(TreeNode* root);
InterCodes* translate_Stmt(TreeNode* root);
InterCodes* translate_StmtList(TreeNode* root);
InterCodes* translate_Exp(TreeNode* root, Operand place);
InterCodes* translate_Args(TreeNode* root, ArgListNode* arg_list);
InterCodes* translate_Cond(TreeNode* root, InterCodes* label1, InterCodes* label2);


#endif
