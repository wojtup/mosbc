/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

#ifndef PARSER_H
#define PARSER_H

/* Standard library includes */
#include <stdbool.h>

/* Custom includes */
#include <ast.h>
#include <lexer.h>
#include <table.h>

/* Note: Those are internal functions, unused outside of the parser */
void parse_error(const char* msg);
bool match(TokenType t);
Token scan();
Node* init_node(NodeType t, Token token, int v, Node* op1, Node* op2);
void synchronize();
Node* literal();
Node* variable();
Node* string();
Node* numeric();
Node* primary();
Node* expr();
Node* comparison();
Node* boolean_expr();
Node* assign();
Node* if_stmt();
Node* do_stmt();
Node* for_stmt();
Node* parse_keyword();
Node* statement();

/* Here are proper functions: one for parsing, two for node managment */
Node* parse(SymbolTable* t, StringTable* s);
void print_node(Node* n, int lvl);
void free_node(Node* n);

#endif
