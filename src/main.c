// ─── main.c ────────────────────────────────────────────────────────────────
#include "../interp/sal.h"
static char *readFile(const char *fn);

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s file.sal\n", argv[0]);
    return 1;
  }
  char *code = readFile(argv[1]);
  if (!code) {
    perror("readFile");
    return 1;
  }

  size_t tn;
  Token *t = tk_tokenize(code, argv[1], &tn);
  int pc;
  Instr *prog = parse(t, (int)tn, &pc);
  if (!prog)
    return 1;
  resolve_labels(prog, pc);
  run(prog, pc);
  free_prog(prog, pc);
  free(code);
  tk_free_tokens(t, tn);
  return 0;
}

static char *readFile(const char *fn) {
  FILE *f = fopen(fn, "rb");
  if (!f)
    return NULL;
  fseek(f, 0, SEEK_END);
  size_t s = ftell(f);
  rewind(f);
  char *b = malloc(s + 1);
  fread(b, 1, s, f);
  fclose(f);
  b[s] = '\0';
  return b;
}
