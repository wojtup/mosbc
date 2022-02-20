/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

/* Standard library includes */
#include <stdio.h>
#include <string.h>

/* Custom includes */
#include <table.h>
#include <codegen.h>
#include <runtime.h>

static SymbolTable* symbols;
static StringTable* strings;
static PatchTable* patches;

/* =========================== COMPILER FUNCTIONS =========================== */
void compile_alert(Node* ast, CompileTarget* code)
{
	compile_expression(ast->op1, code);

	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xC6);			/* AX, SI */
	emit_byte(code, 0x33);			/* XOR */
	emit_byte(code, 0xDB);			/* BX, BX */
	emit_byte(code, 0x33);			/* XOR */
	emit_byte(code, 0xC9);			/* CX, CX */
	emit_byte(code, 0x33);			/* XOR */
	emit_byte(code, 0xD2);			/* DX, DX */

	/* CALL os_dialog_box */
	emit_call(code, 0x003C);
}

void compile_askfile(Node* ast, CompileTarget* code)
{
	uint16_t var = STRVARS + ast->op1->val * 128;

	/* CALL os_file_selector */
	emit_call(code, 0x005A);

	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xF0);			/* SI, AX */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC7);			/* DI, */
	emit_word(code, var);			/* var */

	/* CALL os_string_copy */
	emit_call(code, 0x0039);
}

void compile_break(Node* ast, CompileTarget* code)
{
	/* Make string */
	int line = ast->line;
	char msg[28];
	sprintf(msg, "BREAK CALLED - line %d\r\n", line);
	uint16_t addr = LOAD + code->length + 9;

	/* Print message */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC6);			/* SI, */
	emit_word(code, addr);			/* imm16 */
	emit_call(code, 0x0003);

	/* Exit program */
	make_exit(code);

	emit_string(code, msg);
}

void compile_call(Node* ast, CompileTarget* code)
{
	compile_expression(ast->op1, code);
	emit_byte(code, 0xFF);			/* CALL */
	emit_byte(code, 0xD0);			/* AX */
}

void compile_case(Node* ast, CompileTarget* code)
{
	uint16_t var = STRVARS + ast->op2->val * 128;

	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, var);			/* imm16 */

	/* CALL os_string_lowercase */
	if (ast->op1->attribute == TOKEN_LOWER)
		emit_call(code, 0x0033);
	/* CALL os_string_uppercase */
	else
		emit_call(code, 0x0030);
}

void compile_cls(Node* ast, CompileTarget* code)
{
	ast->op1 = NULL;			/* Shut up */
	/* CALL os_clear_screen */
	emit_call(code, 0x0009);
}

void compile_cursor(Node* ast, CompileTarget* code)
{
	/* CALL os_show_cursor */
	if (ast->op1->attribute == TOKEN_ON)
		emit_call(code, 0x008A);
	/* CALL os_hide_cursor */
	else
		emit_call(code, 0x008D);
}

void compile_curschar(Node* ast, CompileTarget* code)
{
	uint16_t var = VARS + ast->op1->val * 2;

	/* Call BIOS for character */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0800);		/* 0x800 -> AH = 8 */
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0x1E);			/* BX, [imm16] */
	emit_word(code, WORKPAGE);		/* Working page */
	emit_byte(code, 0xCD);			/* INT */
	emit_byte(code, 0x10);			/* 0x10 */

	/* Store it (clean upper byte of AX too!) */
	emit_byte(code, 0x25);			/* AND AX, */
	emit_word(code, 0x00FF);		/* 0x00FF */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, var);
}

void compile_curscol(Node* ast, CompileTarget* code)
{
	uint16_t var = VARS + ast->op1->val * 2;

	/* Call BIOS for character */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0800);		/* 0x800 -> AH = 8 */
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0x1E);			/* BX, [imm16] */
	emit_word(code, WORKPAGE);		/* Working page */
	emit_byte(code, 0xCD);			/* INT */
	emit_byte(code, 0x10);			/* 0x10 */

	/* Store it (clean upper byte of AX too!) */
	emit_byte(code, 0xC1);			/* SHR */
	emit_byte(code, 0xE8);			/* AX, */
	emit_byte(code, 0x08);			/* 8 */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, var);
}

