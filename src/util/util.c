/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

/* Standard library includes */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Custom includes */
#include <util.h>

bool had_error = false;

void string_uppercase(char* str)
{
	while (*str) {
		*str = toupper(*str);
		str++;
	}
}

void raise_error()
{
	had_error = true;
}

void check_for_error()
{
	if (had_error) {
		printf("%c[33mCompilation terminated due to error(s)!%c[0m\n",
			0x1B, 0x1B);
		exit(-1);
	}
}
