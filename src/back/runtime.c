/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

/* Custom includes */
#include <table.h>
#include <codegen.h>

void make_exit(CompileTarget* code)
{
	/* Reset active page */
	emit_byte(code, 0xC7);		/* MOV */
	emit_byte(code, 0xC0);		/* AX, */
	emit_word(code, 0x0500);	/* AH = 5, AL = 0 */
	emit_byte(code, 0xCD);		/* INT */
	emit_byte(code, 0x10);		/* 0x10 */

	/* Rewind stack */
	emit_byte(code, 0x8B);		/* MOV */
	emit_byte(code, 0xE5);		/* SP, BP */

	/* Exit the program */
	emit_byte(code, 0xC3);		/* RET */
}

/* Length of this handler: 4 + 3 + 9 + 32 = 48 bytes */
void zero_divide_handler(CompileTarget* code)
{
	/* Print message (it is 2 + 3 + 3 = 8 bytes) */
	emit_byte(code, 0xC7);				/* MOV */
	emit_byte(code, 0xC6);				/* SI, */
	emit_word(code, LOAD + code->length + 8);	/* imm16 */

	/* Call os_print_string */
	emit_call(code, 0x0003);

	/* Exit */
	make_exit(code);

	emit_string(code, "BASIC Runtime: Division by zero");
}

/* Length of this handler: 1 + 4 + 3 + 2 + 1 + 3 * 2 + 3 = 20 bytes */
void add_strings(CompileTarget* code)
{
	emit_byte(code, 0x57);		/* PUSH DI */
	emit_byte(code, 0xC7);		/* MOV */
	emit_byte(code, 0xC7);		/* DI, */
	emit_word(code, STRBUF);	/* imm16 */

	/* CALL os_string_copy */
	emit_call(code, 0x0039);

	emit_byte(code, 0x8B);		/* MOV */
	emit_byte(code, 0xF7);		/* SI, DI */

	emit_byte(code, 0x5F);		/* POP DI */

	/* Now SI = buffer with old SI, DI = 2nd string */
	emit_byte(code, 0x8B);		/* MOV */
	emit_byte(code, 0xC6);		/* AX, SI */
	emit_byte(code, 0x8B);		/* MOV */
	emit_byte(code, 0xDF);		/* BX, DI */
	emit_byte(code, 0x8B);		/* MOV */
	emit_byte(code, 0xCE);		/* CX, SI */

	/* CALL os_string_join */
	emit_call(code, 0x003F);

	/* Return from handler */
	emit_byte(code, 0xC3);
}

