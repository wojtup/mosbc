# Notes on nodes (pun originally not intended)

Here are rules for construction and interpretation of nodes.
This is not meant to be very specific, just to give the idea for what goes
where.

## Special forms nodes

```
SEQUENCE -> (op1 = First in sequence; op2 = Second in sequence)
ASSIGN -> (op1 = Target; op2 = Value)
EXPR -> (op1 = First value; op2 = Second value)
VARIABLE -> (op1 = NULL; op2 = NULL)
LITERAL -> (op1 = NULL; op2 = NULL)
LABEL -> (op1 = NULL; op2 = NULL)
IF -> (op1 = Condition; op2 = SEQUENCE(Then branch; Else branch))
DO -> (op1 = Condition; op2 = SEQUENCE(Body; Modifier))
FOR -> (op1 = Initializer; op2 = SEQUENCE(To value; Body))
```

## Keyword nodes

```
ALERT -> (op1 = Target; op2 = NULL)
ASKFILE -> (op1 = Target; op2 = NULL)
BREAK -> (op1 = NULL; op2 = NULL)
CALL -> (op1 = Target; op2 = NULL)
CASE -> (op1 = Modifier; op2 = Target)
CLS -> (op1 = NULL; op2 = NULL)
CURSOR -> (op1 = Modifier; op2 = NULL)
CURSCHAR -> (op1 = Target; op2 = NULL)
CURSCOL -> (op1 = Target; op2 = NULL)
CURSPOS -> (op1 = First target; op2 = Second target)
DELETE -> (op1 = Name; op2 = NULL)
END -> (op1 = NULL; op2 = NULL)
FILES -> (op1 = NULL; op2 = NULL)
GETKEY -> (op1 = Target; op2 = NULL)
GOSUB -> (op1 = Target; op2 = NULL)
GOTO -> (op1 = Target; op2 = NULL)
INCLUDE -> N/A
INK -> (op1 = Value; op2 = NULL)
INPUT -> (op1 = Target; op2 = NULL)
LEN -> (op1 = String value; op2 = Target)
LISTBOX -> (op1 = First string; op2 = SEQUENCE(Second string; SEQUENCE(Third string; Target)))
LOAD -> (op1 = String value; op2 = Value)
MOVE -> (op1 = First value; op2 = Second value)
NUMBER -> (op1 = First value; op2 = Second value)
PAGE -> (op1 = First value; op2 = Second value)
PAUSE -> (op1 = Value; op2 = NULL)
PEEK -> (op1 = Target; op2 = Value)
PEEKINT -> (op1 = Target; op2 = Value)
POKE -> (op1 = First value; op2 = Second value)
POKEINT -> (op1 = First value; op2 = Second value)
PORT -> (op1 = Modifier; op2 = SEQUENCE(First value; Second value))
PRINT -> (op1 = First modifier; op2 = SEQUENCE(Value; Second modifier))
RAND -> (op1 = Target; op2 = SEQUENCE(First value; Second value))
READ -> N/A
RENAME -> (op1 = First string; op2 = Second string)
RETURN -> (op1 = NULL; op2 = NULL)
SAVE -> (op1 = String; op2 = SEQUENCE(First value; Second value))
SERIAL -> (op1 = Modifier; op2 = Value)
SIZE -> (op1 = String; op2 = NULL)
SOUND -> (op1 = First value; op2 = Second value)
STRING -> (op1 = Modifier; op2 = SEQUENCE(String target; SEQUENCE(Value; Numeric target)))
WAITKEY -> (op1 = Target; op2 = NULL)
```
