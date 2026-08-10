#pragma once

static int hexNibble(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return 10 + (c - 'a');
  if (c >= 'A' && c <= 'F')
    return 10 + (c - 'A');
  return -1;
}

static int parseHexByte(const char *s) {
  int hi = hexNibble(s[0]);
  int lo = hexNibble(s[1]);
  if (hi < 0 || lo < 0)
    return -1;
  return (hi << 4) | lo;
}
