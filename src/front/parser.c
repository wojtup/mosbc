/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

/* Standard library includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

/* Custom includes */
#include <parser.h>
#include <table.h>
#include <util.h>

SymbolTable* labels;
StringTable* strings;
Token current;
bool is_current;

/* ================================= UTILITY ================================ */
void parse_error(const char* msg)
{
	printf("\x1B[31mError (parse)\x1B[0m: %s at line: %d, got \"%.*s\".\n",
		msg, current.line, current.length, current.text);
	raise_error();
}

/* Check if next token has type t */
bool match(TokenType t)
{
	Token temp = lookahead();
	current = temp;
	is_current = false;
	if (temp.type == t)
		return true;

	return false;
}

/* Get next token, while saving it (for error reporting) */
Token scan()
{
	current = get_token();
	is_current = true;
	return current;
}

Node* init_node(NodeType t, Token token, int v, Node* op1, Node *op2)
{
	Node* ret = malloc(sizeof(Node));
	ret->type = t;
	ret->attribute = token.type;
	ret->line = token.line;
	ret->val = v;
	ret->op1 = op1;
	ret->op2 = op2;

	return ret;
}

void free_node(Node* n)
{
	if (n == NULL)
		return;

	free_node(n->op1);
	free_node(n->op2);

	free(n);
}

/* Recursively "pretty" prints node */
void print_node(Node* n, int lvl)
{
	for (int i = 0; i < lvl; i++)
		putchar(' ');

	/* Write current node */
	if (n == NULL) {
		printf("(null)\n");
		return;
	}
	printf("(%d %d %d\n", n->type, n->attribute, n->val);

	/* Write left operand */
	print_node(n->op1, lvl + 2);

	/* Write right operand */
	print_node(n->op2, lvl + 2);

	/* Close current node */
	for (int i = 0; i < lvl; i++)
		putchar(' ');
	printf(")\n");
}

/* Skip tokens until we are on keyword (to help parser to get up) */
void synchronize()
{
	Token t = scan();
	while (t.type != TOKEN_EOF && t.type > TOKEN_WHILE) {
		t = scan();
		t = lookahead();
	}
}

/* ============================ HELPER FUNCTIONS ============================ */
/* Parse literal value (strings and numbers) */
Node* literal()
{
	Token t;
	int val;
	if (match(TOKEN_NUMERIC_LITERAL)) {
		t = scan();
		val = atoi(t.text);
	}
	else if (match(TOKEN_STRING_LITERAL)) {
		t = scan();
		val = add_string(strings, t.text, t.length);
	}
	else if (match(TOKEN_CHARACTER_LITERAL)) {
		t = scan();
		val = t.text[0];
	}
	else if (match(TOKEN_PROGSTART) || match(TOKEN_RAMSTART) ||
		 match(TOKEN_VARIABLES) || match(TOKEN_VERSION) ||
	 	 match(TOKEN_TIMER) || match(TOKEN_INK))
		return init_node(NODE_KEYWORD_CALL, scan(), 0, NULL, NULL);
	else {
		parse_error("Expected literal");
		synchronize();
		return NULL;
	}

	return init_node(NODE_LITERAL, t, val, NULL, NULL);
}

/* Parse only one variable */
Node* variable()
{
	Token t;
	int target_idx;
	if (match(TOKEN_NUMERIC_VARIABLE)) {
		t = scan();
		target_idx = toupper(t.text[0]) - 'A';
	}
	else if (match(TOKEN_STRING_VARIABLE)) {
		t = scan();
		target_idx = toupper(t.text[1]) - '1';
	}
	else {
		parse_error("Expected variable");
		synchronize();
		return NULL;
	}

	return init_node(NODE_VARIABLE, t, target_idx, NULL, NULL);
}

