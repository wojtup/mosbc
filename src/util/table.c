/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

/* Standard library includes */
#include <stdlib.h>
#include <string.h>

/* Custom includes */
#include <table.h>

/* ============================== SYMBOL TABLE ============================== */
void init_sym_table(SymbolTable* t)
{
	t->len = 0;
	t->capacity = 8;
	t->table = malloc(t->capacity * sizeof(SymbolTableEntry));
}

void free_sym_table(SymbolTable* t)
{
	free(t->table);
	t->table = NULL;
	t->len = 0;
	t->capacity = 0;
}

int add_unreal_symbol(SymbolTable* t, char* str, int len)
{
	/* If entry is found, just return its index */
	for (int i = 0; i < t->len; i++) {
		if (!strncmp(t->table[i].str, str, t->table[i].len))
			return i;
	}

	/* If it is not, check if we need to make more space */
	if (t->capacity < t->len + 1) {
		t->capacity *= 2;

		t->table = realloc(t->table, t->capacity *
					sizeof(SymbolTableEntry));
	}

	/* Now we now there is space, fill it with new entry */
	t->table[t->len].id = t->len;
	t->table[t->len].str = str;
	t->table[t->len].len = len;
	t->table[t->len].target = NULL;
	t->table[t->len].addr = 0;
	t->table[t->len].isreal = false;
	t->len++;

	return t->len - 1;
}

int add_real_symbol(SymbolTable* t, char* str, int len, Node* n)
{
	/* Use function above, just set needed fields */
	int ret = add_unreal_symbol(t, str, len);
	t->table[ret].target = n;
	t->table[ret].isreal = true;

	return ret;
}

bool is_symbol_real(SymbolTable* t, int id)
{
	/* If index is out of bounds, then it surely isn't real */
	if (t->len < id)
		return false;

	return t->table[id].isreal;
}

int find_symbol(SymbolTable* t, Node* n)
{
	/* Look through table */
	for (int i = 0; i < t->len; i++) {
		if (t->table[i].target == n)
			return i;
	}

	/* If we got nothing, return -1 */
	return -1;
}

Node* get_symbol(SymbolTable* t, int id)
{
	/* If index is out of bounds, then give NULL */
	if (t->len < id)
		return NULL;

	return t->table[id].target;
}

/* ============================== STRING TABLE ============================== */
void init_str_table(StringTable* t)
{
	t->len = 0;
	t->capacity = 8;
	t->table = malloc(t->capacity * sizeof(StringTableEntry));
	t->blob = NULL;
	t->blob_len = 0;
}

void free_str_table(StringTable* t)
{
	free(t->table);
	free(t->blob);
	t->table = NULL;
	t->len = 0;
	t->capacity = 0;
	t->blob = NULL;
	t->blob_len = 0;
}

int add_string(StringTable* t, const char* str, int len)
{
	/* If entry is already present, just return its index */
	for (int i = 0; i < t->len; i++) {
		const char* entry_str = t->blob + t->table[i].offset;
		if (!strcmp(entry_str, str))
			return i;
	}

	/* If it is not, check if we need to make more space */
	if (t->capacity < t->len + 1) {
		t->capacity *= 2;

		t->table = realloc(t->table, t->capacity *
					sizeof(StringTableEntry));
	}

	/* Set entry's values */
	t->table[t->len].id = t->len;
	t->table[t->len].offset = t->blob_len;
	t->table[t->len].len = len;
	t->len++;

	/* Append string to blob */
	t->blob = realloc(t->blob, t->blob_len + len + 1);
	strncpy(t->blob + t->blob_len, str, len);
	t->blob_len += len + 1;
	t->blob[t->blob_len - 1] = '\0';

	return t->len - 1;
}

int get_offset_string(StringTable* t, int id)
{
	if (t->len < id)
		return -1;

	return t->table[id].offset;
}

const char* get_string(StringTable* t, int id)
{
	int off = get_offset_string(t, id);
	if (off == -1)
		return NULL;

	return (const char*) t->blob + off;
}
