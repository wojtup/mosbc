/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

/* Standard library includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Custom includes */
#include <codegen.h>

/* ============================== PATCH TABLES ============================== */
void init_patch(PatchTable* p)
{
	p->length = 0;
	p->capacity = 8;
	p->table = malloc(p->capacity * sizeof(PatchTableEntry));
}

void free_patch(PatchTable* p)
{
	free(p->table);
	p->table = NULL;
	p->length = 0;
	p->capacity = 0;
}

void add_patch(PatchTable* p, int id, uint16_t addr)
{
	if (p->capacity < p->length + 1) {
		p->capacity *= 2;
		p->table = realloc(p->table, p->capacity *
				sizeof(PatchTableEntry));
	}

	p->table[p->length].id = id;
	p->table[p->length].addr = addr;
	p->length++;
}

/* ============================= INITIALIZATION ============================= */
void init_code(CompileTarget* c)
{
	c->length = 0;
	c->capacity = 8;
	c->code = malloc(c->capacity);
}

void free_code(CompileTarget* c)
{
	free(c->code);
	c->code = NULL;
	c->length = 0;
	c->capacity = 0;
}

void patch_jumps(CompileTarget* c, PatchTable* p, SymbolTable* sym, Node* n)
{
	uint16_t addr = c->length;

	/* Search all labels whether Node is in them */
	for (int i = 0; i < sym->len; i++)
		if (sym->table[i].target == n) {
			sym->table[i].addr = addr;

			/* Now look if there are any matching entries */
			for (int j = 0; j < p->length; j++)
				if (p->table[j].id == i) {
					uint16_t a = p->table[j].addr;
					int16_t rel = addr - (a + 2);
					c->code[a] = (uint8_t) rel & 0xFF;
					c->code[a + 1] = (uint8_t) (rel >> 8) &
								0xFF;
				}
		}
}

/* =========================== EMITTING FUNCTIONS =========================== */
void emit_byte(CompileTarget* c, uint8_t byte)
{
	/* If there is no room, make some */
	if (c->capacity < c->length + 1) {
		c->capacity *= 2;
		c->code = realloc(c->code, c->capacity);
	}

	/* Put our byte in place */
	c->code[c->length] = byte;
	c->length++;
}

void emit_word(CompileTarget* c, uint16_t word)
{
	emit_byte(c, word & 0xFF);
	emit_byte(c, word >> 8);
}

void emit_call(CompileTarget* c, uint16_t target)
{
	uint16_t next = c->length + 3 + LOAD;
	int16_t rel = target - next;
	emit_byte(c, 0xE8);	/* CALL */
	emit_word(c, rel);	/* rel16 */
}

void emit_jump(CompileTarget* c, uint16_t target)
{
	uint16_t next = c->length + 3 + LOAD;
	int16_t rel = target - next;
	emit_byte(c, 0xE9);		/* JMP NEAR */
	emit_word(c, rel);		/* rel16 */
}

void emit_string(CompileTarget* c, const char* str)
{
	for (unsigned int i = 0; i < strlen(str); i++)
		emit_byte(c, str[i]);

	emit_byte(c, 0x00);	/* NUL terminate */
}
