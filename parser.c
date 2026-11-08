#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include "parser.h"

u8 final_binary[255];

struct Parser {
  Token *current;
  int next_byte;
} parser;

Token peek() {
  return *parser.current;
}

Token consume() {
  Token t = *parser.current;
  if (t.type == TOKEN_EOF) {
    printf("Error on line %d: Unexpected end of file\n", t.line);
    exit(1);
  }
  parser.current++; // go to next token
  return t;
}

Token expect(TokenType type, char *error_message) {
  Token t = consume();
  if (t.type != type) {
    // Task 8: Include the token that was received
    printf("Error on line %d: %s, got '%.*s'\n", t.line, error_message, t.length, t.source);
    exit(1);
  }
  return t;
}

void init_parser() {
  parser.current = tokens;
  parser.next_byte = 0;
}

void compile_byte(u8 x) {
  final_binary[parser.next_byte] = x;
  parser.next_byte += 1;
}

void parse_move() {
  // Task 5: Writing to RAM -> mov [reg2], reg1 (Output: 0b11000001)
  if (peek().type == TOKEN_LBRACKET) {
    consume(); // consume '['
    Token addr_reg = expect(TOKEN_REGISTER, "Expected register inside brackets");
    if (addr_reg.value != 2) {
      printf("Error on line %d: RAM address must be reg2, got '%.*s'.\n",
             addr_reg.line, addr_reg.length, addr_reg.source);
      exit(1);
    }
    expect(TOKEN_RBRACKET, "Expected closing bracket ']'");
    expect(TOKEN_COMMA, "Expected comma");
    Token src_reg = expect(TOKEN_REGISTER, "Expected source register");
    if (src_reg.value != 1) {
      printf("Error on line %d: Can only write reg1 to RAM, got '%.*s'.\n",
             src_reg.line, src_reg.length, src_reg.source);
      exit(1);
    }
    compile_byte(0b11000001);
    return;
  }

  Token regis = expect(TOKEN_REGISTER, "You can only move into a register or [reg2]");
  expect(TOKEN_COMMA, "Expected comma");

  // Task 6: Reading from RAM -> mov reg1, [reg2] (Output: 0b11000000)
  if (peek().type == TOKEN_LBRACKET) {
    consume(); // consume '['
    if (regis.value != 1) {
      printf("Error on line %d: Can only read from RAM into reg1, got '%.*s'.\n",
             regis.line, regis.length, regis.source);
      exit(1);
    }
    Token addr_reg = expect(TOKEN_REGISTER, "Expected register inside brackets");
    if (addr_reg.value != 2) {
      printf("Error on line %d: RAM address must be reg2, got '%.*s'.\n",
             addr_reg.line, addr_reg.length, addr_reg.source);
      exit(1);
    }
    expect(TOKEN_RBRACKET, "Expected closing bracket ']'");
    compile_byte(0b11000000);
    return;
  }

  Token rhs = consume();

  // Task 7: Compile loading constant into any register
  if (rhs.type == TOKEN_INTEGER) {
    if (rhs.value > 63) {
      printf("Error on line %d: Can't move numbers bigger than 63 into a register, got '%.*s'.\n",
             rhs.line, rhs.length, rhs.source);
      exit(1);
    }

    // Instruction 1: mov reg1, CONSTANT
    compile_byte(rhs.value);

    // Instruction 2 (if target is not reg1): mov regX, reg1
    if (regis.value != 1) {
      int register_a = 1 << 3;
      int register_b = regis.value;
      int opcode = 0b10000000;
      compile_byte(opcode | register_a | register_b);
    }
    return;
  }

  // Compile loading register into another register
  if (rhs.type == TOKEN_REGISTER) {
    int register_a = rhs.value << 3;
    int register_b = regis.value;
    int opcode = 0b10000000;
    compile_byte(opcode | register_a | register_b);
    return;
  }

  // Task 8: Invalid case with received token printed
  printf("Error on line %d: Expected either a register or a constant for move operation, got '%.*s'.\n",
         rhs.line, rhs.length, rhs.source);
  exit(1);
}

// Task 2: Compiling not instruction -> "not reg2, reg1" (01 AAA 101)
void parse_not() {
  Token dest = expect(TOKEN_REGISTER, "Expected destination register for not operation");
  expect(TOKEN_COMMA, "Expected comma");
  Token src = expect(TOKEN_REGISTER, "Expected source register for not operation");

  if (src.value != 1) {
    printf("Error on line %d: Second argument to 'not' must be reg1, got '%.*s'.\n",
           src.line, src.length, src.source);
    exit(1);
  }

  u8 byte = 0b01000101 | (dest.value << 3);
  compile_byte(byte);
}

// Task 4: Compiling or/add/sub/nand/and/xor/eq instructions (01 AAA OOO)
void parse_alu(u8 op_bits) {
  Token dest = expect(TOKEN_REGISTER, "Expected destination register");
  expect(TOKEN_COMMA, "Expected comma");
  Token src1 = expect(TOKEN_REGISTER, "Expected first source register (reg1)");
  if (src1.value != 1) {
    printf("Error on line %d: First source register must be reg1, got '%.*s'.\n",
           src1.line, src1.length, src1.source);
    exit(1);
  }
  expect(TOKEN_COMMA, "Expected comma");
  Token src2 = expect(TOKEN_REGISTER, "Expected second source register (reg2)");
  if (src2.value != 2) {
    printf("Error on line %d: Second source register must be reg2, got '%.*s'.\n",
           src2.line, src2.length, src2.source);
    exit(1);
  }

  u8 byte = 0b01000000 | (dest.value << 3) | op_bits;
  compile_byte(byte);
}

void parse_line() {
  Token t = consume();

  switch (t.type) {
    case TOKEN_MOV:
      parse_move();
      return;
    case TOKEN_OR:
      parse_alu(0b000);
      return;
    case TOKEN_ADD:
      parse_alu(0b001);
      return;
    case TOKEN_SUB:
      parse_alu(0b010);
      return;
    case TOKEN_NAND:
      parse_alu(0b011);
      return;
    case TOKEN_AND:
      parse_alu(0b100);
      return;
    case TOKEN_NOT:
      parse_not();
      return;
    case TOKEN_XOR:
      parse_alu(0b110);
      return;
    case TOKEN_EQ:
      parse_alu(0b111);
      return;
    default:
      // Task 8: Print unexpected token
      printf("Error on line %d: Unexpected token '%.*s'.\n", t.line, t.length, t.source);
      exit(1);
  }
}

void parse_lines() {
  init_parser();

  while (peek().type != TOKEN_EOF) {
    parse_line();
  }
}

void print_binary() {
  for (int i = 0; i < parser.next_byte; i++) {
    for (int j = 7; j >= 0; j--) {
      if (final_binary[i] & (1 << j)) {
        printf("1");
      } else {
        printf("0");
      }
    }
    printf("\n");
  }
}