void compile_curspos(Node* ast, CompileTarget* code)
{
	uint16_t vara = VARS + ast->op1->val * 2;
	uint16_t varb = VARS + ast->op2->val * 2;

	/* CALL os_get_cursor_pos */
	emit_call(code, 0x0069);

	/* Store column */
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xC2);			/* AX, DX */
	emit_byte(code, 0x25);			/* AND AX, */
	emit_word(code, 0x00FF);		/* 0x00FF */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, vara);

	/* Store row */
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xC2);			/* AX, DX */
	emit_byte(code, 0xC1);			/* SHR */
	emit_byte(code, 0xE8);			/* AX, */
	emit_byte(code, 0x08);			/* 8 */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, varb);
}

void compile_delete(Node* ast, CompileTarget* code)
{
	uint16_t rvar = VARS + ('r' - 'a') * 2;

	compile_expression(ast->op1, code);

	/* Check if file exists (CALL os_file_exists) */
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xC6);			/* AX, SI */
	emit_call(code, 0x0099);
	emit_byte(code, 0x72);			/* JC */
	emit_byte(code, 0x11);			/* Over exists branch */

	/* Try deleting file (CALL os_remove_file) */
	emit_call(code, 0x009F);
	emit_byte(code, 0x72);			/* JC */
	emit_byte(code, 0x04);			/* Jump over failure */

	/* File deleted, set R to 0 */
	emit_byte(code, 0x33);			/* XOR */
	emit_byte(code, 0xC0);			/* AX, AX */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0x0E);			/* Over failures */

	/* File couldn't be deleted, set R to 1 */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0x06);
	emit_word(code, rvar);			/* [rvar], */
	emit_word(code, 0x0001);		/* 1 */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0x06);			/* Over failure */

	/* File doesn't exist, set R to 2 */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0x06);
	emit_word(code, rvar);			/* [rvar], */
	emit_word(code, 0x0002);		/* 2 */
}

void compile_end(Node* ast, CompileTarget* code)
{
	ast->op1 = NULL;			/* Shut up */
	make_exit(code);
}

void compile_files(Node* ast, CompileTarget* code)
{
	ast->op1 = NULL;			/* Shut up */

	/* First set AX to our buffer */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, STRBUF);		/* STRBUF */

	/* CALL os_get_file_list */
	emit_call(code, 0x0042);

	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xF0);			/* SI, AX */
	emit_byte(code, 0x56);			/* PUSH SI */
	emit_byte(code, 0x25);			/* AND AX, */
	emit_word(code, 0x00FF);

	/* Loop through list, replace all commas for newlines */
	emit_byte(code, 0xAC);			/* LODSB */
	emit_byte(code, 0x85);			/* TEST */
	emit_byte(code, 0xC0);			/* AX, AX */
	emit_byte(code, 0x74);			/* JZ */
	emit_byte(code, 0x10);			/* To the end */
	emit_byte(code, 0x3D);			/* CMP AX, */
	emit_word(code, 0x002C);		/* 0x002C = ',' */
	emit_byte(code, 0x75);			/* JNE */
	emit_byte(code, 0xF6);			/* Back to the loop */

	/* Replace byte */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x000A);		/* 0xA */
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xFE);			/* DI, SI */
	emit_byte(code, 0xFF);			/* DEC */
	emit_byte(code, 0xCF);			/* DI */
	emit_byte(code, 0xAA);			/* STOSB */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0xEB);			/* Back to the loop */

	emit_byte(code, 0x5E);			/* POP SI */
	/* CALL print_string */
	emit_call(code, PRINTSTR);

	/* Put NL in STRBUF and print it */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x000A);		/* 0xA */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm], AX */
	emit_word(code, STRBUF);
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC6);			/* SI, */
	emit_word(code, STRBUF);

	/* CALL print_string */
	emit_call(code, PRINTSTR);
}

void compile_getkey(Node* ast, CompileTarget* code)
{
	uint16_t var = VARS + ast->op1->val * 2;

	/* CALL os_check_for_key */
	emit_call(code, 0x0015);

	/* Is it special char? */
	emit_byte(code, 0x3D);			/* CMP AX, */
	emit_word(code, 0x48E0);		/* 0x48E0 */
	emit_byte(code, 0x74);			/* JE */
	emit_byte(code, 0x18);			/* To UP (24) */
	emit_byte(code, 0x3D);			/* CMP AX, */
	emit_word(code, 0x50E0);		/* 0x50E0 */
	emit_byte(code, 0x74);			/* JE */
	emit_byte(code, 0x19);			/* To DOWN (25) */
	emit_byte(code, 0x3D);			/* CMP AX, */
	emit_word(code, 0x4BE0);		/* 0x4BE0 */
	emit_byte(code, 0x74);			/* JE */
	emit_byte(code, 0x1A);			/* To LEFT (26) */
	emit_byte(code, 0x3D);			/* CMP AX, */
	emit_word(code, 0x4DE0);		/* 0x4DE0 */
	emit_byte(code, 0x74);			/* JE */
	emit_byte(code, 0x1B);			/* To RIGHT (27) */

	/* Store the character */
	emit_byte(code, 0x25);			/* AND AX, */
	emit_word(code, 0x00FF);		/* 0x00FF */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, var);
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0x18);			/* Over others (24) */

	/* It was UP */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0001);		/* 1 */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0xF1);			/* To store (-15) */

	/* It was DOWN */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0002);		/* 2 */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0xEB);			/* To store (-21) */

	/* It was LEFT */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0003);		/* 3 */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0xE5);			/* To store (-27) */

	/* It was RIGHT */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0004);		/* 4 */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0xDF);			/* To store (-33) */

}

