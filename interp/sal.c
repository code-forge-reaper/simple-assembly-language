#define CREATE_TOKENIZER
#include "sal.h"
// there might be memory leaks here
// but since we can't stay memory safe
// and functional at the same time,
// we have to suffer with it
// but like, at most, it is ~20kb of ram that's leaking(i think), so who cares

/* ────── Registers ────── */
#define STK   (registers[current_reg].stack)
#define TOP   (registers[current_reg].top)
#define PUSH(i) (STK[TOP++] = (i))
#define POP() (STK[--TOP])

/* ────── Globals ────── */
#define MAX_REGISTERS 10
static Register registers[MAX_REGISTERS] = {0};
static int      current_reg = 0;
static Item     memory[20]    = {0};
#define MAX_VARS 128
static Var      vars[MAX_VARS] = {0};
static int      var_count      = 0;

#define MAX_FUNCS 64
CFuncMap funcs[MAX_FUNCS] = {0};
int funcCount = 0;
void free_item(Item i) {
    free(i.type);
    free(i.value);
}

__attribute__((visibility("default")))
void set(const char* name, const char* value, const char* type) {
    Var *ex = find_var(name);
    if (ex) {
        free(ex->value.type);
        free(ex->value.value);
        ex->value = (Item){ strdup(type), strdup(value) };
    } else {
        if (var_count >= MAX_VARS) {
            fprintf(stderr, "too many vars\n");
            exit(1);
        }
        vars[var_count++] = (Var){
            .name  = strdup(name),
            .value = (Item){ strdup(type), strdup(value) }
        };
    }
}

__attribute__((visibility("default")))
void registerFunction(const char* name, CFunc func) {
    if (funcCount >= MAX_FUNCS) {
        fprintf(stderr, "Too many functions registered\n");
        exit(1);
    }
    funcs[funcCount++] = (CFuncMap){ name, func };
}

int ToInt(Item i){
    if(!IsInt(i)) {printf("Error: not an int\n"); exit(1);}
    return atoi(i.value);
}

float ToFloat(Item i){
    if(!IsFloat(i)) {printf("Error: not a float\n"); exit(1);}
    return atof(i.value);
}
char *ToString(Item i){
    if(!IsString(i)) {printf("Error: not a string\n"); exit(1);}
    return i.value;
}

bool IsInt(Item i){
    if (!i.type) {
        fprintf(stderr, "IsInt: NULL i.type\n");
        fprintf(stderr, "i.value = %s\n", i.value);
        for(int i = 0; i < TOP; i++){
            printf("%s %s\n", STK[i].type, STK[i].value);
        }
        return false;
    }
    return streq(i.type, "INT");
}

bool IsFloat(Item i){
    if (!i.type) {
        fprintf(stderr, "IsFloat: NULL i.type\n");
        fprintf(stderr, "i.value = %s\n", i.value);
        for(int i = 0; i < TOP; i++){
            printf("%s %s\n", STK[i].type, STK[i].value);
        }
        return false;
    }
    return streq(i.type, "FLOAT");
}
bool IsString(Item i){
    return streq(i.type, "STRING");
}

// Find variable by name
Var *find_var(const char *n) {
    for (int i = 0; i < var_count; i++)
        if (streq(vars[i].name, n))
            return &vars[i];
    return NULL;
}

// Validate integer literal
int is_integer(const char *s) {
    if (*s=='+'||*s=='-') s++;
    if (!*s) return 0;
    for (; *s; s++)
        if (!isdigit((unsigned char)*s)) return 0;
    return 1;
}

// Validate float literal
int is_float(const char *s) {
    char *end;
    errno = 0;
    strtod(s, &end);
    if (errno) return 0;
    return *s != '\0' && *end == '\0';
}

const char* getStrType(TokenType t){
    const char* tt = "";
    switch (t) {
    case TOK_KEYWORD:   tt = "KEYWORD";      break;
    case TOK_INT:       tt = "INT";          break;
    case TOK_FLOAT:     tt = "FLOAT";        break;

    case TOK_STR:       tt = "STRING";       break;
    case TOK_ID:        tt = "ID";           break;
    case TOK_CHAR:      tt = "CHAR";         break;
    case TOK_PP:        tt = "PP";           break;
    case TOK_ATTR:      tt = "ATTR";         break;
    case TOK_ELLIPSIS:  tt = "ELLIPSIS";     break;
    case TOK_PUNCT:     tt = "PUNCT";        break;
    case TOK_OP:        tt = "OP";           break;
    case TOK_MAX:       perror("TOK_MAX");   break;
    };
    return tt;
}

