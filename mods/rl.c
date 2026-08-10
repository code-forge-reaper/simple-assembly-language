#include "../interp/sal.h"
#include "../src/helper.h"
#include <raylib.h>

bool initWindow(Register *r) {
  if (r->top < 2) {
    printf("InitWindow expected the stack to have 3 values at least\n");
    printf("%i\n", r->top);
    return false;
  }
  int width, height;
  const char *title;
  title = ToString(pop(r));
  height = ToInt(pop(r));
  width = ToInt(pop(r));

  InitWindow(width, height, title);
  return true; // true = should continue running, false = should not
}
Color regToColor(Register *reg) {
  int a, r, g, b = -1;
  // if the last thing in the stack before whatever called this, is a int,
  // expect everything to be ints
  if (IsInt(reg->stack[reg->top - 1])) {
    a = ToInt(pop(reg));
    b = ToInt(pop(reg));
    g = ToInt(pop(reg));
    r = ToInt(pop(reg));
  } else {
    // bit shift
    const char *str = ToString(pop(reg));
    // allowd 0..f
    if (strlen(str) != 8) {
      printf("RRGGBBAA format\n");
      exit(1);
    }
    r = parseHexByte(str);
    g = parseHexByte(str + 2);
    b = parseHexByte(str + 4);
    a = parseHexByte(str + 6);
  }

  if (r < 0) {
    printf("RRGGBBAA format\n");
    exit(1);
  }
  if (g < 0) {
    printf("RRGGBBAA format\n");
    exit(1);
  }
  if (b < 0) {
    printf("RRGGBBAA format\n");
    exit(1);
  }
  if (a < 0) {
    printf("RRGGBBAA format\n");
    exit(1);
  }

  if (r > 255)
    r = 255;
  if (g > 255)
    g = 255;
  if (b > 255)
    b = 255;
  if (a > 255)
    a = 255;

  return (Color){r, g, b, a};
}

bool clearBackground(Register *reg) {
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
bool drawRectangle(Register *reg) {
  Color c = regToColor(reg);
  int h = ToInt(pop(reg));
  int w = ToInt(pop(reg));
  int y = ToInt(pop(reg));
  int x = ToInt(pop(reg));
  DrawRectangle(x, y, w, h, c);
  return true;
}

bool beginDrawing(Register *_) {
  BeginDrawing();
  return true;
}
bool endDrawing(Register *_) {
  EndDrawing();
  return true;
}

bool closeWindow(Register *_) {
  CloseWindow();
  return true;
}

bool windowShouldClose(Register *reg) {
  push(reg, (Item){.type = "INT", .value = WindowShouldClose() ? "0" : "1"});
  return true;
}

bool isKeyDown(Register *reg) {
  int k = ToInt(pop(reg));
  push(reg, (Item){.type = "INT", .value = IsKeyDown(k) ? "1" : "0"});
  return true;
}

bool getFrameTime(Register *reg) {
  push(reg, (Item){.type = "FLOAT",
                   .value = (char *)TextFormat("%f", GetFrameTime())});
  return true;
}

// if the language is called "SAL"
// then why "initSalt"?
// in portuguese, "sal" means "salt"
void initShared() {
  registerFunction("InitWindow", initWindow);
  registerFunction("ClearBackground", clearBackground);
  registerFunction("BeginDrawing", beginDrawing);
  registerFunction("EndDrawing", endDrawing);
  registerFunction("WindowShouldClose", windowShouldClose);
  registerFunction("CloseWindow", closeWindow);
  registerFunction("DrawRectangle", drawRectangle);

  registerFunction("IsKeyDown", isKeyDown);
  registerFunction("GetFrameTime", getFrameTime);

  set("KEY_W", TextFormat("%i", KEY_W), "INT");
  set("KEY_S", TextFormat("%i", KEY_S), "INT");
  set("KEY_A", TextFormat("%i", KEY_A), "INT");
  set("KEY_D", TextFormat("%i", KEY_D), "INT");
  set("WHITE", "FFFFFFFF", "STRING");
  set("BLACK", "000000FF", "STRING");
  set("RED", "FF0000FF", "STRING");
  set("GREEN", "00FF00FF", "STRING");
  set("BLUE", "0000FFFF", "STRING");
}
