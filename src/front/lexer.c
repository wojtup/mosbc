/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

/* Standard library includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

/* Custom includes */
#include <lexer.h>
#include <util.h>

/* List of all keywords */
const char* keywords_names[] = {
	[TOKEN_ALERT] = "ALERT",
	[TOKEN_AND] = "AND",
	[TOKEN_ASKFILE] = "ASKFILE",
	[TOKEN_BREAK] = "BREAK",
	[TOKEN_CALL] = "CALL",
	[TOKEN_CASE] = "CASE",
	[TOKEN_CHR] = "CHR",
	[TOKEN_CLS] = "CLS",
	[TOKEN_CURSOR] = "CURSOR",
	[TOKEN_CURSCHAR] = "CURSCHAR",
	[TOKEN_CURSCOL] = "CURSCOL",
	[TOKEN_CURSPOS] = "CURSPOS",
	[TOKEN_DELETE] = "DELETE",
	[TOKEN_DO] = "DO",
	[TOKEN_ELSE] = "ELSE",
	[TOKEN_END] = "END",
	[TOKEN_ENDLESS] = "ENDLESS",
	[TOKEN_FILES] = "FILES",
	[TOKEN_FOR] = "FOR",
	[TOKEN_GET] = "GET",
	[TOKEN_GOSUB] = "GOSUB",
	[TOKEN_GOTO] = "GOTO",
	[TOKEN_GETKEY] = "GETKEY",
	[TOKEN_HEX] = "HEX",
	[TOKEN_IF] = "IF",
	[TOKEN_IN] = "IN",
	[TOKEN_INCLUDE] = "INCLUDE",
	[TOKEN_INK] = "INK",
	[TOKEN_INPUT] = "INPUT",
	[TOKEN_LEN] = "LEN",
	[TOKEN_LISTBOX] = "LISTBOX",
	[TOKEN_LOAD] = "LOAD",
	[TOKEN_LOOP] = "LOOP",
	[TOKEN_LOWER] = "LOWER",
	[TOKEN_MOVE] = "MOVE",
	[TOKEN_NEXT] = "NEXT",
	[TOKEN_NUMBER] = "NUMBER",
	[TOKEN_OFF] = "OFF",
	[TOKEN_ON] = "ON",
	[TOKEN_OUT] = "OUT",
	[TOKEN_PAGE] = "PAGE",
	[TOKEN_PAUSE] = "PAUSE",
	[TOKEN_PEEK] = "PEEK",
	[TOKEN_PEEKINT] = "PEEKINT",
	[TOKEN_POKE] = "POKE",
	[TOKEN_POKEINT] = "POKEINT",
	[TOKEN_PORT] = "PORT",
	[TOKEN_PRINT] = "PRINT",
	[TOKEN_PROGSTART] = "PROGSTART",
	[TOKEN_RAMSTART] = "RAMSTART",
	[TOKEN_RAND] = "RAND",
	[TOKEN_READ] = "READ",
	[TOKEN_REC] = "REC",
	[TOKEN_REM] = "REM",
	[TOKEN_RENAME] = "RENAME",
	[TOKEN_RETURN] = "RETURN",
	[TOKEN_SAVE] = "SAVE",
	[TOKEN_SEND] = "SEND",
	[TOKEN_SERIAL] = "SERIAL",
	[TOKEN_SET] = "SET",
	[TOKEN_SIZE] = "SIZE",
	[TOKEN_SOUND] = "SOUND",
	[TOKEN_STRING] = "STRING",
	[TOKEN_THEN] = "THEN",
	[TOKEN_TIMER] = "TIMER",
	[TOKEN_TO] = "TO",
	[TOKEN_UNTIL] = "UNTIL",
	[TOKEN_UPPER] = "UPPER",
	[TOKEN_VARIABLES] = "VARIABLES",
	[TOKEN_VERSION] = "VERSION",
	[TOKEN_WAITKEY] = "WAITKEY",
	[TOKEN_WHILE] = "WHILE"
};

Lexer lexer;
bool wait;
Token pending;

/* =========================== INTERNAL FUNCTIONS =========================== */
void include(const char* new_fname)
{
	/* Read source code of included file */
	char* src = read_file(new_fname);

	/* Calculate new length and allocate */
	int len = strlen(lexer.source) + strlen(src) + 2;
	char* new_source = malloc(len);

	/* Copy  */
	strcpy(new_source, lexer.source);
	strcat(new_source, "\n");
	strcat(new_source, src);

	/* Free old source and set new */
	free((char*) lexer.source);
	lexer.source = new_source;
}

/* Consider NUL and operators as whitespace */
bool iswhite(char c)
{
	if (isspace(c))
		return true;
	else if (c == '\0')
		return true;
	else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' ||
		c == '\'' || c == '"' || c == '!' || c == '>' || c == '<' ||
		c == '%' || c == '&' || c == ';')
		return true;

	return false;
}

void lex_error(const char* str)
{
	printf("\x1B[31mError (lex)\x1B[0m: %s at line: %d.\n", str,
		lexer.line);
	raise_error();
}

Token init_token(TokenType t)
{
	Token ret;
	ret.type = t;
	ret.text = lexer.beginning;
	ret.length = (int) (lexer.current - lexer.beginning);
	ret.line = lexer.line;

	return ret;
}