/* Parse primary expression - variables, literals and addresses of strings */
Node* primary()
{
	if (match(TOKEN_AMPERSAND)) {
		Token skip = scan();		/* Skip & */
		Node* next = variable();
		return init_node(NODE_EXPR, skip, 0, next, NULL);
	}
	else if (match(TOKEN_NUMERIC_VARIABLE) || match(TOKEN_STRING_VARIABLE))
		return variable();
	else if (match(TOKEN_NUMERIC_LITERAL) || match(TOKEN_STRING_LITERAL) ||
		 match(TOKEN_CHARACTER_LITERAL))
		return literal();
	else if (match(TOKEN_PROGSTART) || match(TOKEN_RAMSTART) ||
		 match(TOKEN_VARIABLES) || match(TOKEN_VERSION) ||
	 	 match(TOKEN_TIMER) || match(TOKEN_INK))
		return init_node(NODE_KEYWORD_CALL, scan(), 0, NULL, NULL);

	parse_error("Expected primary expression");
	synchronize();
	return NULL;
}

/* Parse expression */
Node* expr()
{
	Node* ret = primary();

	while (match(TOKEN_PLUS) || match(TOKEN_MINUS) || match(TOKEN_STAR) ||
	       match(TOKEN_SLASH) || match(TOKEN_PERCENT)) {
		Token next = scan();		/* Skip operator */
		Node* op2 = primary();
		ret = init_node(NODE_EXPR, next, 0, ret, op2);
	}

	return ret;
}

/* Parse only one comparison */
Node* comparison()
{
	Node* target = expr();

	if (match(TOKEN_EQUALS) || match(TOKEN_SMALLER) ||
	    match(TOKEN_GREATER) || match(TOKEN_NOT_EQUALS)) {
		Token op = scan();
		Node* value = expr();
		return init_node(NODE_EXPR, op, 0, target, value);
	}

	parse_error("Expected comparison");
	synchronize();
	free_node(target);
	return NULL;
}

/* Parse whole boolean expression (with AND) */
Node* boolean_expr()
{
	Node* ret = comparison();
	while (match(TOKEN_AND)) {
		Token t = scan();	/* Skip AND */
		Node* op2 = comparison();
		ret = init_node(NODE_EXPR, t, 0, ret, op2);
	}

	return ret;
}

/* ============================ MAIN PARSING CODE =========================== */
/* Parse an assignment */
Node* assign()
{
	Node* target = variable();

	if (match(TOKEN_EQUALS)) {
		Token t = scan();	/* Discard = */
		Node* val = expr();
		Node* ret = init_node(NODE_ASSIGN, t, 0, target, val);
		return ret;
	}

	parse_error("Expected assignment");
	synchronize();
	free_node(target);
	return NULL;
}

/* Parse an IF statement */
Node* if_stmt()
{
	Token skip = scan();	/* Discard IF */
	Token t;

	Node* cond = boolean_expr();

	/* Expect THEN */
	if (!match(TOKEN_THEN)) {
		parse_error("Expected THEN");
		synchronize();
		free_node(cond);
		return NULL;
	}
	t = scan();			/* Discard THEN */

	Node* then_case = statement();
	Node* else_case = NULL;

	/* We have an else branch */
	if (match(TOKEN_ELSE)) {
		t = scan();		/* Discard ELSE */
		else_case = statement();
	}

	Node* then_else = init_node(NODE_SEQUENCE, t, 0, then_case, else_case);
	return init_node(NODE_IF, skip, 0, cond, then_else);
}

/* Parse a DO loop */
Node* do_stmt()
{
	Token skip = scan();	/* Discard DO */
	Token t = skip;

	Node* ret = NULL;
	Node* body = NULL;	/* Body of the loop */
	Node* mod = NULL;	/* Keyword modifier (UNTIL, WHILE or ENDLESS) */
	Node* cond = NULL;	/* Condition */

	/* Parse the body */
	while (!match(TOKEN_EOF) && !match(TOKEN_LOOP)) {
		Node* stmt = statement();
		body = init_node(NODE_SEQUENCE, t, 0, body, stmt);
	}

	if (scan().type != TOKEN_LOOP) {	/* Discard LOOP */
		parse_error("Reached End Of File before LOOP");
		free_node(body);
		return NULL;
	}

	/* Parse modifier and condition */
	if (match(TOKEN_ENDLESS)) {
		t = scan();	/* Discard ENDLESS */
	}
	else if (match(TOKEN_WHILE) || match(TOKEN_UNTIL)) {
		t = scan();	/* Discard WHILE or UNTIL */
		cond = boolean_expr();
	}
	else {
		parse_error("Expected LOOP modifier (UNTIL, WHILE or ENDLESS)");
		free_node(body);
		free_node(cond);
		return NULL;
	}

	mod = init_node(NODE_KEYWORD_CALL, t, 0, NULL, NULL);
	ret = init_node(NODE_DO, skip, 0, cond,
			init_node(NODE_SEQUENCE, t, 0, body, mod));
	return ret;
}