void compile_gosub(Node* ast, CompileTarget* code)
{
	int id = ast->op1->val;

	/* Is label even real? */
	if (!symbols->table[id].isreal) {
		compile_error("GOSUB label not present", ast);
		return;
	}

	/* Label was already compiled */
	if (symbols->table[id].addr != 0)
		emit_call(code, symbols->table[id].addr);
	/* It wasn't */
	else {
		emit_byte(code, 0xE8);
		add_patch(patches, id, code->length);
		emit_word(code, 0x0000);
	}
}

void compile_goto(Node* ast, CompileTarget* code)
{
	int id = ast->op1->val;

	/* Is label even real? */
	if (!symbols->table[id].isreal) {
		compile_error("GOTO label not present", ast);
		return;
	}

	/* Label was already compiled */
	if (symbols->table[id].addr != 0)
		emit_jump(code, symbols->table[id].addr);
	/* It wasn't */
	else {
		emit_byte(code, 0xE9);
		add_patch(patches, id, code->length);
		emit_word(code, 0x0000);
	}
}

void compile_ink(Node* ast, CompileTarget* code)
{
	compile_expression(ast->op1, code);

	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, INKADDR);
}

void compile_input(Node* ast, CompileTarget* code)
{
	/* Do we want string? */
	if (ast->op1->attribute == TOKEN_STRING_VARIABLE) {
		uint16_t var = STRVARS + ast->op1->val * 128;
		emit_byte(code, 0xC7);		/* MOV */
		emit_byte(code, 0xC0);		/* AX, */
		emit_word(code, var);		/* var */

		/* CALL os_input_string */
		emit_call(code, 0x0036);
		/* CALL os_print_newline */
		emit_call(code, 0x000F);

		return;
	}

	/* No, we want numeric, use buffer */
	uint16_t var = VARS + ast->op1->val * 2;

	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, STRBUF);		/* STRBUF */

	/* CALL os_input_string */
	emit_call(code, 0x0036);

	/* Check for empty string */
	emit_call(code, 0x002D);
	emit_byte(code, 0x85);			/* TEST */
	emit_byte(code, 0xC0);			/* AX, AX */
	emit_byte(code, 0x75);			/* JNZ */
	emit_byte(code, 0x08);			/* Over zeroing it */

	/* We need to put "0(NUL)" in buffer */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0030);		/* 0x0030 -> "0\0" */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, STRBUF);		/* STRBUF */

	/* Convert string to number */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC6);			/* SI, */
	emit_word(code, STRBUF);		/* STRBUF */
	/* CALL os_string_to_int */
	emit_call(code, 0x00B1);

	/* Store it */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, var);			/* var */

	/* CALL os_print_newline */
	emit_call(code, 0x000F);
}

void compile_len(Node* ast, CompileTarget* code)
{
	uint16_t var = VARS + ast->op2->val * 2;

	/* Compile string first */
	compile_expression(ast->op1, code);

	/* Calculate its length */
	emit_call(code, 0x002D);

	/* Store it */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, var);			/* var */
}

void compile_listbox(Node* ast, CompileTarget* code)
{
	uint16_t var = VARS + ast->op2->op2->op2->val * 2;

	/* First string to AX */
	compile_expression(ast->op1, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xC6);			/* AX, SI */

	/* Second to BX */
	compile_expression(ast->op2->op1, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xDE);			/* BX, SI */

	/* Third to CX */
	compile_expression(ast->op2->op2->op1, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xCE);			/* CX, SI */

	/* CALL os_list_dialog */
	emit_call(code, 0x00AB);

	/* Maybe it was ESC? */
	emit_byte(code, 0x72);			/* JC */
	emit_byte(code, 0x06);			/* Over store */

	/* Store the value */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, var);			/* var */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0x04);			/* Over ESC */

	/* ESC pressed, set AX to zero */
	emit_byte(code, 0x33);			/* XOR */
	emit_byte(code, 0xC0);			/* AX, AX */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0xF6);			/* Back to store */
}

