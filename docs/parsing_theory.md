# Table of contents

- [About this parser](#about-this-parser)
- [Main parsing function](#main-parsing-function)
- [Statements](#statements)
- [Assignments](#assignments)
- [IF statements](#if-statements)
- [DO loops](#do-loops)
- [FOR loops](#for-loops)
- [Keyword statements](#keyword-statements)
- [Labels](#labels)

---

## About this parser

It is a [recursive descent parser](https://en.wikipedia.org/wiki/Recursive_descent_parser),
strictly following specified grammar.

Every function corresponds to one rule (although helpers are often put in one
function). They take no arguments (besides `parse()`, see below) and return
`Node*`. This return value is AST for parsed rule.

In case of an error, each and every function has the moral duty to clean up
after itself (so free every node initialized) and return `NULL`. No successful
parsing will result in the return of `NULL` pointer.

And one last thing about grammar itself: if you look very closely at the parsing
code, you will see that it is not, in fact, context free. This is due to the way
FOR loops are parsed. For more info read [here](#for-loops).

---

## Main parsing function

Parsing obviously starts by calling `parse()` with 2 arguments: *symbol table*
and *string table*. Those will be filled with entries as the parser goes through
the code. Both arguments are stored in global variables, and are not moved (it
is pointers don't change, only values under those pointers).

The way `parse()` works is pretty simple: while it has anything to parse (isn't
at the EOF) it calls `statement()` and appends it to its tree of nodes, linked
via `NODE_SEQUENCE`.

## Statements

Real parsing starts in `statement()` because, in this BASIC, **program is just**
**a list of statements**. There are only a few types of statements, namely:
- Assignment [(more info)](#assignments)
- IF statement [(more info)](#if-statements)
- DO loops [(more info)](#do-loops)
- FOR loops [(more info)](#for-loops)
- Keyword statement [(more info)](#keyword-statements)
- Label statement [(more info)](#labels)

Now what happens, is that looking only at the first token of the entire
statement we can tell which type it is, so we only need one token of lookahead.
So `statement()`'s only job is to call correct function by looking at next
token, and sometimes throw out an error if it doesn't match any of these.

---

## Assignments

Assignments are handled by an intuitively named function: `assign()`. It works
like this:
1. Consume a variable. This is assignment's *target*.
2. Consume `TOKEN_EQUALS`.
3. Parse following expression. It becomes the *value*.

It is really that simple, although as you may have noted, we don't check
validity of assignment, that is, typing. We will do it during the compilation.

**Important**: Assignments are statements, not expressions, so you can't chain
them like in C (for example `a = b = c;`). It is consistent with MikeOS'
implementation, yet it is still worth a note.

## IF statements

IF statements are parsed in `if_stmt()` function. Its rough description is:
1. Consume `TOKEN_IF`.
2. Parse boolean expression. This will be our *condition*.
3. Consume `TOKEN_THEN`.
4. Parse **exactly 1 (one)** statement. It is a *then branch*.
5. Check if there is `TOKEN_ELSE`. If there is, proceed. Else parsing is done.
6. Consume said token.
7. Parse **exactly 1 (one)** statement. Now it becomes an *else branch*.

Only thing that should be noted is how each branch is only **one** statement.
There is no C-like construct like block in here, and given that IF is not
followed by some sort of "FI" keyword, there is no way of knowing where branch
ends.

## DO loops

Parsed by `do_stmt()`; this is an algorithm for it:
1. Consume `TOKEN_DO`.
2. Until you hit `TOKEN_EOF` (meaning an error) or `TOKEN_LOOP` (meaning the
loop's body is finished), parse statements, building tree of them. This tree is
the *body*.
3. Consume `TOKEN_LOOP`.
4. Consume loop's *modifier*.

Code for this really straightforward, but you have to be careful to stop
parsing when you hit `TOKEN_EOF`, or else an invalid syntax can cause an
infinite loop in your parser.

## FOR loops

Parsed by `for_stmt()`. As always, an algorithm:
1. Consume `TOKEN_FOR`.
2. Call `assign()`. Returned node is loop's *initializer*.
3. Consume `TOKEN_TO`.
4. Parse expression. This is *to field*.
5. Until you hit `TOKEN_EOF` (meaning an error) or `TOKEN_NEXT` (meaning the
loop's body is finished), parse statements, building tree of them. This tree is
the *body*.
6. Consume `TOKEN_NEXT`.
7. Consume a variable. Compare it to initializer's target, and if they don't
match, raise an error.

**Important**: We compare the target of initializer and variable after `NEXT`
*at parse time* rather than during compiling. There is no particular difference
in where it happens, as it will get reported anyways. Also, for this check to
happen we need to ensure that initializer is not `NULL`, or else parser will
seg-fault on it.
**Additional note**: Due to this check, parser becomes not context-free (or at
least I think).

## Keyword statements

Keywords are handled in [separate file](../src/front/keyword_parser.c), for
clarity of code. Main function there is, unsurprisingly, `parse_keyword()`,
which looks up next token in a dispatch table, handing parser to found function.
All of those `NULL` entries correspond to tokens that should have been handled
elsewhere, like `TOKEN_IF` or `TOKEN_DO`. If we meet them then something went
very wrong and we report an error.

How all of those functions work individually is too much to describe here, but
none of them is really anything more than simply direct implementation of its
grammar rule.

## Labels

How labels work is actually very educative, and it is because of quite peculiar
thing about MikeOS' BASIC: you can call *forward* in code.

It is even more interesting given that it's a feature not present in most of
older languages (like C or Pascal), so it was to be expected for BASIC to not
support it, but well, it does.

So now, how it is implemented: each label is kept in **symbol table**, and has
a flag attached to it: **the "real" flag**. As we parse, we can add *real* and
*unreal* entries to the table. The distinction is:
- Real entries represent labels **actually found** in the code.
- Unreal entries represent labels that are **only targets** for jumps.

Now, this table has 2 functions:
- `add_unreal_symbol()` adds the symbol, but doesn't set its real flag. If the
entry is already present, then it does nothing.
- `add_real_symbol()` adds the symbol, and sets its real flag. If the entry is
already present **and is unreal**, then set its real flag. If it is present and
real, does nothing.

So now after parsing whole file, during compiling, when we hit `GOTO` or
`GOSUB`, we check if its target is real or not. If it is not, then it means we
found reference to undefined label, and call for an error. Effectively what we
do is **delay checking label's presence**.
