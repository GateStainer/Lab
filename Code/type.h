#ifndef TYPE_H_
#define TYPE_H_


struct FieldList_
{
	char* name;
	struct Type_* type;
	struct FieldList_* tail;
};
typedef struct FieldList_ FieldList_;

struct Type_
{
	enum { BASIC, ARRAY, STRUCTURE, FUNCTION } kind;
	union
	{
		int basic;
		struct { struct Type_* elem; int size; } array;
		struct FieldList_* structure;
	} u;
};

typedef struct Type_ Type_;
typedef struct Type_* TypeP;
typedef struct FieldList_* FieldListP;



#endif