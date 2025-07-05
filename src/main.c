// ─── main.c ────────────────────────────────────────────────────────────────
#include "sal.h"
#include <raylib.h>
static char* readFile(const char *fn);


bool hello(Register*) {
    printf("Hello, World!\n");
    return true;
}

bool initWindow(Register * r){
    if (r->top < 2) {
        printf("InitWindow expected the stack to have 3 values at least\n");
        printf("%i\n", r->top);
        return false;
    }
    int width, height;
    const char* title;
    title = ToString(pop(r));
    height = ToInt(pop(r));
    width = ToInt(pop(r));

    InitWindow(width, height, title);
    return true; // true = should continue running, false = should not
}
static int hexNibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static int parseHexByte(const char *s) {
    int hi = hexNibble(s[0]);
    int lo = hexNibble(s[1]);
    if (hi < 0 || lo < 0) return -1;
    return (hi << 4) | lo;
}

Color regToColor(Register * reg){
    int a,r,g,b = -1;
    // if the last thing in the stack before whatever called this, is a int, expect everything to be ints
    if(IsInt(reg->stack[reg->top-1])){
        a = ToInt(pop(reg));
        b = ToInt(pop(reg));
        g = ToInt(pop(reg));
        r = ToInt(pop(reg));
    } else {
        // bit shift
        const char* str = ToString(pop(reg));
        // allowd 0..f
        if(strlen(str) != 8) {printf("RRGGBBAA format\n");exit(1);}
        r = parseHexByte(str);
        g = parseHexByte(str + 2);
        b = parseHexByte(str + 4);
        a = parseHexByte(str + 6);

    }

    if(r < 0) {printf("RRGGBBAA format\n");exit(1);}
    if(g < 0) {printf("RRGGBBAA format\n");exit(1);}
    if(b < 0) {printf("RRGGBBAA format\n");exit(1);}
    if(a < 0) {printf("RRGGBBAA format\n");exit(1);}

    if(r > 255) r = 255;
    if(g > 255) g = 255;
    if(b > 255) b = 255;
    if(a > 255) a = 255;


    return (Color){r,g,b,a};
}

bool clearBackground(Register * reg){
    ClearBackground(regToColor(reg));
    return true;
}

/*
push <x>
push <y>
push <width>
push <height>
push <color>
*/
bool drawRectangle(Register * reg){
    Color c = regToColor(reg);
    int h = ToInt(pop(reg));
    int w = ToInt(pop(reg));
    int y = ToInt(pop(reg));
    int x = ToInt(pop(reg));
    DrawRectangle(x, y, w, h, c);
    return true;
}


bool beginDrawing(Register *){
    BeginDrawing();
    return true;
}
bool endDrawing(Register *){
    EndDrawing();
    return true;
}

bool closeWindow(Register *){
    CloseWindow();
    return true;
}

bool windowShouldClose(Register * reg){
    push(reg, (Item){
        .type = "INT",
        .value = WindowShouldClose() ? "0" : "1"
    });
    return true;
}

bool isKeyDown(Register * reg){
    int k = ToInt(pop(reg));
    push(reg, (Item){
        .type = "INT",
        .value = IsKeyDown(k) ? "1" : "0"
    });
    return true;
}

bool getFrameTime(Register * reg){
    push(reg, (Item){
        .type = "FLOAT",
        .value = (char*)TextFormat("%f",GetFrameTime())
    });
    return true;
}

// if the language is called "SAL"
// then why "initSalt"?
// in portuguese, "sal" means "salt"
void initSalt(){
    registerFunction("hello", hello);
    registerFunction("InitWindow", initWindow);
    registerFunction("ClearBackground", clearBackground);
    registerFunction("BeginDrawing", beginDrawing);
    registerFunction("EndDrawing", endDrawing);
    registerFunction("WindowShouldClose", windowShouldClose);
    registerFunction("CloseWindow", closeWindow);
    registerFunction("DrawRectangle", drawRectangle);

    registerFunction("IsKeyDown", isKeyDown);
    registerFunction("GetFrameTime", getFrameTime);

    set("KEY_W", TextFormat("%i",KEY_W), "INT");
    set("KEY_S", TextFormat("%i",KEY_S), "INT");
    set("KEY_A", TextFormat("%i",KEY_A), "INT");
    set("KEY_D", TextFormat("%i",KEY_D), "INT");
    set("WHITE", "FFFFFFFF", "STRING");
    set("BLACK", "000000FF", "STRING");
    set("RED",   "FF0000FF", "STRING");
    set("GREEN", "00FF00FF", "STRING");
    set("BLUE",  "0000FFFF", "STRING");
}


int main(int argc,char**argv){
    if(argc!=2){fprintf(stderr,"Usage: %s file.sal\n",argv[0]);return 1;}
    char*code=readFile(argv[1]); if(!code){perror("readFile");return 1;}

    initSalt();

    size_t tn; Token*t=tk_tokenize(code,argv[1],&tn);
    int pc; Instr*prog=parse(t,(int)tn,&pc); if(!prog)return 1;
    resolve_labels(prog,pc);
    run(prog,pc);
    free_prog(prog,pc);
    free(code); tk_free_tokens(t,tn);
    return 0;
}


static char*readFile(const char*fn){
    FILE*f=fopen(fn,"rb"); if(!f)return NULL;
    fseek(f,0,SEEK_END); size_t s=ftell(f); rewind(f);
    char*b=malloc(s+1); fread(b,1,s,f); fclose(f); b[s]='\0';
    return b;
}
