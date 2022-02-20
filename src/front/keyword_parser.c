/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

/* Standard library includes */
#include <stdio.h>

/* Custom includes */
#include <parser.h>
#include <table.h>

SymbolTable* klabels;	/* Keyword LABELS */

/* ============================ HELPER FUNCTIONS ============================ */
Node* string()
{
	if (match(TOKEN_STRING_VARIABLE))
		return variable();
	if (match(TOKEN_STRING_LITERAL))
		return literal();

	parse_error("Expected string expression");
	synchronize();
	return NULL;
}

Node* numeric()
{
	if (match(TOKEN_NUMERIC_VARIABLE))
		return variable();
	if (match(TOKEN_NUMERIC_LITERAL))
		return literal();
	if (match(TOKEN_PROGSTART) || match(TOKEN_RAMSTART) ||
	    match(TOKEN_VARIABLES) || match(TOKEN_VERSION) ||
	    match(TOKEN_TIMER) || match(TOKEN_INK))
		return literal();
	if (match(TOKEN_AMPERSAND))
		return primary();

	parse_error("Expected numeric expression");
	synchronize();
	return NULL;
}

Node* label()
{
	if (match(TOKEN_IDENTIFIER)) {
		Token t = scan();
		int val = add_unreal_symbol(klabels, (char*) t.text, t.length);
		return init_node(NODE_LABEL, t, val, NULL, NULL);
	}

	parse_error("Expected label (note: labels can't be keywords!)");
	synchronize();
	return NULL;
}

Node* init_keyword(Token t, Node* op1, Node* op2)
{
	return init_node(NODE_KEYWORD_CALL, t, 0, op1, op2);
}

/* =========================== PARSING FUNCTIONS ============================ */
Node* do_alert()
{
	Token t = scan();	/* Discard ALERT */
	Node* op = string();
	return init_keyword(t, op, NULL);
}

Node* do_askfile()
{
	Token t = scan();	/* Discard ASKFILE */
	if (match(TOKEN_STRING_VARIABLE)) {
		Node* op = variable();
		return init_keyword(t, op, NULL);
	}

	parse_error("Expected string variable after ASKFILE");
	synchronize();
	return NULL;
}

Node* do_break()
{
	Token t = scan();	/* Discard BREAK */
	return init_keyword(t, NULL, NULL);
}

Node* do_call()
{
	Token t = scan();	/* Discard CALL */
	Node* op = expr();

	return init_keyword(t, op, NULL);
}

Node* do_case()
{
	Token t = scan();	/* You get the point... */
	if (match(TOKEN_LOWER) || match(TOKEN_UPPER)) {
		Node* op1 = init_keyword(scan(), NULL, NULL);
		if (match(TOKEN_STRING_VARIABLE)) {
			Node* op2 = variable();
			return init_keyword(t, op1, op2);
		}

		parse_error("Expected string variable after CASE");
		synchronize();
		free_node(op1);
		return NULL;
	}

	parse_error("Expected LOWER or UPPER after CASE");
	synchronize();
	return NULL;
}

Node* do_cls()
{
	Token t = scan();
	return init_keyword(t, NULL, NULL);
}

Node* do_cursor()
{
	Token t = scan();
	if (match(TOKEN_ON) || match(TOKEN_OFF)) {
		Token mod = scan();
		Node* op = init_keyword(mod, NULL, NULL);
		return init_keyword(t, op, NULL);
	}

	parse_error("Expected ON or OFF after CURSOR");
	synchronize();
	return NULL;
}

Node* do_curschar()
{
	Token t = scan();
	if (match(TOKEN_NUMERIC_VARIABLE)) {
		Node* op = variable();
		return init_keyword(t, op, NULL);
	}

	parse_error("Expected numeric variable after CURSCHAR");
	synchronize();
	return NULL;
}

Node* do_curscol()
{
	Token t = scan();
	if (match(TOKEN_NUMERIC_VARIABLE)) {
		Node* op = variable();
		return init_keyword(t, op, NULL);
	}

	parse_error("Expected numeric variable after CURSCOL");
	synchronize();
	return NULL;
}

Node* do_curspos()
{
	Token t = scan();
	if (match(TOKEN_NUMERIC_VARIABLE)) {
		Node* op1 = variable();
		if (match(TOKEN_NUMERIC_VARIABLE)) {
			Node* op2 = variable();
			return init_keyword(t, op1, op2);
		}

		free_node(op1);
	}

	parse_error("Expected two numeric variables after CURSPOS");
	synchronize();
	return NULL;
}

Node* do_delete()
{
	Token t = scan();
	Node* op = string();
	return init_keyword(t, op, NULL);
}

