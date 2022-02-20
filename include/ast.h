/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

#ifndef AST_H
#define AST_H

/* Standard library includes */
#include <stdbool.h>

/* Custom includes */
#include <lexer.h>

typedef enum {
	NODE_SEQUENCE = 0,	/* Synthetic node, puts 2 nodes in order */
	NODE_ASSIGN = 1,	/* Left operand is target, right is value */
	NODE_EXPR = 2,		/* Generic node for expressions */
	NODE_VARIABLE = 3,	/* Variables (both string and numeric) */
	NODE_LITERAL = 4,	/* Literals (numeric and string) */
	NODE_LABEL = 5,		/* Labels (as targets for GOTO and GOSUB) */
	NODE_IF = 6,		/* Left op is condition, right then-else */
	NODE_DO = 7,		/* Same as above, right is body and modifiers */
	NODE_FOR = 8,		/* Left - initializer, right - "to" and body */
	NODE_KEYWORD_CALL = 9	/* Synthetic token, keyword is in attribute */
} NodeType;

typedef struct _Node {
	NodeType type;		/* Generic type of Node */
	TokenType attribute;	/* Used to distinguish subtypes, 0 if unused */
	int val;		/* Literal value, else 0 */
	int line;		/* Line on which Node lies */
	struct _Node* op1;	/* Both operands are NULL if unused */
	struct _Node* op2;
} Node;

#endif
