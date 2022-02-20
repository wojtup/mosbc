/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

#ifndef CODEGEN_H
#define CODEGEN_H

/* Standard library includes */
#include <stdint.h>

/* Custom includes */
#include <ast.h>
#include <table.h>

/* Some constants needed for code generation:
 * LOAD - Load address of the binary
 * INKADDR - Address of INK value in runtime
 * RAMSTART - Address of RAMSTART value in runtime (not the value itself!)
 * WORKPAGE - Address of working page's number
 * ACTIVEPAGE - Address of active page's number
 * VARS - Address of beginning of variables - THIS MAY CHANGE!!!
 * STRVARS - Address of beginning of string variables - THIS MAY CHANGE!!!
 * STRBUF - Temporary buffer for string operations
 * RUNTIMELEN - Length of the runtime (together with jump at the beginning)
 * VERSION - API version. Update accordingly
 */
#define LOAD 0x8000
#define STRADD 0x8003
#define ZERODIV 0x8018
#define PRINTSTR 0x8048
#define INKADDR 0x8098
#define RAMSTART 0x809A
#define WORKPAGE 0x809C
#define ACTIVEPAGE 0x809E
#define VARS 0x4941
#define STRVARS 0x4B76
#define STRBUF 0x7C00
#define RUNTIMELEN 0xA0
#define VERSION 18

/* ============================== PATCH TABLE =============================== */
typedef struct {
	int id;			/* ID of the label (NOT ENTRY'S!!!) */
	uint16_t addr;		/* Address to patch up */
} PatchTableEntry;

typedef struct {
	PatchTableEntry* table;	/* Table */
	int length;		/* Its length */
	int capacity;		/* And its capacity */
} PatchTable;

/* Initialize and free */
void init_patch(PatchTable* p);
void free_patch(PatchTable* p);

/* Add one entry */
void add_patch(PatchTable* p, int id, uint16_t addr);

/* ======================== COMPILED CODE CONTAINER ========================= */
typedef struct {
	char* code;		/* Bytes of compiled code */
	int length;		/* Length of code */
	int capacity;		/* Boy I love them dynamic arrays */
} CompileTarget;

/* Initialize and free */
void init_code(CompileTarget* c);
void free_code(CompileTarget* c);

/* Emit pieces of machine code (takes care of endianness) */
void emit_byte(CompileTarget* c, uint8_t byte);
void emit_word(CompileTarget* c, uint16_t word);
void emit_call(CompileTarget* c, uint16_t target);
void emit_jump(CompileTarget* c, uint16_t target);
void emit_string(CompileTarget* c, const char* str);

/* Patch all jumps when compiling Node n */
void patch_jumps(CompileTarget* c, PatchTable* p, SymbolTable* sym, Node* n);

/* Convenience function to dump compiled code as ASM */
void disassemble(CompileTarget* c);

/* Different helpers for the compiler:
 * compile_error() - Emit error message
 * init_expr_compiler() - Initialize expression compiler
 * init_kword_compiler() - Initialize keyword compiler
 * compile_expression() - Returns true if expr was numeric, false if string
 * compile_keyword() - Compile keyword statement
 * compile_ast() - Compile one AST node
 */
void compile_error(const char* msg, Node* ast);
void init_expr_compiler(StringTable* str);
void init_kword_compiler(StringTable* str, SymbolTable* sym, PatchTable* p);
bool compile_expression(Node* ast, CompileTarget* code);
void compile_keyword(Node* ast, CompileTarget* code);
void compile_ast(Node* ast, CompileTarget* code);

/* Main function of code generation: compiler */
void compile(Node* ast, CompileTarget* code, StringTable* str, SymbolTable* t);

#endif