/* Length of this handler: 80 bytes */
void print_string(CompileTarget* code)
{
	/* Prepare to enter print loop (19 bytes) */
	emit_byte(code, 0xC7);		/* MOV */
	emit_byte(code, 0xC0);		/* AX, */
	emit_word(code, 0x0900);	/* 0x900 -> AH = 9 */
	emit_byte(code, 0x8B);		/* MOV */
	emit_byte(code, 0x1E);		/* BX, */
	emit_word(code, WORKPAGE);	/* [WORKPAGE] */
	emit_byte(code, 0xC1);		/* SHL */
	emit_byte(code, 0xE3);		/* BX, */
	emit_byte(code, 0x08);		/* 8 */
	emit_byte(code, 0x0B);		/* OR */
	emit_byte(code, 0x1E);		/* BX, */
	emit_word(code, INKADDR);	/* [INKADDR] */
	emit_byte(code, 0xC7);		/* MOV */
	emit_byte(code, 0xC1);		/* CX, */
	emit_word(code, 0x0001);	/* 1 */

	/* Loop itself (11 bytes) */
	emit_byte(code, 0xAC);		/* LODSB */
	emit_byte(code, 0x3D);		/* CMP AX, */
	emit_word(code, 0x0900);	/* 0x900 -> AL = 0 */
	emit_byte(code, 0x74);		/* JE */
	emit_byte(code, 0x35);		/* To the end (+53) */
	emit_byte(code, 0x3D);		/* CMP AX, */
	emit_word(code, 0x090A);	/* 0x90A -> AL = 0x0A */
	emit_byte(code, 0x74);		/* JE */
	emit_byte(code, 0x16);		/* To newline (+22) */

	/* Print character (2 bytes) */
	emit_byte(code, 0xCD);		/* INT */
	emit_byte(code, 0x10);		/* 0x10 */

	/* Get cursor postion and adjust (20 bytes) */
	emit_byte(code, 0x50);		/* PUSH AX */
	emit_byte(code, 0x51);		/* PUSH CX */
	emit_byte(code, 0xC7);		/* MOV */
	emit_byte(code, 0xC0);		/* AX, */
	emit_word(code, 0x0300);	/* AH = 3 */
	emit_byte(code, 0xCD);		/* INT */
	emit_byte(code, 0x10);		/* 0x10 */
	emit_byte(code, 0xFF);		/* INC */
	emit_byte(code, 0xC2);		/* DX */
	emit_byte(code, 0xC7);		/* MOV */
	emit_byte(code, 0xC0);		/* AX, */
	emit_word(code, 0x0200);	/* AH = 2 */
	emit_byte(code, 0xCD);		/* INT */
	emit_byte(code, 0x10);		/* 0x10 */
	emit_byte(code, 0x59);		/* POP CX */
	emit_byte(code, 0x58);		/* POP AX */
	emit_byte(code, 0xEB);		/* JMP */
	emit_byte(code, 0xDF);		/* Back to the loop (-33) */

	/* We got 0x0A, move to next line (26 bytes) */
	emit_byte(code, 0x50);		/* PUSH AX */
	emit_byte(code, 0x53);		/* PUSH BX */
	emit_byte(code, 0xC7);		/* MOV */
	emit_byte(code, 0xC0);		/* AX, */
	emit_word(code, 0x0500);	/* AH = 5 */
	emit_byte(code, 0x0B);		/* OR */
	emit_byte(code, 0x06);		/* AX, */
	emit_word(code, WORKPAGE);	/* [WORKPAGE] */
	emit_byte(code, 0xCD);		/* INT */
	emit_byte(code, 0x10);		/* 0x10 */
	emit_byte(code, 0x8B);		/* MOV */
	emit_byte(code, 0x1E);		/* BX, */
	emit_word(code, WORKPAGE);	/* [WORKPAGE] */
	emit_byte(code, 0xC1);		/* SHL */
	emit_byte(code, 0xE3);		/* BX, */
	emit_byte(code, 0x08);		/* 8 */
	/* CALL os_print_newline */
	emit_call(code, 0x000F);
	emit_byte(code, 0x5B);		/* POP BX */
	emit_byte(code, 0x58);		/* POP AX */
	emit_byte(code, 0xEB);		/* JMP */
	emit_byte(code, 0xC5);		/* Back to the loop (-59) */

	/* Return from the handler and padding for alignment (2 bytes) */
	emit_byte(code, 0xC3);		/* RET */
	emit_byte(code, 0x90);		/* NOP */
}

void make_entry(CompileTarget* code, StringTable* strings)
{
	int len = strings->blob_len;
	int rel = RUNTIMELEN + len;		/* Runtime + strings */

	/* Jump over runtime and strings */
	emit_jump(code, LOAD + rel);

	/* Runtime functions */
	add_strings(code);
	zero_divide_handler(code);
	print_string(code);

	/* Empty places for different values */
	emit_word(code, 0x0007);	/* INK (default 7) */
	emit_word(code, 0x0000);	/* RAMSTART (codegen will fill it) */
	emit_word(code, 0x0000);	/* WORKPAGE */
	emit_word(code, 0x0000);	/* ACTIVEPAGE */

	/* Write out string table */
	for (int i = 0; i < len; i++)
		emit_byte(code, strings->blob[i]);

	/* Clear out numeric variables */
	emit_byte(code, 0x33);		/* XOR */
	emit_byte(code, 0xC0);		/* AX, AX */
	emit_byte(code, 0xC7);		/* MOV */
	emit_byte(code, 0xC7);		/* DI, */
	emit_word(code, VARS);		/* VARS */
	emit_byte(code, 0xC7);		/* MOV */
	emit_byte(code, 0xC1);		/* CX, */
	emit_word(code, 0x002E);	/* 46 */
	emit_byte(code, 0xF3);		/* REP */
	emit_byte(code, 0xAA);		/* STOSB */

	/* Clear out string variables */
	emit_byte(code, 0xC7);		/* MOV */
	emit_byte(code, 0xC7);		/* DI, */
	emit_word(code, STRVARS);	/* STRVARS */
	emit_byte(code, 0xC7);		/* MOV */
	emit_byte(code, 0xC1);		/* CX, */
	emit_word(code, 0x0400);	/* 1024 */
	emit_byte(code, 0xF3);		/* REP */
	emit_byte(code, 0xAA);		/* STOSB */

	/* Setup stack */
	emit_byte(code, 0x8B);		/* MOV */
	emit_byte(code, 0xEC);		/* BP, SP */
}
