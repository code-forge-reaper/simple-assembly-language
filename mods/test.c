#include "../interp/sal.h"

bool hello(Register *_) {
  printf("Hello, World!\n");
  return true;
}

void initShared() { registerFunction("hello", hello); }
