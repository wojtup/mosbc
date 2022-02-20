/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

/* Standard library includes */
#include <stdio.h>
#include <stdbool.h>

/* Custom includes */
#include <lexer.h>
#include <parser.h>
#include <table.h>
#include <codegen.h>
#include <runtime.h>
#include <util.h>

static PatchTable* patches;
static SymbolTable* symbols;

void compile_error(const char* msg, Node* current)
{
	printf("\x1B[31mError (codegen)\x1B[0m: %s at line: %d.\n", msg,
		current->line);
	raise_error();
}

/* =========================== COMPILER FUNCTIONS =========================== */
void compile_assign(Node* ast, CompileTarget* code)
{
	bool expr = compile_expression(ast->op2, code);
	bool type = (ast->op1->attribute == TOKEN_NUMERIC_VARIABLE) ?
			true : false;

	/* Check typing */
	if (expr != type) {
		compile_error("Assigning invalid type", ast);
		return;
	}

	/* It is a numeric assignment */
	if (expr) {
		uint16_t addr = VARS + ast->op1->val * 2;
		emit_byte(code, 0x89);	/* MOV */
		emit_byte(code, 0x06);	/* [addr], AX */
		emit_word(code, addr);
	}
	else {
		uint16_t addr = STRVARS + ast->op1->val * 128;
		emit_byte(code, 0xC7);	/* MOV */
		emit_byte(code, 0xC7);	/* DI, */
		emit_word(code, addr);	/* addr */

		/* Copy string into variable */
		emit_call(code, 0x0039);
	}
}

void compile_if(Node* ast, CompileTarget* code)
{
	/* Compile condition */
	compile_expression(ast->op1, code);

	/* If it is false skip THEN branch */
	emit_byte(code, 0x85);		/* TEST */
	emit_byte(code, 0xC0);		/* AX, AX */
	emit_byte(code, 0x0F);		/* JZ NEAR */
	emit_byte(code, 0x84);
	emit_word(code, 0x0000);	/* To patch up */

	int patch = code->length - 2;

	/* Compile THEN branch */
	compile_ast(ast->op2->op1, code);

	/* Jump over ELSE branch (and patch the jump) */
	emit_byte(code, 0xE9);		/* JMP NEAR */
	emit_word(code, 0x0000);	/* To patch up */
	uint16_t rel = code->length - (patch + 2);
	code->code[patch] = (uint8_t) rel & 0xFF;
	code->code[patch + 1] = (uint8_t) (rel >> 8) & 0xFF;

	/* New patch will be needed */
	patch = code->length - 2;

	/* Compile ELSE branch */
	compile_ast(ast->op2->op2, code);
	rel = code->length - (patch + 2);
	code->code[patch] = (uint8_t) rel & 0xFF;
	code->code[patch + 1] = (uint8_t) (rel >> 8) & 0xFF;
}

void compile_do(Node* ast, CompileTarget* code)
{
	/* Compile the body first */
	uint16_t start = code->length;
	compile_ast(ast->op2->op1, code);

	/* Now we need to check if there is a condition */
	if (ast->op1 == NULL)
		emit_jump(code, LOAD + start);	/* Just loop endlessly */
	else {
		/* Check modifier */
		TokenType mod = ast->op2->op2->attribute;

		/* Compile condition */
		compile_expression(ast->op1, code);
		uint16_t rel = start - (code->length + 6);

		/* Pick what to do next */
		if (mod == TOKEN_WHILE) {
			emit_byte(code, 0x85);		/* TEST */
			emit_byte(code, 0xC0);		/* AX, AX */
			emit_byte(code, 0x0F);		/* JNE NEAR */
			emit_byte(code, 0x85);
			emit_word(code, rel);
		}
		else {
			emit_byte(code, 0x85);		/* TEST */
			emit_byte(code, 0xC0);		/* AX, AX */
			emit_byte(code, 0x0F);		/* JZ NEAR */
			emit_byte(code, 0x84);
			emit_word(code, rel);
		}
	}
}

void compile_for(Node* ast, CompileTarget* code)
{
	int var_num = ast->op1->op1->val;
	uint16_t var = VARS + var_num * 2;

	/* Compile initializer */
	compile_assign(ast->op1, code);

	/* Save where to jump */
	uint16_t start = code->length;

	/* Compile the body and NEXT */
	compile_ast(ast->op2->op2, code);
	emit_byte(code, 0xFF);				/* INC */
	emit_byte(code, 0x06);				/* [imm16] */
	emit_word(code, var);

	/* Compile "TO" field */
	bool to = compile_expression(ast->op2->op1, code);
	if (!to) {
		compile_error("Type error in FOR TO field", ast);
		return;
	}
	emit_byte(code, 0x8B);				/* MOV */
	emit_byte(code, 0xD8);				/* BX, AX */

	/* Perform bound check */
	emit_byte(code, 0x8B);				/* MOV */
	emit_byte(code, 0x06);				/* AX, */
	emit_word(code, var);				/* [imm16] */
	emit_byte(code, 0x3B);				/* CMP */
	emit_byte(code, 0xD8);				/* BX, AX */
	emit_byte(code, 0x0F);				/* JGE NEAR */
	emit_byte(code, 0x8D);

	uint16_t rel = start - (code->length + 2);
	emit_word(code, rel);
}

/* =========================== MAIN CODE GENERATOR ========================== */
typedef void (*CompileFuncPtr)(Node*, CompileTarget*);
static CompileFuncPtr node_compiler[] = {
	[NODE_ASSIGN] = compile_assign,
	[NODE_EXPR] = NULL,	/* Expression is an invalid statement anyways */
	[NODE_VARIABLE] = NULL,	/* And so is a variable */
	[NODE_LITERAL] = NULL,	/* Or a literal */
	[NODE_LABEL] = NULL,	/* And a label too */
	[NODE_IF] = compile_if,
	[NODE_DO] = compile_do,
	[NODE_FOR] = compile_for,
	[NODE_KEYWORD_CALL] = compile_keyword
};

/* Generate code proper, with no prologue */
void compile_ast(Node* ast, CompileTarget* code)
{
	/* When you hit empty node, just return */
	if (ast == NULL)
		return;

	patch_jumps(code, patches, symbols, ast);

	/* Now select what to do */
	if (ast->type == NODE_SEQUENCE) {
		compile_ast(ast->op1, code);
		compile_ast(ast->op2, code);
	}
	else {
		CompileFuncPtr rule = node_compiler[ast->type];
		rule(ast, code);
	}
}

void compile(Node* ast, CompileTarget* code, StringTable* str, SymbolTable* t)
{
	PatchTable p;
	init_patch(&p);
	patches = &p;
	symbols = t;

	init_expr_compiler(str);
	init_kword_compiler(str, t, &p);
	make_entry(code, str);
	compile_ast(ast, code);

	/* If program doesn't have END, add one */
	if ((uint8_t) code->code[code->length - 1] != 0xC3)
		make_exit(code);

	/* Fix RAMSTART */
	uint16_t ramstart = LOAD + code->length;
	code->code[RAMSTART - LOAD] = (uint8_t) ramstart & 0xFF;
	code->code[RAMSTART + 1 - LOAD] = (uint8_t) (ramstart >> 8) & 0xFF;

	free_patch(&p);
	patches = NULL;
}
