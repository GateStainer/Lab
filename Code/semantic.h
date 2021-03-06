#ifndef SEMANTIC_H_
#define SEMANTIC_H_

#include "TreeNode.h"
#include "syntax.tab.h"
#include "symbolTable.h"
#include "type.h"

extern void semanticTest(TreeNode* root);
extern void semanticTest(TreeNode* root);
extern void visitProgram(TreeNode* root);
extern void visitExtDefList(TreeNode* root);
extern void visitExtDef(TreeNode* root);
extern TypeP visitSpecifier(TreeNode* root);
extern FieldListP visitDefList(TreeNode* root, bool inStruct);
extern FieldListP visitDef(TreeNode* root, bool inStruct);
extern FieldListP visitVarDec(TreeNode* root, TypeP type, bool inStruct);
extern TypeP visitFunDec(TreeNode* root, TypeP type);
extern void visitCompSt(TreeNode* root, TypeP type);
extern void visitStmt(TreeNode* root, TypeP returntype);
extern TypeP visitFunDec(TreeNode* root, TypeP type);
extern void visitExtDecList(TreeNode* root, TypeP type);
extern TypeP visitExp(TreeNode* root);


extern bool sameType(TypeP type1, TypeP type2);



#endif