Node* do_end()
{
	Token t = scan();
	return init_keyword(t, NULL, NULL);
}

Node* do_files()
{
	Token t = scan();
	return init_keyword(t, NULL, NULL);
}

Node* do_getkey()
{
	Token t = scan();
	if (match(TOKEN_NUMERIC_VARIABLE)) {
		Node* op = variable();
		return init_keyword(t, op, NULL);
	}

	parse_error("Expected numeric variable after GETKEY");
	synchronize();
	return NULL;
}

Node* do_gosub()
{
	Token t = scan();
	Node* op = label();
	return init_keyword(t, op, NULL);
}

Node* do_goto()
{
	Token t = scan();
	Node* op = label();
	return init_keyword(t, op, NULL);
}

Node* do_include()
{
	/* Actually we don't need it */
	scan();
	scan();

	return parse_keyword(klabels);
}

Node* do_ink()
{
	Token t = scan();
	Node* op = numeric();
	return init_keyword(t, op, NULL);
}

Node* do_input()
{
	Token t = scan();
	Node* op = variable();
	return init_keyword(t, op, NULL);
}

Node* do_len()
{
	Token t = scan();
	Node* op1 = string();

	if (match(TOKEN_NUMERIC_VARIABLE)) {
		Node* op2 = variable();
		return init_keyword(t, op1, op2);
	}

	parse_error("Expected numeric variable as a target for LEN");
	synchronize();
	free_node(op1);
	return NULL;
}

Node* do_listbox()
{
	Token t = scan();

	Node* op1 = string();
	Node* op2 = string();
	Node* op3 = string();
	if (match(TOKEN_NUMERIC_VARIABLE)) {
		Node* op4 = variable();
		Node* subseq = init_node(NODE_SEQUENCE, t, 0, op3, op4);
		Node* seq = init_node(NODE_SEQUENCE, t, 0, op2, subseq);
		return init_keyword(t, op1, seq);
	}

	parse_error("Expected numeric variable as a target for LISTBOX");
	synchronize();
	free_node(op1);
	free_node(op2);
	free_node(op3);
	return NULL;
}

Node* do_load()
{
	Token t = scan();

	Node* op1 = string();
	Node* op2 = numeric();

	return init_keyword(t, op1, op2);
}

Node* do_move()
{
	Token t = scan();

	Node* op1 = numeric();
	Node* op2 = numeric();

	return init_keyword(t, op1, op2);
}

Node* do_number()
{
	Token t = scan();

	Node* op1, *op2;
	if (match(TOKEN_STRING_VARIABLE)) {
		op1 = variable();
		if (!match(TOKEN_NUMERIC_VARIABLE)) {
			parse_error("Expected numeric variable in NUMBER");
			synchronize();
			free_node(op1);
			return NULL;
		}
		op2 = variable();
	}
	else if (match(TOKEN_NUMERIC_VARIABLE)) {
		op1 = variable();
		if (!match(TOKEN_STRING_VARIABLE)) {
			parse_error("Expected string variable in NUMBER");
			synchronize();
			free_node(op1);
			return NULL;
		}
		op2 = variable();
	}
	else {
		parse_error("Expected variable as a source for NUMBER");
		synchronize();
		return NULL;
	}

	return init_keyword(t, op1, op2);
}

Node* do_page()
{
	Token t = scan();
	Node* op1 = numeric();
	Node* op2 = numeric();

	return init_keyword(t, op1, op2);
}

Node* do_pause()
{
	Token t = scan();
	Node* op = numeric();

	return init_keyword(t, op, NULL);
}

Node* do_peek()
{
	Token t = scan();

	if (!match(TOKEN_NUMERIC_VARIABLE)) {
		parse_error("Expected numeric target for PEEK");
		synchronize();
		return NULL;
	}
	Node* op1 = variable();
	Node* op2 = numeric();

	return init_keyword(t, op1, op2);
}

Node* do_peekint()
{
	Token t = scan();

	if (!match(TOKEN_NUMERIC_VARIABLE)) {
		parse_error("Expected numeric target for PEEKINT");
		synchronize();
		return NULL;
	}
	Node* op1 = variable();
	Node* op2 = numeric();

	return init_keyword(t, op1, op2);
}

Node* do_poke()
{
	Token t = scan();
	Node* op1 = numeric();
	Node* op2 = numeric();

	return init_keyword(t, op1, op2);
}

Node* do_pokeint()
{
	Token t = scan();
	Node* op1 = numeric();
	Node* op2 = numeric();

	return init_keyword(t, op1, op2);
}

