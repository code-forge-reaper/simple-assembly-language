#pragma once
// this is needed for tokenizer.h
// since it is a general purpose, header-only library
// that you can change what are considered keywords
// it was meant for a neon rewrite, but i gave up on that idea,
// and used it anywhere where tokenizing is needed
#define TK_KEYWORDS_LIST {\
  "reg","push","pop","add","sub","mult","div",\
  "eq","lt","gt","print","set","get","load","save",\
  "jmp","jz","jnz","label","concat","cast","dup",\
  "read", "call", "loadShared"\
}

#include "../../libs/tokenizer.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
/* ────── Core Types ────── */
typedef struct {
    char *type;    // e.g. "NUMBER", "STRING", etc.
    char *value;   // literal text
} Item;

typedef struct {
    char *name;
    Item  value;
} Var;

typedef struct {
    Item stack[1024];
    int  top;
} Register;

/* ────── IR Instruction Set ────── */
typedef enum {
    OP_REG, OP_PUSH, OP_POP,
    OP_ADD, OP_SUB, OP_MULT, OP_DIV,
    OP_EQ,  OP_LT,  OP_GT,
    OP_PRINT,
    OP_SET, OP_GET,
    OP_SAVE, OP_LOAD,
    OP_JMP, OP_JZ,  OP_JNZ,
    OP_LABEL,
    OP_CONCAT,
    OP_CAST,
    OP_DUP,
    OP_READ,
    OP_CALL,
    OP_LOADSHARED
} OpCode;

typedef struct {
    OpCode op;
    char  *arg;    // strdup’d: register#, var name, literal, memory/label name, cast-target, or read-type
    char  *type;   // for OP_PUSH: literal’s type, else NULL
    int    target; // for jumps: filled in label resolution
} Instr;


/* ────── Helpers ────── */
#define streq(a,b) (strcmp((a),(b))==0)

static inline Item pop(Register *r) {
    if (r->top <= 0) {
        fprintf(stderr, "Stack underflow\n");
        exit(1);
    }
    return r->stack[--r->top];
}

static inline void push(Register *r, Item i) {
    if (r->top >= 1023) {
        fprintf(stderr, "Stack overflow\n");
        exit(1);
    }
    r->stack[r->top++] = i;
}


typedef bool (*CFunc)(Register*);

typedef struct {
    const char* name;
    CFunc func;
} CFuncMap;


// functions

Var *find_var(const char *n);
int is_integer(const char *s);
int is_float(const char *s);
Instr *parse(Token *tokens, int tn, int *out_count);
void resolve_labels(Instr *prog, int pc_count);
void run(Instr *prog, int pc_count);
void free_prog(Instr *p,int n);
void registerFunction(const char* name, CFunc func);
void set(const char* name, const char* value, const char* type);

bool IsInt(Item i);
bool IsFloat(Item i);
bool IsString(Item i);

int ToInt(Item i);
float ToFloat(Item i);
char *ToString(Item i);
void free_item(Item i);