void compile_load(Node* ast, CompileTarget* code)
{
	uint16_t rvar = VARS + ('r' - 'a') * 2;
	uint16_t svar = VARS + ('s' - 'a') * 2;

	/* Put load position in CX */
	compile_expression(ast->op2, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xC8);			/* CX, AX */

	/* Put filename in AX */
	compile_expression(ast->op1, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xC6);			/* AX, SI */

	/* CALL os_load_file */
	emit_call(code, 0x0021);

	/* First check if file was loaded */
	emit_byte(code, 0x72);			/* JC */
	emit_byte(code, 0x08);			/* To set R to 1 */

	/* Okay, set BX to 0 and S to file size */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x1E);			/* [imm16], BX */
	emit_word(code, svar);			/* svar */
	emit_byte(code, 0x33);			/* XOR */
	emit_byte(code, 0xD8);			/* BX, BX */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0x04);			/* Over failure */

	/* Set BX to 1 */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC3);			/* BX, */
	emit_word(code, 0x0001);		/* 1 */

	/* Store BX to R */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x1E);			/* [imm16], BX */
	emit_word(code, rvar);			/* rvar */
}

void compile_move(Node* ast, CompileTarget* code)
{
	/* Put row in DX */
	compile_expression(ast->op2, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xD0);			/* DX, AX */

	/* Shift DX left, to make room for column */
	emit_byte(code, 0xC1);			/* SHL */
	emit_byte(code, 0xE2);			/* DX, */
	emit_byte(code, 0x08);			/* 8 */

	/* Add column to DX, putting it in DL */
	compile_expression(ast->op1, code);
	emit_byte(code, 0x03);			/* ADD */
	emit_byte(code, 0xD0);			/* DX, AX */

	/* CALL os_move_cursor */
	emit_call(code, 0x0006);
}

void compile_number(Node* ast, CompileTarget* code)
{
	bool src = compile_expression(ast->op1, code);

	/* Is it numeric to string? */
	if (src) {
		uint16_t dst = STRVARS + ast->op2->val * 128;

		/* CALL os_int_to_string */
		emit_call(code, 0x0018);

		/* Copy internal buffer into destination */
		emit_byte(code, 0x8B);		/* MOV */
		emit_byte(code, 0xF0);		/* SI, AX */
		emit_byte(code, 0xC7);		/* MOV */
		emit_byte(code, 0xC7);		/* DI, */
		emit_word(code, dst);		/* dst */

		/* CALL os_string_copy */
		emit_call(code, 0x0039);
	}
	/* No, string to numeric */
	else {
		uint16_t dst = VARS + ast->op2->val * 2;

		/* CALL os_string_to_int */
		emit_call(code, 0x00B1);

		/* Store result to variable */
		emit_byte(code, 0x89);		/* MOV */
		emit_byte(code, 0x06);		/* [imm16], AX */
		emit_word(code, dst);		/* dst */
	}
}

void compile_page(Node* ast, CompileTarget* code)
{
	/* First value is new work page */
	compile_expression(ast->op1, code);
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, WORKPAGE);		/* WORKPAGE */

	/* Second value is new active page */
	compile_expression(ast->op2, code);
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, ACTIVEPAGE);		/* ACTIVEPAGE */

	/* Now set AX to 0x500, OR with ACTIVEPAGE */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0500);		/* 0x500 -> AH = 5 */
	emit_byte(code, 0x0B);			/* OR */
	emit_byte(code, 0x06);			/* AX, [imm16] */
	emit_word(code, ACTIVEPAGE);		/* ACTIVEPAGE */

	/* Now all is set for BIOS. Call it */
	emit_byte(code, 0xCD);			/* INT */
	emit_byte(code, 0x10);			/* 0x10 */
}

void compile_pause(Node* ast, CompileTarget* code)
{
	compile_expression(ast->op1, code);
	emit_call(code, 0x0024);
}

void compile_peek(Node* ast, CompileTarget* code)
{
	uint16_t var = VARS + ast->op1->val * 2;

	/* Address will be in AX, put it in BX and load */
	compile_expression(ast->op2, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xD8);			/* BX, AX */
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0x07);			/* AX, [BX] */

	/* Now mask the upper byte */
	emit_byte(code, 0x25);			/* AND AX, */
	emit_word(code, 0x00FF);		/* 0x00FF */

	/* Store the result */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, var);			/* var */
}

