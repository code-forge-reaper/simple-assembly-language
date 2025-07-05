#include "sal.h"

bool hello(Register*){
	printf("Hello, World!\n");
	return true;
}

void initShared()
{
	registerFunction("hello", hello);
}
