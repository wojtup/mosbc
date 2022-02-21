# Table of contents

- [Directories](#directories)
- [General structure](#general-structure)
- [Used techniques](#used-techniques)
  - [Dispatch tables](#dispatch-tables)
  - [Data structures](#data-structures)

## Directories

Directories of this project are:
- `additional`: Here are some additional files (like test files or syntax
highlighting for Vim)
- `bin`: Here lies the compiled executable (**Note:** you need to run
`make init` first to have this directory).
- `docs`: You are here.
- `include`: All of the headers are in here.
- `obj`: Object files produced during compilation (**Note:** you need to run
`make init` first to have this directory).
- `src`: This is the main directory with source files.
  - `back`: Source of the back-end (code generator).
  - `front`: Source of the front-end (lexer and parser).
  - `util`: Source of various helpers.

## General structure

Execution starts obviously in [main.c](../src/main.c), which through a rather
primitive interface gets the source filename, and reads it. It then initializes
some compiler structures, and calls the function `parse()` from
[parser.c](../src/front/parser.c).

Parser repeatedly calls `get_token()` from [lexer.c](../src/lexer.c). It is a
boring function with nothing interesting in it. It outputs next token from the
source code, and changes some internal state to move to the next one. Function
`lookahead()` also grabs next token, but without advancing.

In short, `parse()` returns Abstract Syntax Tree (AST), which represents what
source code is doing in more machine-friendly way (so with lots of pointers,
numbers and no text). It is defined in [ast.h](../include/ast.h). For more
information about AST in general, see this pretty good explanation on the
[Wikipedia](https://en.wikipedia.org/wiki/Abstract_syntax_tree). How parser
works is described in greater detail in ["Parsing Theory"](parsing_theory.md).

Parsed code is passed then to the `compile()` function. It writes out bytes of
machine code to [dynamic array](#data-structures), which, after successful code
generation, is returned to `main()`. There, it is written to output file, and
compilation terminates.

---

## Used techniques

Here are described basic data structures and "algorithms" used (yes I know
dispatch tables aren't "algorithms" but it sounds better).

### Dispatch tables

Dispatch tables are really cool (and sometimes efficient) so they are used
*a lot* in this project. If you don't know what they are, it is basically a
table of pointers to the functions. Now you can just use your value as an index
in the table, get the pointer and call it like a normal function.

### Data structures

As any large enough program, this compiler needs to use some data structures.
It isn't rocket science, but we use those two:
- **Dynamic array**
- **Binary tree** (in form of AST)

Dynamic arrays are like normal contiguous arrays, but have added parameter:
their capacity. So if you want to add new element, inserting function checks if
there is enough space, and if not it extends array by a factor of 2. This way
we can reduce number of needed reallocations (that's good). If you want to know
more about dynamic arrays, read yet another article on Wikipedia, right
[here](https://en.wikipedia.org/wiki/Dynamic_array).

Binary trees are basically a set of nodes, where all nodes hold references to
two of their *children*. If the reference is `NULL` it means there is no child.
Because every node specifies all of its descendants, we can pass the whole tree
using only the top-most node (called root). Don't ask me why root is on top.
And, the Wikipedia [article](https://en.wikipedia.org/wiki/Binary_tree).
