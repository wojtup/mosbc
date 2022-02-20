/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

/* Custom includes */
#include <lexer.h>
#include <codegen.h>

static StringTable* strings;

/* Return true if is numeric, false if string */
bool compile_expression(Node* ast, CompileTarget* code)
{
	switch (ast->attribute) {
		/* Keyword values: */
		case TOKEN_INK: {
			emit_byte(code, 0x8B);			/* MOV */
			emit_byte(code, 0x06);			/* AX */
			emit_word(code, INKADDR);		/* [imm16] */
			return true;
		}
		case TOKEN_PROGSTART: {
			emit_byte(code, 0xC7);			/* MOV */
			emit_byte(code, 0xC0);			/* AX, */
			emit_word(code, (uint16_t)LOAD);	/* imm16 */
			return true;
		}
		case TOKEN_RAMSTART: {
			emit_byte(code, 0x8B);			/* MOV */
			emit_byte(code, 0x06);			/* AX */
			emit_word(code, RAMSTART);		/* [imm16] */
			return true;
		}
		case TOKEN_TIMER: {
			emit_byte(code, 0x33);			/* XOR */
			emit_byte(code, 0xC0);			/* AX, AX */
			emit_byte(code, 0xCD);			/* INT */
			emit_byte(code, 0x1A);			/* 0x1A */
			emit_byte(code, 0x8B);			/* MOV */
			emit_byte(code, 0xC2);			/* AX, DX */
			return true;
		}
		case TOKEN_VARIABLES: {
			emit_byte(code, 0xC7);			/* MOV */
			emit_byte(code, 0xC0);			/* AX, */
			emit_word(code, (uint16_t)VARS);	/* imm16 */
			return true;
		}
		case TOKEN_VERSION: {
			emit_byte(code, 0xC7);			/* MOV */
			emit_byte(code, 0xC0);			/* AX, */
			emit_word(code, (uint16_t)VERSION);	/* imm16 */
			return true;
		}

		/* Literals / variables: */
		case TOKEN_NUMERIC_LITERAL: {
			emit_byte(code, 0xC7);			/* MOV */
			emit_byte(code, 0xC0);			/* AX, */
			emit_word(code, (uint16_t)ast->val);	/* imm16 */
			return true;
		}
		case TOKEN_NUMERIC_VARIABLE: {
			uint16_t addr = VARS + ast->val * 2;
			emit_byte(code, 0x8B);			/* MOV */
			emit_byte(code, 0x06);			/* AX, */
			emit_word(code, addr);			/* [imm16] */
			return true;
		}
		case TOKEN_STRING_LITERAL: {
			int offset = get_offset_string(strings, ast->val);
			uint16_t addr = LOAD + RUNTIMELEN + offset;
			emit_byte(code, 0xC7);			/* MOV */
			emit_byte(code, 0xC6);			/* SI, */
			emit_word(code, addr);			/* imm16 */
			return false;
		}
		case TOKEN_STRING_VARIABLE: {
			uint16_t addr = STRVARS + ast->val * 128;
			emit_byte(code, 0xC7);			/* MOV */
			emit_byte(code, 0xC6);			/* SI, */
			emit_word(code, addr);			/* imm16 */
			return false;
		}
		case TOKEN_CHARACTER_LITERAL: {
			emit_byte(code, 0xC7);			/* MOV */
			emit_byte(code, 0xC0);			/* AX, */
			emit_word(code, (uint16_t)ast->val);	/* imm16 */
			return true;
		}

		/* Numeric / string operators: */
		case TOKEN_PLUS: {
			bool a = compile_expression(ast->op1, code);

			/* Save value (numeric to BX, string to DI) */
			if (a) {
				emit_byte(code, 0x8B);		/* MOV */
				emit_byte(code, 0xD8);		/* BX, AX */
			}
			else {
				emit_byte(code, 0x8B);		/* MOV */
				emit_byte(code, 0xFE);		/* DI, SI */
			}

			bool b = compile_expression(ast->op2, code);

			/* Check typing */
			if (a != b) {
				compile_error("Type error in expression", ast);
				return false;
			}

			/* Perform addition */
			if (a) {
				emit_byte(code, 0x03);		/* ADD */
				emit_byte(code, 0xC3);		/* AX, BX */
				return true;
			}
			else {
				emit_byte(code, 0x87);		/* XCHG */
				emit_byte(code, 0xFE);		/* DI, SI */
				emit_call(code, STRADD);	/* Call adder */
				return false;
			}
		}
		case TOKEN_MINUS: {
			bool a = compile_expression(ast->op1, code);
			if (!a) {
				compile_error("Type error in expression", ast);
				return false;
			}

			/* Save result */
			emit_byte(code, 0x8B);			/* MOV */
			emit_byte(code, 0xD8);			/* BX, AX */

			bool b = compile_expression(ast->op2, code);
			if (!b) {
				compile_error("Type error in expression", ast);
				return false;
			}

			emit_byte(code, 0x93);			/* XCHG AX,BX */

			emit_byte(code, 0x2B);			/* SUB */
			emit_byte(code, 0xC3);			/* AX, BX */

			return true;
		}
		case TOKEN_STAR: {
			bool a = compile_expression(ast->op1, code);
			if (!a) {
				compile_error("Type error in expression", ast);
				return false;
			}

			/* Save result */
			emit_byte(code, 0x8B);			/* MOV */
			emit_byte(code, 0xD8);			/* BX, AX */

			bool b = compile_expression(ast->op2, code);
			if (!b) {
				compile_error("Type error in expression", ast);
				return false;
			}

			emit_byte(code, 0xF7);			/* MUL */
			emit_byte(code, 0xE3);			/* BX */

			return true;
		}
		case TOKEN_SLASH: {
			bool a = compile_expression(ast->op1, code);
			if (!a) {
				compile_error("Type error in expression", ast);
				return false;
			}

			/* Save result */
			emit_byte(code, 0x8B);			/* MOV */
			emit_byte(code, 0xD8);			/* BX, AX */

			bool b = compile_expression(ast->op2, code);
			if (!b) {
				compile_error("Type error in expression", ast);
				return false;
			}

			/* Handle division by zero */
			emit_byte(code, 0x85);			/* TEST */
			emit_byte(code, 0xC0);			/* AX, AX */
			emit_byte(code, 0x75);			/* JNE */
			emit_byte(code, 0x03);			/* rel8 */
			emit_call(code, ZERODIV);		/* Error! */

			/* Proceed */
			emit_byte(code, 0x93);			/* XCHG AX,BX */
			emit_byte(code, 0x33);			/* XOR */
			emit_byte(code, 0xD2);			/* DX, DX */
			emit_byte(code, 0xF7);			/* DIV */
			emit_byte(code, 0xF3);			/* BX */

			return true;
		}
		case TOKEN_PERCENT: {
			bool a = compile_expression(ast->op1, code);
			if (!a) {
				compile_error("Type error in expression", ast);
				return false;
			}

			/* Save result */
			emit_byte(code, 0x8B);			/* MOV */
			emit_byte(code, 0xD8);			/* BX, AX */

			bool b = compile_expression(ast->op2, code);
			if (!b) {
				compile_error("Type error in expression", ast);
				return false;
			}

			/* Handle division by zero */
			emit_byte(code, 0x85);			/* TEST */
			emit_byte(code, 0xC0);			/* AX, AX */
			emit_byte(code, 0x75);			/* JNE */
			emit_byte(code, 0x03);			/* rel8 */
			emit_call(code, ZERODIV);		/* Error! */

			/* Proceed */
			emit_byte(code, 0x93);			/* XCHG AX,BX */
			emit_byte(code, 0x33);			/* XOR */
			emit_byte(code, 0xD2);			/* DX, DX */
			emit_byte(code, 0xF7);			/* DIV */
			emit_byte(code, 0xF3);			/* BX */
			emit_byte(code, 0x8B);			/* MOV */
			emit_byte(code, 0xC2);			/* AX, DX */

			return true;
		}
		case TOKEN_AMPERSAND: {
			uint16_t addr;
			if (ast->op1->attribute == TOKEN_NUMERIC_VARIABLE)
				addr = VARS + ast->op1->val * 2;
			else
				addr = STRVARS + ast->op1->val * 128;

			emit_byte(code, 0xC7);			/* MOV */
			emit_byte(code, 0xC0);			/* AX, */
			emit_word(code, addr);			/* imm16 */
			return true;
		}

		/* Boolean operators: */
		case TOKEN_AND: {
			bool a = compile_expression(ast->op1, code);
			if (!a) {
				compile_error("Type error in expression", ast);
				return false;
			}

			/* Check first boolean */
			emit_byte(code, 0x85);			/* TEST */
			emit_byte(code, 0xC0);			/* AX, AX */
			emit_byte(code, 0x0F);			/* JZ */
			emit_byte(code, 0x84);			/* NEAR */
			emit_word(code, 0x0000);		/* False */
			int patch = code->length - 2;

			bool b = compile_expression(ast->op2, code);
			if (!b) {
				compile_error("Type error in expression", ast);
				return false;
			}

			/* Check second boolean */
			emit_byte(code, 0x85);			/* TEST */
			emit_byte(code, 0xC0);			/* AX, AX */
			emit_byte(code, 0x74);			/* JZ */
			emit_byte(code, 0x06);			/* False */

			/* Both were true */
			emit_byte(code, 0xC7);			/* MOV */
			emit_byte(code, 0xC0);			/* AX, */
			emit_word(code, 0x0001);		/* 1 */
			emit_byte(code, 0xEB);			/* JMP SHORT */
			emit_byte(code, 0x02);			/* Skip false */

			/* Patch up our previous jump */
			uint16_t rel = code->length - (patch + 2);
			code->code[patch] = (uint8_t) rel & 0xFF;
			code->code[patch + 1] = (uint8_t) (rel >> 8) & 0xFF;

			/* One was false */
			emit_byte(code, 0x33);			/* XOR */
			emit_byte(code, 0xC0);			/* AX, AX */

			return true;
		}
		case TOKEN_EQUALS: {
			bool a = compile_expression(ast->op1, code);
			/* Save value, depending on string/numeric */
			if (a) {
				emit_byte(code, 0x8B);	/* MOV */
				emit_byte(code, 0xD8);	/* BX, AX */
			}
			else {
				emit_byte(code, 0x8B);	/* MOV */
				emit_byte(code, 0xFE);	/* DI, SI */
			}

			bool b = compile_expression(ast->op2, code);
			if (a != b) {
				compile_error("Type error in expression", ast);
				return false;
			}

			/* Perform comparison */
			if (a) {
				emit_byte(code, 0x3B);	/* CMP */
				emit_byte(code, 0xC3);	/* AX, BX */
				emit_byte(code, 0x75);	/* JNE */
				emit_byte(code, 0x06);	/* Skip == branch */
				emit_byte(code, 0xC7);	/* MOV */
				emit_byte(code, 0xC0);	/* AX, */
				emit_word(code, 0x0001);/* 1 */
				emit_byte(code, 0xEB);	/* JMP short */
				emit_byte(code, 0x02);	/* Skip != branch */
				emit_byte(code, 0x33);	/* XOR */
				emit_byte(code, 0xC0);	/* AX, AX */
			}
			else {
				/* CALL os_string_compare */
				emit_call(code, 0x0045);
				emit_byte(code, 0x72);	/* JC */
				emit_byte(code, 0x04);	/* Skip != branch */
				emit_byte(code, 0x33);	/* XOR */
				emit_byte(code, 0xC0);	/* AX, AX */
				emit_byte(code, 0xEB);	/* JMP short */
				emit_byte(code, 0x04);	/* Skip == branch */
				emit_byte(code, 0xC7);	/* MOV */
				emit_byte(code, 0xC0);	/* AX, */
				emit_word(code, 0x0001);/* 1 */
			}
			return true;
		}
		case TOKEN_SMALLER: {
			bool a = compile_expression(ast->op1, code);
			if (!a) {
				compile_error("Type error in expression", ast);
				return false;
			}

			emit_byte(code, 0x8B);		/* MOV */
			emit_byte(code, 0xD8);		/* BX, AX */

			bool b = compile_expression(ast->op2, code);
			if (!b) {
				compile_error("Type error in expression", ast);
				return false;
			}

			emit_byte(code, 0x3B);		/* CMP */
			emit_byte(code, 0xC3);		/* AX, BX */
			emit_byte(code, 0x7F);		/* JG */
			emit_byte(code, 0x04);		/* Skip <= branch */
			emit_byte(code, 0x33);		/* XOR */
			emit_byte(code, 0xC0);		/* AX, AX */
			emit_byte(code, 0xEB);		/* JMP short */
			emit_byte(code, 0x04);		/* Skip > branch */
			emit_byte(code, 0xC7);		/* MOV */
			emit_byte(code, 0xC0);		/* AX, */
			emit_word(code, 0x0001);	/* 1 */
			return true;
		}
		case TOKEN_GREATER: {
			bool a = compile_expression(ast->op1, code);
			if (!a) {
				compile_error("Type error in expression", ast);
				return false;
			}

			emit_byte(code, 0x8B);		/* MOV */
			emit_byte(code, 0xD8);		/* BX, AX */

			bool b = compile_expression(ast->op2, code);
			if (!b) {
				compile_error("Type error in expression", ast);
				return false;
			}

			emit_byte(code, 0x3B);		/* CMP */
			emit_byte(code, 0xD8);		/* BX, AX */
			emit_byte(code, 0x7F);		/* JG */
			emit_byte(code, 0x04);		/* skip >= branch */
			emit_byte(code, 0x33);		/* XOR */
			emit_byte(code, 0xC0);		/* AX, AX */
			emit_byte(code, 0xEB);		/* JMP short */
			emit_byte(code, 0x04);		/* skip < branch */
			emit_byte(code, 0xC7);		/* MOV */
			emit_byte(code, 0xC0);		/* AX, */
			emit_word(code, 0x0001);	/* 1 */
			return true;
		}
		case TOKEN_NOT_EQUALS: {
			bool a = compile_expression(ast->op1, code);
			/* Save value, depending on string/numeric */
			if (a) {
				emit_byte(code, 0x8B);	/* MOV */
				emit_byte(code, 0xD8);	/* BX, AX */
			}
			else {
				emit_byte(code, 0x8B);	/* MOV */
				emit_byte(code, 0xFE);	/* DI, SI */
			}

			bool b = compile_expression(ast->op2, code);
			if (a != b) {
				compile_error("Type error in expression", ast);
				return false;
			}

			/* Perform comparison */
			if (a) {
				emit_byte(code, 0x3B);	/* CMP */
				emit_byte(code, 0xC3);	/* AX, BX */
				emit_byte(code, 0x75);	/* JNE */
				emit_byte(code, 0x04);	/* Skip == branch */
				emit_byte(code, 0x33);	/* XOR */
				emit_byte(code, 0xC0);	/* AX, AX */
				emit_byte(code, 0xEB);	/* JMP short */
				emit_byte(code, 0x04);	/* Skip != branch */
				emit_byte(code, 0xC7);	/* MOV */
				emit_byte(code, 0xC0);	/* AX, */
				emit_word(code, 0x0001);/* 1 */
			}
			else {
				/* CALL os_string_compare */
				emit_call(code, 0x0045);
				emit_byte(code, 0x72);	/* JC */
				emit_byte(code, 0x06);	/* Skip != branch */
				emit_byte(code, 0xC7);	/* MOV */
				emit_byte(code, 0xC0);	/* AX, */
				emit_word(code, 0x0001);/* 1 */
				emit_byte(code, 0xEB);	/* JMP short */
				emit_byte(code, 0x02);	/* Skip == branch */
				emit_byte(code, 0x33);	/* XOR */
				emit_byte(code, 0xC0);	/* AX, AX */
			}
			return true;
		}

		/* No match: */
		default:
			compile_error("Cannot compile expression", ast);
			break;
	}

	/* Unreached */
	return false;
}

void init_expr_compiler(StringTable* str)
{
	strings = str;
}
