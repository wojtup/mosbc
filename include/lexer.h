/*
 * Copyright (C) 2022, Wojciech Grzela <grzela.wojciech@gmail.com>
 * Licensed under GNU General Public License version 3.
 */

#ifndef LEXER_H
#define LEXER_H

typedef enum {
	/* Keywords */
	TOKEN_ALERT = 0, TOKEN_AND, TOKEN_ASKFILE, TOKEN_BREAK, TOKEN_CALL,
	TOKEN_CASE, TOKEN_CHR, TOKEN_CLS, TOKEN_CURSOR, TOKEN_CURSCHAR,
	TOKEN_CURSCOL, TOKEN_CURSPOS, TOKEN_DELETE, TOKEN_DO, TOKEN_ELSE,
	TOKEN_END, TOKEN_ENDLESS, TOKEN_FILES, TOKEN_FOR, TOKEN_GET,
	TOKEN_GETKEY, TOKEN_GOSUB, TOKEN_GOTO, TOKEN_HEX, TOKEN_IF, TOKEN_IN,
	TOKEN_INCLUDE, TOKEN_INK, TOKEN_INPUT, TOKEN_LEN, TOKEN_LISTBOX,
	TOKEN_LOAD, TOKEN_LOOP, TOKEN_LOWER, TOKEN_MOVE, TOKEN_NEXT,
	TOKEN_NUMBER, TOKEN_OFF, TOKEN_ON, TOKEN_OUT, TOKEN_PAGE, TOKEN_PAUSE,
	TOKEN_PEEK, TOKEN_PEEKINT, TOKEN_POKE, TOKEN_POKEINT, TOKEN_PORT,
	TOKEN_PRINT, TOKEN_PROGSTART, TOKEN_RAMSTART, TOKEN_RAND, TOKEN_READ,
	TOKEN_REC, TOKEN_REM, TOKEN_RENAME, TOKEN_RETURN, TOKEN_SAVE,
	TOKEN_SEND, TOKEN_SERIAL, TOKEN_SET, TOKEN_SIZE, TOKEN_SOUND,
	TOKEN_STRING, TOKEN_THEN, TOKEN_TIMER, TOKEN_TO, TOKEN_UNTIL,
	TOKEN_UPPER, TOKEN_VARIABLES, TOKEN_VERSION, TOKEN_WAITKEY, TOKEN_WHILE,

	/* Operators */
	TOKEN_PLUS = 72, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_PERCENT,
	TOKEN_EQUALS, TOKEN_NOT_EQUALS, TOKEN_GREATER, TOKEN_SMALLER,
	TOKEN_AMPERSAND, TOKEN_SEMICOLON,

	/* Literals */
	TOKEN_NUMERIC_VARIABLE = 83, TOKEN_STRING_VARIABLE,
	TOKEN_NUMERIC_LITERAL, TOKEN_STRING_LITERAL, TOKEN_CHARACTER_LITERAL,
	TOKEN_LABEL, TOKEN_IDENTIFIER,

	/* Synthetic tokens */
	TOKEN_ERROR = 90, TOKEN_EOF
} TokenType;

typedef struct {
	const char* source;	/* Beginning of whole source code */
	const char* beginning;	/* Beginning of current token */
	const char* current;	/* Current position in source */
	int line;		/* Current line */
} Lexer;

typedef struct {
	TokenType type;		/* Type of the token */
	const char* text;	/* Pointer to the beginning of lexeme */
	int length;		/* Length of the lexeme */
	int line;		/* Line on which it lies */
} Token;

void init_lexer(const char* source);
Token get_token();
Token lookahead();

#endif