/* ────── Phase 1: Parse Tokens → raw Instr[] ────── */
Instr *parse(Token *tokens, int tn, int *out_count) {
    #define NXT(n)   Token *n = &tokens[++i];

    Instr *prog = malloc(sizeof(Instr) * tn);
    int pc = 0;
    for (int i = 0; i < tn; i++) {
        Token *t = &tokens[i];
        #define ADD(op,a,ty) prog[pc++] = (Instr){op, a, ty, -1}
        if(t->type == TOK_KEYWORD){

        if (streq(t->value, "reg")) {
            NXT(n);
            ADD(OP_REG,  strdup(n->value), NULL);
        }
        else if (streq(t->value, "push")) {
            NXT(v);
            ADD(OP_PUSH, strdup(v->value), strdup(getStrType(v->type)));
        }
        else if (streq(t->value, "pop")) {
            ADD(OP_POP, NULL, NULL);
        }
        else if (streq(t->value, "add"))   ADD(OP_ADD,    NULL, NULL);
        else if (streq(t->value, "sub"))   ADD(OP_SUB,    NULL, NULL);
        else if (streq(t->value, "mult"))  ADD(OP_MULT,   NULL, NULL);
        else if (streq(t->value, "div"))   ADD(OP_DIV,    NULL, NULL);
        else if (streq(t->value, "eq"))    ADD(OP_EQ,     NULL, NULL);
        else if (streq(t->value, "lt"))    ADD(OP_LT,     NULL, NULL);
        else if (streq(t->value, "gt"))    ADD(OP_GT,     NULL, NULL);
        else if (streq(t->value, "print")) ADD(OP_PRINT,  NULL, NULL);

        else if (streq(t->value, "set")) {
            NXT(n);
            ADD(OP_SET, strdup(n->value), NULL);
        }
        else if (streq(t->value, "get")) {
            NXT(n);
            ADD(OP_GET, strdup(n->value), NULL);
        }

        else if (streq(t->value, "save"))  ADD(OP_SAVE,   NULL, NULL);
        else if (streq(t->value, "load")) {
            NXT(n);
            ADD(OP_LOAD, strdup(n->value), NULL);
        }

        else if (streq(t->value, "jmp")) {
            NXT(n);
            ADD(OP_JMP,  strdup(n->value), NULL);
        }
        else if (streq(t->value, "jz")) {
            NXT(n);
            ADD(OP_JZ,   strdup(n->value), NULL);
        }
        else if (streq(t->value, "jnz")) {
            NXT(n);
            ADD(OP_JNZ,  strdup(n->value), NULL);
        }
        else if(streq(t->value, "loadShared")){
            ADD(OP_LOADSHARED, NULL, NULL);
        }
        else if (streq(t->value, "label")) {
            NXT(n);
            ADD(OP_LABEL, strdup(n->value), NULL);
        }
        else if (streq(t->value, "concat")) {
            ADD(OP_CONCAT, NULL, NULL);
        }
        else if (streq(t->value, "cast")) {
            NXT(nt);
            ADD(OP_CAST, strdup(nt->value), NULL);
        }
        else if (streq(t->value, "dup")) {
            ADD(OP_DUP, NULL, NULL);
        }
        else if (streq(t->value, "read")) {
            NXT(nt);
            ADD(OP_READ, strdup(nt->value), NULL);
        }
        else if (streq(t->value, "call")) { // "call <c function name>"
            NXT(n);
            ADD(OP_CALL, strdup(n->value), NULL);
        }
        }
        else {
            fprintf(stderr, "Parse error: unexpected '%s'\n", t->value);
            free(prog);
            return NULL;
        }
    }
    *out_count = pc;
    return prog;
}