void compile_peekint(Node* ast, CompileTarget* code)
{
	uint16_t var = VARS + ast->op1->val * 2;

	/* Address will be in AX, put it in BX and load */
	compile_expression(ast->op2, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xD8);			/* BX, AX */
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0x07);			/* AX, [BX] */

	/* Store the result */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, var);			/* var */
}

void compile_poke(Node* ast, CompileTarget* code)
{
	/* Before we can poke, we need to read one byte more */
	compile_expression(ast->op2, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xD8);			/* BX, AX */
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0x07);			/* AX, [BX] */
	emit_byte(code, 0x25);			/* AND AX, */
	emit_word(code, 0xFF00);		/* 0xFF00 */

	/* Save upper byte in CX */
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xC8);			/* CX, AX */

	/* Get value to poke, and mask upper byte */
	compile_expression(ast->op1, code);
	emit_byte(code, 0x25);			/* AND AX, */
	emit_word(code, 0x00FF);		/* 0x00FF */

	/* Add it into CX */
	emit_byte(code, 0x03);			/* ADD */
	emit_byte(code, 0xC8);			/* CX, AX */

	/* Store CX under [BX] */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x0F);			/* [BX], CX */
}

void compile_pokeint(Node* ast, CompileTarget* code)
{
	/* Put address in BX */
	compile_expression(ast->op2, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xD8);			/* BX, AX */

	/* Get value to poke, and mask upper byte */
	compile_expression(ast->op1, code);

	/* Store AX under [BX] */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x07);			/* [BX], AX */
}

void compile_port(Node* ast, CompileTarget* code)
{
	/* First set DX to wanted port */
	compile_expression(ast->op2->op1, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xD0);			/* DX, AX */

	/* Do we want to write byte out? */
	if (ast->op1->attribute == TOKEN_OUT) {
		compile_expression(ast->op2->op2, code);

		/* CALL os_port_byte_out */
		emit_call(code, 0x00C9);
	}
	/* No, read it in */
	else {
		uint16_t var = VARS + ast->op2->op2->val * 2;

		/* CALL os_port_byte_in */
		emit_call(code, 0x00CC);

		/* Mask upper byte (just in case) and store */
		emit_byte(code, 0x25);		/* AND AX, */
		emit_word(code, 0x00FF);	/* 0x00FF */
		emit_byte(code, 0x89);		/* MOV */
		emit_byte(code, 0x06);		/* [imm16], AX */
		emit_word(code, var);		/* var */
	}
}

void compile_print(Node* ast, CompileTarget* code)
{
	/* First we need to compile what we want to print */
	bool expr = compile_expression(ast->op2->op1, code);

	/* Check if modifier is valid */
	if (!expr && ast->op1 != NULL) {
		compile_error("PRINT modifier used with string", ast);
		return;
	}

	/* We want to print SI, but if it is numeric, we need to
	   fill it with what we want */
	if (expr && ast->op1 == NULL) {
		/* CALL os_int_to_string */
		emit_call(code, 0x0018);

		/* Put it in SI */
		emit_byte(code, 0x8B);		/* MOV */
		emit_byte(code, 0xF0);		/* SI, AX */
	}
	else if (expr && ast->op1->attribute == TOKEN_CHR) {
		emit_byte(code, 0x25);		/* AND AX, */
		emit_word(code, 0x00FF);	/* 0x00FF */
		emit_byte(code, 0x89);		/* MOV */
		emit_byte(code, 0x06);		/* [imm], AX */
		emit_word(code, STRBUF);

		/* Set SI to buffer */
		emit_byte(code, 0xC7);		/* MOV */
		emit_byte(code, 0xC6);		/* SI, imm */
		emit_word(code, STRBUF);
	}
	else if (expr && ast->op1->attribute == TOKEN_HEX) {
		/* First set DX to zero and BX to 16 */
		emit_byte(code, 0x33);		/* XOR */
		emit_byte(code, 0xD2);		/* DX, DX */
		emit_byte(code, 0xC7);		/* MOV */
		emit_byte(code, 0xC3);		/* BX, imm */
		emit_word(code, 0x0010);	/* 0x10 = 16 */

		/* Now set DI to STRBUF */
		emit_byte(code, 0xC7);		/* MOV */
		emit_byte(code, 0xC7);		/* DI, imm */
		emit_word(code, STRBUF);

		/* CALL os_long_int_to_string */
		emit_call(code, 0x007E);

		/* Now set SI to DI */
		emit_byte(code, 0x8B);		/* MOV */
		emit_byte(code, 0xF7);		/* SI, DI */
	}

	/* CALL print_string */
	emit_call(code, PRINTSTR);

	/* If there is no semicolon, print NL */
	if (ast->op2->op2 == NULL) {
		emit_byte(code, 0xC7);		/* MOV */
		emit_byte(code, 0xC0);		/* AX, */
		emit_word(code, 0x000A);	/* 0xA */
		emit_byte(code, 0x89);		/* MOV */
		emit_byte(code, 0x06);		/* [imm], AX */
		emit_word(code, STRBUF);
		emit_byte(code, 0xC7);		/* MOV */
		emit_byte(code, 0xC6);		/* SI, */
		emit_word(code, STRBUF);

		/* CALL print_string */
		emit_call(code, PRINTSTR);
	}
}