Node* do_port()
{
	Token t = scan();
	Node* mod, *op2, *op3;
	if (match(TOKEN_IN)) {
		mod = init_keyword(scan(), NULL, NULL);
		op2 = numeric();
		if (!match(TOKEN_NUMERIC_VARIABLE)) {
			parse_error("Expected numeric target for PORT IN");
			synchronize();
			free_node(mod);
			free_node(op2);
			return NULL;
		}
		op3 = variable();
	}
	else if (match(TOKEN_OUT)) {
		mod = init_keyword(scan(), NULL, NULL);
		op2 = numeric();
		op3 = numeric();
	}
	else {
		parse_error("Expected modifier for PORT (IN or OUT)");
		synchronize();
		return NULL;
	}

	Node* seq = init_node(NODE_SEQUENCE, t, 0, op2, op3);
	return init_keyword(t, mod, seq);
}

Node* do_print()
{
	Token t = scan();
	Node* mod1 = NULL;
	if (match(TOKEN_CHR) || match(TOKEN_HEX))
		mod1 = init_keyword(scan(), NULL, NULL);

	Node* op = expr();
	Node* mod2 = NULL;
	if (match(TOKEN_SEMICOLON))
		mod2 = init_keyword(scan(), NULL, NULL);

	Node* seq = init_node(NODE_SEQUENCE, t, 0, op, mod2);
	return init_keyword(t, mod1, seq);
}

Node* do_rand()
{
	Token t = scan();
	if (!match(TOKEN_NUMERIC_VARIABLE)) {
		parse_error("Expected numeric target for RAND");
		synchronize();
		return NULL;
	}

	Node* target = variable();
	Node* low = numeric();
	Node* high = numeric();

	Node* seq = init_node(NODE_SEQUENCE, t, 0, low, high);
	return init_keyword(t, target, seq);
}

Node* do_read()
{
	Token t = scan();

	Node* l = label();
	Node* offset = numeric();
	if (!match(TOKEN_NUMERIC_VARIABLE)) {
		parse_error("Expected numeric target for READ");
		synchronize();
		free_node(l);
		free_node(offset);
		return NULL;
	}

	Node* target = variable();

	Node* seq = init_node(NODE_SEQUENCE, t, 0, offset, target);

	parse_error("READ is not supported (sorry). It is");
	free_node(l);
	free_node(seq);
	return NULL;
}

Node* do_rename()
{
	Token t = scan();
	Node* op1 = string();
	Node* op2 = string();

	return init_keyword(t, op1, op2);
}

Node* do_return()
{
	Token t = scan();

	return init_keyword(t, NULL, NULL);
}

Node* do_save()
{
	Token t = scan();

	Node* name = string();
	Node* pos = numeric();
	Node* len = numeric();

	Node* seq = init_node(NODE_SEQUENCE, t, 0, pos, len);

	return init_keyword(t, name, seq);
}

Node* do_serial()
{
	Token t = scan();

	Node* mod, *val;
	if (match(TOKEN_ON) || match(TOKEN_SEND)) {
		mod = init_keyword(scan(), NULL, NULL);
		val = numeric();
	}
	else if (match(TOKEN_REC)) {
		mod = init_keyword(scan(), NULL, NULL);
		if (!match(TOKEN_NUMERIC_VARIABLE)) {
			parse_error("Expected numeric target for SERIAL REC");
			synchronize();
			free_node(mod);
			return NULL;
		}
		val = variable();
	}
	else {
		parse_error("Expected modifier for SERIAL (ON, SEND or REC)");
		synchronize();
		return NULL;
	}

	return init_keyword(t, mod, val);
}

Node* do_size()
{
	Token t = scan();
	Node* op = string();

	return init_keyword(t, op, NULL);
}

Node* do_sound()
{
	Token t = scan();
	Node* op1 = numeric();
	Node* op2 = numeric();

	return init_keyword(t, op1, op2);
}

Node* do_string()
{
	Token t = scan();

	if (!match(TOKEN_GET) && !match(TOKEN_SET)) {
		parse_error("Expected modifier for STRING (GET or SET)");
		synchronize();
		return NULL;
	}

	Node* mod = init_keyword(scan(), NULL, NULL);

	if (!match(TOKEN_STRING_VARIABLE)) {
		parse_error("Expected string variable for STRING");
		synchronize();
		free_node(mod);
		return NULL;
	}

	Node* target = variable();
	Node* offset = numeric();

	if (!match(TOKEN_NUMERIC_VARIABLE)) {
		parse_error("Expected numeric target for STRING");
		synchronize();
		free_node(mod);
		free_node(target);
		free_node(offset);
		return NULL;
	}

	Node* num = variable();

	Node* subseq = init_node(NODE_SEQUENCE, t, 0, offset, num);
	Node* seq = init_node(NODE_SEQUENCE, t, 0, target, subseq);

	return init_keyword(t, mod, seq);
}