TokenType match_keyword(const char* str)
{
	size_t len = lexer.current - lexer.beginning;
	char* upper_str = malloc(len);
	strncpy(upper_str, str, len);
	string_uppercase(upper_str);

	TokenType ret = TOKEN_IDENTIFIER;
	for (int i = TOKEN_ALERT; i <= TOKEN_WHILE; i++) {
		/* If lengths are different then surely not */
		if (strlen(keywords_names[i]) != len)
			continue;

		/* Now compare */
		if (!strncmp(upper_str, keywords_names[i], len))
			ret = i;
	}

	free(upper_str);
	return ret;
}

/* ============================ EXPOSED FUNCTIONS =========================== */
void init_lexer(const char* source)
{
	lexer.source = source;
	lexer.beginning = source;
	lexer.current = source;
	lexer.line = 1;

	/* First pass is for INCLUDE */
	while (lookahead().type != TOKEN_EOF) {
		Token t = get_token();
		if (t.type == TOKEN_INCLUDE) {
			t = get_token();
			if (t.type == TOKEN_STRING_LITERAL) {
				char* str = malloc(t.length + 1);
				strncpy(str, t.text, t.length);
				str[t.length] = '\0';

				include(str);
			}
		}
	}

	/* Reset lexer */
	lexer.beginning = lexer.source;
	lexer.current = lexer.source;
	lexer.line = 1;
	wait = false;
}

Token get_token()
{
	if (wait) {
		wait = false;
		return pending;
	}

	Token ret;

	next:
	lexer.beginning = lexer.current;
	switch (*lexer.current++) {
		/* If we meet NULL - we ended lexing */
		case '\0':
			lexer.current--;	/* Don't move forward! */
			ret = init_token(TOKEN_EOF);
			break;

		/* Handle whitespace */
		case '\n':
			lexer.line++;
		case '\r':
		case '\t':
		case ' ':
			goto next;

		/* Handle basic, one character operators - this is easy */
		case '+':
			ret = init_token(TOKEN_PLUS);
			break;
		case '-':
			ret = init_token(TOKEN_MINUS);
			break;
		case '*':
			ret = init_token(TOKEN_STAR);
			break;
		case '/':
			ret = init_token(TOKEN_SLASH);
			break;
		case '%':
			ret = init_token(TOKEN_PERCENT);
			break;
		case '=':
			ret = init_token(TOKEN_EQUALS);
			break;
		case '>':
			ret = init_token(TOKEN_GREATER);
			break;
		case '<':
			ret = init_token(TOKEN_SMALLER);
			break;
		case '&':
			ret = init_token(TOKEN_AMPERSAND);
			break;
		case ';':
			ret = init_token(TOKEN_SEMICOLON);
			break;

		case '!':
			if (*lexer.current == '=') {
				lexer.current++;	/* Skip = */
				ret = init_token(TOKEN_NOT_EQUALS);
				break;
			}
			lex_error("Bang not followed by equal sign");
			ret = init_token(TOKEN_ERROR);
			break;

		/* String literals handling */
		case '"':
			lexer.beginning++;	/* Skip first " */
			while (*lexer.current++ != '"')
				if (*lexer.current == '\n') {
					lex_error("Newline in string constant");
					ret = init_token(TOKEN_ERROR);
					return ret;
				}
			lexer.current--;	/* We don't want " in string */
			ret = init_token(TOKEN_STRING_LITERAL);
			lexer.current++;	/* Skip ending " */
			break;
		case '\'':
			/* Skip first ' */
			lexer.beginning++;
			if (*++lexer.current != '\'') {
				lex_error("Character constant too long");
				ret = init_token(TOKEN_ERROR);
				return ret;
			}
			ret = init_token(TOKEN_CHARACTER_LITERAL);
			lexer.current++;	/* Skip ending ' */
			break;

		/* String variables */
		case '$': {
			char c = *lexer.current;
			if (c < '1' || c > '8') {
				lex_error("Invalid string variable");
				lexer.current++;
				ret = init_token(TOKEN_ERROR);
				return ret;
			}
			lexer.current++;	/* Take in number */
			ret = init_token(TOKEN_STRING_VARIABLE);
			break;
		}

		default:
			/* Is it a number? */
			if (isdigit((unsigned char) *(lexer.current - 1))) {
				while (isdigit((unsigned char) *lexer.current))
					lexer.current++;

				ret = init_token(TOKEN_NUMERIC_LITERAL);
			}
			/* Is it a numeric variable? */
			else if (iswhite(*lexer.current)) {
				if (!isalpha((unsigned char) *(lexer.current - 1))) {
					lex_error("Invalid numeric variable");
					ret = init_token(TOKEN_ERROR);
					return ret;
				}
				ret = init_token(TOKEN_NUMERIC_VARIABLE);
			}
			/* No! It is a random string! */
			else {
				while (!iswhite(*lexer.current))
					lexer.current++;

				TokenType t = match_keyword(lexer.beginning);

				/* Maybe it was a label? */
				if (*(lexer.current - 1) == ':') {
					lexer.current--;	/* Ignore : */
					ret = init_token(TOKEN_LABEL);
					lexer.current++;	/* Skip it */
				}

				/* Or quite possibly comment */
				else if (t == TOKEN_REM) {
					while (*lexer.current != '\n')
						lexer.current++;
					goto next;
				}
				else
					ret = init_token(t);
			}
	}

	return ret;
}

Token lookahead()
{
	pending = get_token();
	wait = true;

	return pending;
}