void compile_rand(Node* ast, CompileTarget* code)
{
	uint16_t var = VARS + ast->op1->val * 2;

	/* Second value to BX */
	compile_expression(ast->op2->op2, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xD8);			/* BX, AX */

	/* First to AX */
	compile_expression(ast->op2->op1, code);

	/* CALL os_get_random */
	emit_call(code, 0x00B7);

	/* Store result from CX */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x0E);			/* [imm16], CX */
	emit_word(code, var);
}

void compile_rename(Node* ast, CompileTarget* code)
{
	uint16_t rvar = VARS + ('r' - 'a') * 2;

	/* First we check if destination exists */
	compile_expression(ast->op2, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xC6);			/* AX, SI */
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xDE);			/* BX, SI */

	/* CALL os_file_exists */
	emit_call(code, 0x0099);
	emit_byte(code, 0x72);			/* JC */
	emit_byte(code, 0x06);			/* Over failure */

	/* Destination exists, set R to 3 */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0003);		/* 3 */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0x00);			/* To store */
	uint16_t patch = code->length - 1;

	/* Now check source */
	compile_expression(ast->op1, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xC6);			/* AX, SI */

	/* CALL os_file_exists */
	emit_call(code, 0x0099);
	emit_byte(code, 0x72);			/* JC */
	emit_byte(code, 0x09);			/* To set R to 1 */

	/* Rename proper, CALL os_rename_file */
	emit_call(code, 0x00A2);
	emit_byte(code, 0x72);			/* JC */
	emit_byte(code, 0x0A);			/* To set R to 2 */

	/* No errors, set R to 0 */
	emit_byte(code, 0x33);			/* XOR */
	emit_byte(code, 0xC0);			/* AX, AX */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0x0A);			/* To store (+10) */

	/* Source not present, set R to 1 */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0001);		/* 1 */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0x04);			/* To store (+4) */

	/* Rename failed, set R to 2 */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0002);		/* 2 */

	/* Store AX to R (and patch up) */
	uint8_t rel = code->length - (patch + 1);
	code->code[patch] = rel & 0xFF;
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, rvar);
}

void compile_return(Node* ast, CompileTarget* code)
{
	ast->op1 = NULL;			/* Shut up */
	emit_byte(code, 0xC3);			/* RET */
}

void compile_save(Node* ast, CompileTarget* code)
{
	uint16_t rvar = VARS + ('r' - 'a') * 2;

	/* Put load address in BX */
	compile_expression(ast->op2->op1, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xD8);			/* BX, AX */

	/* Put file size in CX */
	compile_expression(ast->op2->op2, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xC8);			/* CX, AX */

	/* Put filename in AX */
	compile_expression(ast->op1, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xC6);			/* AX, SI */

	/* CALL os_file_exists */
	emit_call(code, 0x0099);
	emit_byte(code, 0x72);			/* JC */
	emit_byte(code, 0x02);			/* Proceed */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0x09);			/* File exists */

	/* All set, CALL os_write_file */
	emit_call(code, 0x0096);
	emit_byte(code, 0x72);			/* JC */
	emit_byte(code, 0x0A);			/* Couldn't save */

	/* All is good, set AX to 0 */
	emit_byte(code, 0x33);			/* XOR */
	emit_byte(code, 0xC0);			/* AX, AX */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0x0A);			/* To store */

	/* File exists, set AX to 2 */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0002);		/* 2 */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0x04);			/* To store */

	/* Cannot save, set AX to 1 */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0001);		/* 1 */

	/* Store AX to R */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, rvar);			/* rvar */
}