Node* do_waitkey()
{
	Token t = scan();

	if (!match(TOKEN_NUMERIC_VARIABLE)) {
		parse_error("Expected numeric target for WAITKEY");
		synchronize();
		return NULL;
	}

	Node* op = variable();

	return init_keyword(t, op, NULL);
}

/* ============================== ENTRY POINT =============================== */
typedef Node* (*FunctionPtr)();
static FunctionPtr keywords_compilers[] = {
	/* Those all are valid keyword-statements */
	[TOKEN_ALERT] = do_alert,
	[TOKEN_ASKFILE] = do_askfile,
	[TOKEN_BREAK] = do_break,
	[TOKEN_CALL] = do_call,
	[TOKEN_CASE] = do_case,
	[TOKEN_CLS] = do_cls,
	[TOKEN_CURSOR] = do_cursor,
	[TOKEN_CURSCHAR] = do_curschar,
	[TOKEN_CURSCOL] = do_curscol,
	[TOKEN_CURSPOS] = do_curspos,
	[TOKEN_DELETE] = do_delete,
	[TOKEN_END] = do_end,
	[TOKEN_FILES] = do_files,
	[TOKEN_GETKEY] = do_getkey,
	[TOKEN_GOSUB] = do_gosub,
	[TOKEN_GOTO] = do_goto,
	[TOKEN_INCLUDE] = do_include,
	[TOKEN_INK] = do_ink,
	[TOKEN_INPUT] = do_input,
	[TOKEN_LEN] = do_len,
	[TOKEN_LISTBOX] = do_listbox,
	[TOKEN_LOAD] = do_load,
	[TOKEN_MOVE] = do_move,
	[TOKEN_NUMBER] = do_number,
	[TOKEN_PAGE] = do_page,
	[TOKEN_PAUSE] = do_pause,
	[TOKEN_PEEK] = do_peek,
	[TOKEN_PEEKINT] = do_peekint,
	[TOKEN_POKE] = do_poke,
	[TOKEN_POKEINT] = do_pokeint,
	[TOKEN_PORT] = do_port,
	[TOKEN_PRINT] = do_print,
	[TOKEN_RAND] = do_rand,
	[TOKEN_READ] = do_read,
	[TOKEN_RENAME] = do_rename,
	[TOKEN_RETURN] = do_return,
	[TOKEN_SAVE] = do_save,
	[TOKEN_SERIAL] = do_serial,
	[TOKEN_SIZE] = do_size,
	[TOKEN_SOUND] = do_sound,
	[TOKEN_STRING] = do_string,
	[TOKEN_WAITKEY] = do_waitkey,

	/* While those are not (or are handled elsewhere) */
	[TOKEN_AND] = NULL,
	[TOKEN_CHR] = NULL,
	[TOKEN_DO] = NULL,
	[TOKEN_ELSE] = NULL,
	[TOKEN_ENDLESS] = NULL,
	[TOKEN_FOR] = NULL,
	[TOKEN_GET] = NULL,
	[TOKEN_HEX] = NULL,
	[TOKEN_IF] = NULL,
	[TOKEN_IN] = NULL,
	[TOKEN_LOOP] = NULL,
	[TOKEN_LOWER] = NULL,
	[TOKEN_NEXT] = NULL,
	[TOKEN_OFF] = NULL,
	[TOKEN_ON] = NULL,
	[TOKEN_OUT] = NULL,
	[TOKEN_PROGSTART] = NULL,
	[TOKEN_RAMSTART] = NULL,
	[TOKEN_REC] = NULL,
	[TOKEN_REM] = NULL,
	[TOKEN_SEND] = NULL,
	[TOKEN_SET] = NULL,
	[TOKEN_THEN] = NULL,
	[TOKEN_TIMER] = NULL,
	[TOKEN_TO] = NULL,
	[TOKEN_UNTIL] = NULL,
	[TOKEN_UPPER] = NULL,
	[TOKEN_VARIABLES] = NULL,
	[TOKEN_VERSION] = NULL,
	[TOKEN_WHILE] = NULL
};

Node* parse_keyword(SymbolTable* l)
{
	klabels = l;
	Token t = lookahead();

	FunctionPtr rule = keywords_compilers[t.type];
	if (rule != NULL)
		return rule();

	parse_error("Expected keyword");
	synchronize();
	return NULL;
}
