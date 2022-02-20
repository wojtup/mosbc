/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

#ifndef RUNTIME_H
#define RUNTIME_H

/* Custom includes */
#include <table.h>
#include <codegen.h>

/* Setup stack frame */
void make_entry(CompileTarget* code, StringTable* str);

/* Exit program */
void make_exit(CompileTarget* code);

/* Handle division by zero */
void zero_divide_handler(CompileTarget* code);

/* SI = SI + DI (original strings are left unmodified) */
void add_strings(CompileTarget* code);

/* Print SI, using WORKPAGE and INK */
void print_string(CompileTarget* code);

#endif