void compile_serial(Node* ast, CompileTarget* code)
{
	/* Turn serial on */
	if (ast->op1->attribute == TOKEN_ON) {
		int mode = ast->op2->val;

		/* Slow mode */
		if (mode == 1200) {
			emit_byte(code, 0x33);	/* XOR */
			emit_byte(code, 0xC0);	/* AX, AX */
		}
		/* Fast mode */
		else if (mode == 9600) {
			emit_byte(code, 0xC7);	/* MOV */
			emit_byte(code, 0xC0);	/* AX, */
			emit_word(code, 0x0001);/* 1 */
		}
		/* Invalid mode, compile error */
		else {
			compile_error("Invalid mode for SERIAL",
				ast);
		}

		/* CALL os_serial_port_enable */
		emit_call(code, 0x00BD);
		return;
	}

	/* Send value through serial */
	if (ast->op1->attribute == TOKEN_SEND) {
		compile_expression(ast->op2, code);

		/* CALL os_send_via_serial */
		emit_call(code, 0x0060);

		return;
	}

	/* We want to receive byte */
	uint16_t var = VARS + ast->op2->val * 2;

	/* CALL os_get_via_serial */
	emit_call(code, 0x0063);

	/* Mask upper byte and store */
	emit_byte(code, 0x25);			/* AND AX, */
	emit_word(code, 0x00FF);		/* 0x00FF */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, var);			/* var */
}

void compile_size(Node* ast, CompileTarget* code)
{
	uint16_t rvar = VARS + ('r' - 'a') * 2;
	uint16_t svar = VARS + ('s' - 'a') * 2;

	/* Store filename to AX */
	compile_expression(ast->op1, code);
	emit_byte(code, 0x8B);			/* MOV */
	emit_byte(code, 0xC6);			/* AX, SI */

	/* CALL os_get_file_size */
	emit_call(code, 0x00A5);
	emit_byte(code, 0x72);			/* JC */
	emit_byte(code, 0x08);			/* To failure */

	/* Okay, store size (BX) to S and zero it */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x1E);			/* [imm16], BX */
	emit_word(code, svar);			/* svar */
	emit_byte(code, 0x33);			/* XOR */
	emit_byte(code, 0xDB);			/* BX, BX */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0x04);			/* Over failure */

	/* No such file found, set BX to 1 */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC3);			/* BX, */
	emit_word(code, 0x0001);		/* 1 */

	/* Store BX to R */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x1E);			/* [imm16], BX */
	emit_word(code, rvar);			/* rvar */
}

void compile_sound(Node* ast, CompileTarget* code)
{
	/* Frequency to AX */
	compile_expression(ast->op1, code);

	/* CALL os_speaker_tone */
	emit_call(code, 0x001B);

	/* Duration to AX */
	compile_expression(ast->op2, code);

	/* CALL os_pause and CALL os_speaker_off */
	emit_call(code, 0x0024);
	emit_call(code, 0x001E);
}

void compile_string(Node* ast, CompileTarget* code)
{
	uint16_t var = VARS + ast->op2->op2->op2->val * 2;

	/* String in SI */
	compile_expression(ast->op2->op1, code);
	/* Offset in AX */
	compile_expression(ast->op2->op2->op1, code);

	/* Offsets start from 1, not zero */
	emit_byte(code, 0x03);			/* ADD */
	emit_byte(code, 0xF0);			/* SI, AX */
	emit_byte(code, 0xFF);			/* DEC */
	emit_byte(code, 0xCE);			/* SI */

	/* GET it with LODSB */
	if (ast->op1->attribute == TOKEN_GET) {
		/* Mask upper byte of AX */
		emit_byte(code, 0xAC);		/* LODSB */
		emit_byte(code, 0x25);		/* AND AX, */
		emit_word(code, 0x00FF);

		/* And store it back */
		emit_byte(code, 0x89);		/* MOV */
		emit_byte(code, 0x06);		/* [imm16], AX */
		emit_word(code, var);		/* var */
	}
	/* Set it with STOSB */
	else {
		/* Put SI to DI first */
		emit_byte(code, 0x8B);		/* MOV */
		emit_byte(code, 0xFE);		/* DI, SI */

		/* Compile variable now */
		compile_expression(ast->op2->op2->op2, code);

		/* Set byte and all is done */
		emit_byte(code, 0xAA);		/* STOSB */
	}
}

