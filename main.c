#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "scanner.h"
#include "parser.h"

int main(int argc, char *argv[]) {
  char *filename = "example.asm";

  // Task 3: Flags to the program (e.g., -f <file> or direct filename argument)
  for (int i = 1; i < argc; i++) {
    if ((strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0) && i + 1 < argc) {
      filename = argv[i + 1];
      i++;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printf("Usage: %s [-f <filename>]\n", argv[0]);
      return 0;
    } else if (argv[i][0] != '-') {
      filename = argv[i];
    }
  }

  init_scanner(filename);
  scan_tokens();
  parse_lines();
  print_binary();
  return 0;
}