/* ────── Phase 1.5: Resolve Labels ────── */
void resolve_labels(Instr *prog, int pc_count) {
    for (int i = 0; i < pc_count; i++)
        if (prog[i].op == OP_LABEL)
            prog[i].target = i;

    for (int i = 0; i < pc_count; i++) {
        if (prog[i].op == OP_JMP ||
            prog[i].op == OP_JZ  ||
            prog[i].op == OP_JNZ) {
            char *lbl = prog[i].arg;
            int dest = -1;
            for (int j = 0; j < pc_count; j++) {
                if (prog[j].op == OP_LABEL && streq(prog[j].arg, lbl)) {
                    dest = j;
                    break;
                }
            }
            if (dest < 0) {
                fprintf(stderr, "Error: undefined label '%s'\n", lbl);
                exit(1);
            }
            prog[i].target = dest;
        }
    }
}

/* ────── Phase 2: Execute Instr[] ────── */
void run(Instr *prog, int pc_count) {
    int ip = 0;
    while (ip < pc_count) {
        Instr *ins = &prog[ip];
        switch (ins->op) {
            case OP_REG: {
                int r = atoi(ins->arg);
                if (r<0||r>=MAX_REGISTERS) {
                    fprintf(stderr,"Error: reg %d oob\n",r);
                    return;
                }
                current_reg = r; ip++;
            } break;

            case OP_PUSH: {
                Item it = { strdup(ins->type), strdup(ins->arg) };
                STK[TOP++] = it; ip++;
            } break;

            case OP_POP:
                if (TOP>0) TOP--;
                ip++;
                break;

            case OP_DUP: {
                if (TOP<1) { fprintf(stderr,"dup underflow\n"); return; }
                Item a = STK[TOP-1];
                STK[TOP++] = (Item){ strdup(a.type), strdup(a.value) };
                ip++;
            } break;

            case OP_ADD: case OP_SUB: case OP_MULT: case OP_DIV:
            case OP_EQ:  case OP_LT:  case OP_GT: {
                if (TOP<2) { fprintf(stderr,"underflow\n"); return; }
                Item b=STK[--TOP], a=STK[--TOP];
                bool isint = (streq(a.type,"INT")&&streq(b.type,"INT"));
                bool isfloat = (streq(a.type,"FLOAT")&&streq(b.type,"FLOAT"));
                if (!isint&&!isfloat) {
                    fprintf(stderr,"Type error\n");
                    fprintf(stderr,"a=%s b=%s\n",a.type,b.type);
                    return;
                }
                if(isint){
                    int av=atoi(a.value), bv=atoi(b.value), rv=0;
                    switch(ins->op){
                        case OP_ADD: rv=av+bv; break;
                        case OP_SUB: rv=av-bv; break;
                        case OP_MULT:rv=av*bv; break;
                        case OP_DIV: rv=bv?av/bv:0; break;
                        case OP_EQ:  rv=(av==bv); break;
                        case OP_LT:  rv=(av< bv); break;
                        case OP_GT:  rv=(av> bv); break;
                        default: break;
                    }
                    char buf[32];
                    sprintf(buf,"%i",rv);
                    STK[TOP++] = (Item){ strdup(a.type), strdup(buf) };
                }
                else{
                    float av=atof(a.value), bv=atof(b.value), rv=0;
                    switch(ins->op){
                        case OP_ADD: rv=av+bv; break;
                        case OP_SUB: rv=av-bv; break;
                        case OP_MULT:rv=av*bv; break;
                        case OP_DIV: rv=bv?av/bv:0; break;
                        case OP_EQ:  rv=(av==bv); break;
                        case OP_LT:  rv=(av< bv); break;
                        case OP_GT:  rv=(av> bv); break;
                        default: break;
                    }
                    char buf[32]; snprintf(buf,32,"%f",rv);
                    STK[TOP++] = (Item){ strdup(a.type), strdup(buf) };
                }
                ip++;
            } break;

            case OP_PRINT:
                if (TOP>0) printf("%s\n", STK[TOP-1].value);
                ip++;
                break;

            case OP_SET: {
                if (TOP==0){fprintf(stderr,"set underflow\n");return;}
                Item v=STK[--TOP]; Var *ex=find_var(ins->arg);
                if (ex) {
                    free(ex->value.type); free(ex->value.value);
                    ex->value=(Item){strdup(v.type),strdup(v.value)};
                } else {
                    if (var_count>=MAX_VARS){fprintf(stderr,"too many vars\n");return;}
                    vars[var_count++] = (Var){
                        .name  = strdup(ins->arg),
                        .value = (Item){strdup(v.type),strdup(v.value)}
                    };
                }
                ip++;
            } break;

            case OP_GET: {
                Var *v=find_var(ins->arg);
                if (!v){fprintf(stderr,"undef var\n");return;}
                STK[TOP++] = (Item){ strdup(v->value.type), strdup(v->value.value) };
                ip++;
            } break;

            case OP_SAVE:
                if (TOP>0) memory[current_reg]=STK[TOP-1];
                ip++;
                break;

            case OP_LOAD: {
                int m=atoi(ins->arg);
                if(m<0||m>=20){fprintf(stderr,"mem oob\n");return;}
                STK[TOP++]=memory[m]; ip++;
            } break;

            case OP_JMP:
                ip = ins->target; break;

            case OP_JZ: {
                if (TOP==0){fprintf(stderr,"jz underflow\n");return;}
                Item v=STK[--TOP];
                ip = atoi(v.value)==0 ? ins->target : ip+1;
            } break;
            case OP_LOADSHARED:{
                Item pathItem = pop(&registers[current_reg]);
                const char *path = ToString(pathItem);

                void *handle = dlopen(path, RTLD_NOW);
                if (!handle) {
                    fprintf(stderr, "loadShared: Failed to open '%s': %s\n", path, dlerror());
                    exit(1);
                }

                // Try to find an init function
                void (*init_func)(void) = dlsym(handle, "initShared");
                if (!init_func) {
                    fprintf(stderr, "loadShared: '%s' missing 'initShared' symbol\n", path);
                    exit(1);
                }

                // Call it — this function should register SAL functions/values
                init_func();

                ip++;
                break;

            }break;

            case OP_JNZ: {
                if (TOP==0){fprintf(stderr,"jnz underflow\n");return;}
                Item v=STK[--TOP];
                ip = atoi(v.value)!=0 ? ins->target : ip+1;
            } break;

            case OP_LABEL:
                ip++; break;

            case OP_CONCAT: {
                if (TOP<2){fprintf(stderr,"concat underflow\n");return;}
                Item b=STK[--TOP], a=STK[--TOP];
                if (!streq(a.type,"STRING")||!streq(b.type,"STRING")){
                    fprintf(stderr,"concat expects STRINGs\n");return;
                }
                size_t la=strlen(a.value), lb=strlen(b.value);
                char *buf=malloc(la+lb+1);
                memcpy(buf,a.value,la); memcpy(buf+la,b.value,lb); buf[la+lb]='\0';
                STK[TOP++] = (Item){ strdup("STRING"), buf };
                free(a.type); free(a.value);
                free(b.type); free(b.value);
                ip++;
            } break;

            case OP_CAST: {
                if (TOP<1){fprintf(stderr,"cast underflow\n");return;}
                Item v=STK[--TOP];
                free(v.type);
                v.type = strdup(ins->arg);
                STK[TOP++] = v;
                ip++;
            } break;

            case OP_CALL: {
                const char* name = ins->arg;
                bool exists = false;
                for(int i =0; i < funcCount; i++){
                    if(streq(funcs[i].name,name)){
                        if(!funcs[i].func(&registers[current_reg])) return;
                        exists = true;
                        break;
                    }
                }

                if(!exists){
                    fprintf(stderr,"Error: undefined function '%s'\n",name);
                    return;
                }

                ip++;
            } break;

            case OP_READ: {
                char buf[256];
                if (!fgets(buf,sizeof(buf),stdin)){
                    fprintf(stderr,"read error\n");return;
                }
                size_t L=strlen(buf);
                if (L&&buf[L-1]=='\n') buf[L-1]='\0';
                // validate
                if (streq(ins->arg,"INT") || streq(ins->arg,"FLOAT")){
                    if (!is_integer(buf)&&!is_float(buf)){
                        fprintf(stderr,"Error: expected int or float, but got '%s'\n",buf);
                        return;
                    }
                }
                // push
                STK[TOP++] = (Item){ strdup(ins->arg), strdup(buf) };
                ip++;
            } break;
        }
    }
}

/* ────── Cleanup ────── */
void free_prog(Instr *p,int n){
    for(int i=0;i<n;i++){
        free(p[i].arg);
        free(p[i].type);
    }
    free(p);
}