void compile_waitkey(Node* ast, CompileTarget* code)
{
	uint16_t var = VARS + ast->op1->val * 2;

	/* CALL os_wait_for_key */
	emit_call(code, 0x0012);

	/* Is it special char? */
	emit_byte(code, 0x3D);			/* CMP AX, */
	emit_word(code, 0x48E0);		/* 0x48E0 */
	emit_byte(code, 0x74);			/* JE */
	emit_byte(code, 0x18);			/* To UP (24) */
	emit_byte(code, 0x3D);			/* CMP AX, */
	emit_word(code, 0x50E0);		/* 0x50E0 */
	emit_byte(code, 0x74);			/* JE */
	emit_byte(code, 0x19);			/* To DOWN (25) */
	emit_byte(code, 0x3D);			/* CMP AX, */
	emit_word(code, 0x4BE0);		/* 0x4BE0 */
	emit_byte(code, 0x74);			/* JE */
	emit_byte(code, 0x1A);			/* To LEFT (26) */
	emit_byte(code, 0x3D);			/* CMP AX, */
	emit_word(code, 0x4DE0);		/* 0x4DE0 */
	emit_byte(code, 0x74);			/* JE */
	emit_byte(code, 0x1B);			/* To RIGHT (27) */

	/* Store the character */
	emit_byte(code, 0x25);			/* AND AX, */
	emit_word(code, 0x00FF);		/* 0x00FF */
	emit_byte(code, 0x89);			/* MOV */
	emit_byte(code, 0x06);			/* [imm16], AX */
	emit_word(code, var);
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0x18);			/* Over others (24) */

	/* It was UP */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0001);		/* 1 */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0xF1);			/* To store (-15) */

	/* It was DOWN */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0002);		/* 2 */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0xEB);			/* To store (-21) */

	/* It was LEFT */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0003);		/* 3 */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0xE5);			/* To store (-27) */

	/* It was RIGHT */
	emit_byte(code, 0xC7);			/* MOV */
	emit_byte(code, 0xC0);			/* AX, */
	emit_word(code, 0x0004);		/* 4 */
	emit_byte(code, 0xEB);			/* JMP */
	emit_byte(code, 0xDF);			/* To store (-33) */
}

/* ========================= MAIN COMPILATION CODE ========================== */
typedef void (*KeywordCompileFuncPtr)(Node*, CompileTarget*);
static KeywordCompileFuncPtr compiler[] = {
	[TOKEN_ALERT] = compile_alert,
	[TOKEN_ASKFILE] = compile_askfile,
	[TOKEN_BREAK] = compile_break,
	[TOKEN_CALL] = compile_call,
	[TOKEN_CASE] = compile_case,
	[TOKEN_CLS] = compile_cls,
	[TOKEN_CURSOR] = compile_cursor,
	[TOKEN_CURSCHAR] = compile_curschar,
	[TOKEN_CURSCOL] = compile_curscol,
	[TOKEN_CURSPOS] = compile_curspos,
	[TOKEN_DELETE] = compile_delete,
	[TOKEN_END] = compile_end,
	[TOKEN_FILES] = compile_files,
	[TOKEN_GETKEY] = compile_getkey,
	[TOKEN_GOSUB] = compile_gosub,
	[TOKEN_GOTO] = compile_goto,
	[TOKEN_INCLUDE] = NULL,
	[TOKEN_INK] = compile_ink,
	[TOKEN_INPUT] = compile_input,
	[TOKEN_LEN] = compile_len,
	[TOKEN_LISTBOX] = compile_listbox,
	[TOKEN_LOAD] = compile_load,
	[TOKEN_MOVE] = compile_move,
	[TOKEN_NUMBER] = compile_number,
	[TOKEN_PAGE] = compile_page,
	[TOKEN_PAUSE] = compile_pause,
	[TOKEN_PEEK] = compile_peek,
	[TOKEN_PEEKINT] = compile_peekint,
	[TOKEN_POKE] = compile_poke,
	[TOKEN_POKEINT] = compile_pokeint,
	[TOKEN_PORT] = compile_port,
	[TOKEN_PRINT] = compile_print,
	[TOKEN_RAND] = compile_rand,
	[TOKEN_READ] = NULL,
	[TOKEN_RENAME] = compile_rename,
	[TOKEN_RETURN] = compile_return,
	[TOKEN_SAVE] = compile_save,
	[TOKEN_SERIAL] = compile_serial,
	[TOKEN_SIZE] = compile_size,
	[TOKEN_SOUND] = compile_sound,
	[TOKEN_STRING] = compile_string,
	[TOKEN_WAITKEY] = compile_waitkey
};

void compile_keyword(Node* ast, CompileTarget* code)
{
	TokenType t = ast->attribute;
	KeywordCompileFuncPtr rule = compiler[t];

	/* Compile our keyword */
	rule(ast, code);
}

void init_kword_compiler(StringTable* s, SymbolTable* t, PatchTable* p)
{
	strings = s;
	symbols = t;
	patches = p;
}
