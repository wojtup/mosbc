# Differences between this implementation and original one

All things listed are what this version does and original doesn't. List may be
incomplete:
- Accepts tabs as whitespace.
- Doesn't have limit on source length.
- Tokens can have no whitespace between them (so `2+2` is okay).
- String variables are validated at compile-time. In original version due to an
obscure bug in token detection, things like `$J` or `$~` pass unnoticed. It
isn't mentioned anywhere in documentation, so I assume it is a bug.
- Labels can't be keywords (so no `GOTO LOOP`).
- Address operator (`&`) can be used on both string and numeric variables.
- If variables specified by `FOR` initializer and `NEXT` don't match, error is
raised.
- In almost every built-in function possible sources were expanded. Instead of
using for example only numeric literals, now using variables is also possible.
Note: this doesn't apply to *target* variables, so writing is only allowed to
variables.
- `NEXT` cannot be put inside `FOR` loop, only at the end. It is a bit of a
shame, because in original version it is like `continue` in C.
- `FOR` loop will stop after iterator goes over `TO` field, even if it is not
exactly met (so if loop skips 2 every iteration, it will halt even if `TO` is
missed).
- Numbers cannot be added to strings. It is not that bad actually, as you can
always convert both ways with `NUMBER`.
- `FILES` doesn't print filenames like `DIR` command but more like `LS`, with
every file on different line.
- `READ` doesn't work, and probably won't ever. Sorry, it breaks some key
assumptions the compiler uses.
