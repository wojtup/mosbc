/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

/* Standard library includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Custom includes */
#include <lexer.h>
#include <parser.h>
#include <table.h>
#include <codegen.h>
#include <util.h>

extern Lexer lexer;

char* read_file(const char* filename)
{
	FILE* f = fopen(filename, "rb");
	if (f == NULL) {
		printf("\x1B[31mError\x1B[0m: Could not open source file.\n");
		raise_error();
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	rewind(f);

	char* source = malloc(len + 1);		/* One more for NUL */
	if (source == NULL) {
		printf("\x1B[31mError\x1B[0m: Could not allocate to read.\n");
		raise_error();
		return NULL;
	}

	int read = fread(source, sizeof(char), len, f);
	if (read != len) {
		printf("\x1B[31mError\x1B[0m: Could not read source file.\n");
		raise_error();
		return NULL;
	}

	source[len] = '\0';			/* Zero terminate string */
	fclose(f);
	return source;
}

void write_file(const char* filename, CompileTarget* ct)
{
	FILE* f = fopen(filename, "wb");
	if (f == NULL) {
		printf("\x1B[31mError\x1B[0m: Could not open output file.\n");
		raise_error();
		return;
	}

	int len = fwrite(ct->code, sizeof(char), ct->length, f);

	if (len != ct->length) {
		printf("\x1B[31mError\x1B[0m: Could not write output file.\n");
		raise_error();
		return;
	}

	fclose(f);
}

int main(int argc, char** argv)
{
	if (argc < 3) {
		printf("----- \x1B[33mMikeOS Basic Compiler\x1B[0m -----\n"
			"Usage: mosbc \x1B[35msrc\x1B[0m \x1B[36mout\x1B[0m "
			"\x1B[33m[-debug]\x1B[0m\n"
			"  \x1B[35msrc\x1B[0m - Name of the source file\n"
			"  \x1B[36mout\x1B[0m - Name of output file\n"
			"  \x1B[33m-debug\x1B[0m - Print compiler data "
			"structures.\n");
		return -1;
	}

	/* Read */
	char* src = read_file(argv[1]);
	check_for_error();

	/* Now we read the source, initialize all data structures */
	SymbolTable t;
	StringTable s;
	CompileTarget ct;
	init_sym_table(&t);
	init_str_table(&s);
	init_code(&ct);

	/* Parse */
	init_lexer(src);
	Node* ast = parse(&t, &s);
	check_for_error();

	/* Compile */
	compile(ast, &ct, &s, &t);
	check_for_error();

	/* Output debug info, if needed */
	if (argc == 4 && strcmp(argv[3], "-debug") == 0) {
		printf("\x1B[32mSource:\x1B[0m\n%s\n\n", lexer.source);
		printf("\x1B[36mAST\x1B[0m:\n");
		print_node(ast, 0);
		printf("\n\x1B[34mASM:\x1B[0m\n");
		disassemble(&ct);
	}

	/* Finally, write out our compiled code to file */
	write_file(argv[2], &ct);

	/* Please be reassuring: */
	printf("\x1B[32mCompilation successful\x1B[0m: written file %s "
		"(%d bytes long)\n", argv[2], ct.length);

	/* Clean up */
	free_node(ast);
	free_code(&ct);
	free_sym_table(&t);
	free_str_table(&s);

	return 0;
}
