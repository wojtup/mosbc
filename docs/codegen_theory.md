# Table of contents

- [About code generation](#about-code-generation)
- [Binary layout of the compiled file](#binary-layout-of-the-compiled-file)
- [Statements](#statements)
- [Expressions](#expressions)
- [Labels](#labels)

---

## About code generation

The way machine-code is generated is very very simple: main generating function
is `compile()`, which is basically a wrapper around `compile_ast()`.

It is a recursive function, which upon encountering `NODE_ASSIGN` calls itself
on both children nodes. Else it looks up the rule in dispatch table and calls
it.

**Note:** What is very important, is the distinction between *runtime* and
*compile time*. When codegen function "returns" something, it doesn't return it
to the caller in actual compiler, but rather sets registers to some state.

## Binary layout of the compiled file

| Range of addresses   |  Contents of the range                    |
|:--------------------:|:-----------------------------------------:|
| `0x8000 - 0x8002`    |  JMP NEAR 0x???? (skip runtime & strings) |
| `0x8003 - 0x8016`    |  String addition handler                  |
| `0x8018 - 0x8047`    |  Division by zero handler                 |
| `0x8048 - 0x8097`    |  Printing string code                     |
| `0x8098 - 0x8099`    |  INK value                                |
| `0x809A - 0x809B`    |  RAMSTART value                           |
| `0x809C - 0x809D`    |  Working page                             |
| `0x809E - 0x809F`    |  Active page                              |
| `0x80A0 - 0x????`    |  String table                             |
| `0x???? - 0xFFFF`    |  Compiled binary and its RAM              |

---

## Statements

There are 2 types of statements: special forms and keyword statements. Former
are handled in main code generation file, [codegen.c](../src/back/codegen.c),
while latter are compiled in [keyword.c](../src/back/keyword.c).

Keyword statements are easy ones: you just look at the keyword and pick function
to call from a dispatch table. Called function does all the generation, usually
emitting calls to the API or BIOS.

Special forms are a little bit more tricky, because you need to patch jumps. It
isn't hard, just that we need to remember code addresses to later fix offsets
(because jumps in x86 are relative to current IP).

## Expressions

Expressions were difficult but here we have a quite elegant approach (because
nothing is more elegant than recursion everywhere) - function
`compile_expression()` will return true if all subexpressions are numeric and
false if they are not.

Based on keyword being compiled we can determine how we should interpret the
expression. For example, if we want to multiply 2 values, we would first call
this function on both of the children and compute the result.
*Note:* Here also type checking is done.

**The main rule is:** if expression is numeric, `compile_expression` puts the
result to `AX` register. If it is a string, it puts the address into `SI`.

## Labels

First read ["Parsing theory"](parsing_theory.md), chapter about labels to
understand it. Alright, now how it is compiled.

In every entry of symbol table, we have an `addr` field. During parsing it is
set to zero, and we don't bother with it. During code generation however, it is
really important.

Before compiling any statement, so at the beginning of `compile_ast()` we call
`patch_jumps()`. What it does, is it checks if current node is pointed to by
one of the labels. If it is, it sets `addr` to current place in memory, and
checks in the **patch table** if label was referred to in earlier `GOTO` or
`GOSUB` statements. If it was, then we go back and patch the jump to correct
location.