/* Parse a FOR loop */
Node* for_stmt()
{
	Token t = scan();	/* Discard FOR */

	/* We can't use string variable */
	if (!match(TOKEN_NUMERIC_VARIABLE)) {
		parse_error("FOR loops require numeric iterator");
		synchronize();
		return NULL;
	}

	Node* init = assign();		/* Initializer */
	Node* to = NULL;		/* "To" field */
	Node* body = NULL;		/* Body of the loop */

	if (init == NULL || init->op1 == NULL) {
		parse_error("Expected valid initializer in FOR");
		synchronize();
		return NULL;
	}

	if (!match(TOKEN_TO)) {
		parse_error("Expected TO keyword in FOR loop");
		synchronize();
		free_node(init);
		return NULL;
	}
	t = scan();		/* Discard TO */

	/* Parse "To" field */
	to = expr();

	/* Parse the body */
	while (!match(TOKEN_EOF) && !match(TOKEN_NEXT)) {
		Node* stmt = statement();
		body = init_node(NODE_SEQUENCE, t, 0, body, stmt);
	}

	if (scan().type != TOKEN_NEXT) {	/* Discard NEXT */
		parse_error("Reached End Of File before LOOP");
		free_node(init);
		free_node(to);
		free_node(body);
		return NULL;
	}

	/* Check if NEXT points to loop's variable */
	Node* temp = variable();
	if (temp->attribute != init->op1->attribute ||
	    temp->val != init->op1->val) {
		parse_error("Incorrect target for NEXT");
		/* Don't synchronize, we know what is going on */
		free_node(init);
		free_node(to);
		free_node(body);
		return NULL;
	}

	return init_node(NODE_FOR, t, 0, init,
		init_node(NODE_SEQUENCE, t, 0, to, body));
}

/* Parse whole statement (calling appropriate functions) */
Node* statement()
{
	/* End of file, stop parsing */
	if (match(TOKEN_EOF))
		return NULL;

	/* If it is a variable, parse assignment */
	if (match(TOKEN_NUMERIC_VARIABLE) || match(TOKEN_STRING_VARIABLE))
		return assign();

	/* If it is an IF, parse conditional */
	if (match(TOKEN_IF))
		return if_stmt();

	/* If it is DO, parse loop */
	if (match(TOKEN_DO))
		return do_stmt();

	/* Parse FOR */
	if (match(TOKEN_FOR))
		return for_stmt();

	/* Labels aren't proper statements, but we need to remember them */
	if (match(TOKEN_LABEL)) {
		Token t = scan();		/* Grab the label */
		Node* ret = statement();
		add_real_symbol(labels, (char*) t.text, t.length, ret);
		return ret;
	}

	/* Is it a normal keyword? */
	Token t = lookahead();
	if (t.type >= TOKEN_ALERT && t.type <= TOKEN_WHILE)
		return parse_keyword(labels);

	/* We found something that isn't proper statement, synchronize */
	parse_error("Expected statement");
	synchronize();
	return NULL;
}

Node* parse(SymbolTable* t, StringTable* s)
{
	Token empty = {0, NULL, 0, 0};
	labels = t;
	strings = s;
	Node* ret = init_node(NODE_SEQUENCE, empty, 0, statement(), NULL);
	if (!match(TOKEN_EOF))
		ret->op2 = parse(t, s);

	return ret;
}
