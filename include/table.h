/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

#ifndef TABLE_H
#define TABLE_H

/* Standard library includes */
#include <stdint.h>
#include <stdbool.h>

/* Custom includes */
#include <ast.h>

/* ============================== SYMBOL TABLE ============================== */
typedef struct {
	int id;			/* Entry's index into table */
	const char* str;	/* Content */
	int len;		/* Content's length */
	Node* target;		/* Where label points (NULL if unused) */
	uint16_t addr;		/* It will be set in codegen */
	bool isreal;		/* Was symbol really found in source? */
} SymbolTableEntry;

typedef struct {
	SymbolTableEntry* table;	/* Array of symbol descriptors */
	int len;			/* Length of said array */
	int capacity;			/* Obviously, dynamic array :) */
} SymbolTable;

void init_sym_table(SymbolTable* t);
void free_sym_table(SymbolTable* t);

/* Add symbol (without setting isreal or with) */
int add_unreal_symbol(SymbolTable* t, char* str, int len);
int add_real_symbol(SymbolTable* t, char* str, int len, Node* n);
/* Check if given ID is assigned to real symbol */
bool is_symbol_real(SymbolTable* t, int id);
/* Get ID for given node, or the other way around */
int find_symbol(SymbolTable* t, Node* n);
Node* get_symbol(SymbolTable* t, int id);

/* ============================== STRING TABLE ============================== */
typedef struct {
	int id;				/* Index into table */
	int offset;			/* Offset into string */
	int len;			/* Length of string */
} StringTableEntry;

typedef struct {
	StringTableEntry* table;	/* Array itself */
	int len;			/* Length of array */
	int capacity;			/* Classic dynamic array */
	char* blob;			/* String blob (here offset points) */
	int blob_len;			/* Length of whole string */
} StringTable;

void init_str_table(StringTable* t);
void free_str_table(StringTable* t);

/* Add string */
int add_string(StringTable* t, const char* str, int len);
/* Get string (or just offset) of given ID */
int get_offset_string(StringTable* t, int id);
const char* get_string(StringTable* t, int id);

#endif
