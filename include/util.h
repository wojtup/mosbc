/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

#ifndef UTIL_H
#define UTIL_H

void string_uppercase(char* str);
void raise_error();
void check_for_error();		/* This will exit whole program */
char* read_file(const char* filename);

#endif
