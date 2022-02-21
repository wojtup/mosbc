" Syntax is case insensitive
syntax case ignore

" Keywords
syn keyword basicStmtKeywords ALERT AND ASKFILE BREAK CALL
syn keyword basicStmtKeywords CASE CHR CLS CURSOR CURSCHAR
syn keyword basicStmtKeywords CURSCOL CURSPOS DELETE DO ELSE
syn keyword basicStmtKeywords END ENDLESS FILES FOR GET GOSUB
syn keyword basicStmtKeywords GOTO GETKEY HEX IF IN INCLUDE
syn keyword basicStmtKeywords INK INPUT LEN LISTBOX LOAD
syn keyword basicStmtKeywords LOOP LOWER MOVE NEXT NUMBER
syn keyword basicStmtKeywords OFF ON OUT PAGE PAUSE PEEK
syn keyword basicStmtKeywords PEEKINT POKE POKEINT PORT PRINT
syn keyword basicStmtKeywords RAND READ REC RENAME RETURN SAVE
syn keyword basicStmtKeywords SEND SERIAL SET SIZE SOUND STRING
syn keyword basicStmtKeywords THEN TO UNTIL UPPER WAITKEY WHILE

" Expression keywords
syn keyword basicExprKeywords INK PROGSTART RAMSTART TIMER
syn keyword basicExprKeywords VARIABLES VERSION

" Variables and literals (order matters!)
syn match basicNumericVar "[a-zA-Z]"
syn match basicCharLiteral "'[a-zA-Z]'"
syn match basicStringVar "$[1-8]"
syn match basicNumericLiteral '\d\+'
syn region basicStringLiteral start='"' end='"'

" Operators
syn match basicOperator '+*-/%'

" Comments and TODO
syn keyword basicTodo contained TODO NOTE
syn region basicComment start="[rR][eE][mM]" end="$" contains=basicTodo

" Link all of the to specific classes
highlight link basicTodo Todo
highlight link basicComment Comment
highlight link basicStmtKeywords Keyword
highlight link basicExprKeywords Constant
highlight link basicStringLiteral String
highlight link basicCharLiteral Character
highlight link basicNumericVar Identifier
highlight link basicStringVar Identifier
highlight link basicNumericLiteral Number
highlight link basicOperator Operator

let b:current_syntax = "mosbasic"
