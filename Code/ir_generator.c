#include <stdlib.h>
#include<stdio.h>
#include "ir_generator.h"

int temp_index = -1;
int label_index = -1;

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

