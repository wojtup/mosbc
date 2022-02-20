rem TODO LIST
rem [ok] ALERT
rem [ok] ASKFILE
rem [ok] BREAK AND CLS
rem [ok] POKE AND CALL
rem [ok] CASE
rem [ok] CURSOR, PAUSE, CURSPOS, CURSCHAR AND CURSCOL
rem [ok] DELETE
rem [ok] FILES
rem [ok] GETKEY
rem [ok] GOSUB, GOTO, RETURN
rem [ok] INK
rem [ok] INPUT
rem [ok] LEN
rem [ok] LISTBOX
rem [ok] LOAD
rem [ok] MOVE
rem [ok] NUMBER
rem [ok] PAGE
rem [ok] PEEK/INT, POKE,INT
rem [ok] PRINT
rem [ok] RAND
rem [ok] RENAME
rem [ok] SAVE
rem [??] SIZE
rem [??] STRING
rem [??] WAITKEY

$1 = "ok"
x = PROGSTART

rem ALERT TEST
alert "start"
alert $1

rem ASKFILE TEST
askfile $1
cls
print $1

rem POKE & CALL TEST
poke 195 40000
call 40000

rem CASE TEST
case upper $1
print "upper: " + $1
case lower $1
print "lower: " + $1

rem CURSOR & PAUSE TEST
cursor off
pause 40
cursor on

rem CURSCHAR TEST
cls
print "Hello world"
move 0 0
curschar x
move 0 1
print chr x

rem CURSCOL TEST
move 0 0
curscol x
move 0 2
print hex x

rem CURSPOS TEST
curspos x y
print hex x ;
print " " ;
print hex y

rem DELETE TEST
print "DELETE TEST: ";
delete "abababab.txt"
print r

rem FILES TEST
print "FILES TEST: ";
files

rem GETKEY TEST
print "GETKEY TEST: ";
getkey x
print hex x

rem GOSUB & GOTO TEST
print "GOSUB & GOTO TEST: ";
gosub func
goto cont
print "hi"
cont:
print "hi continued"

rem INK TEST
print "INK TEST: ";
ink 8
print "test"
ink 7
print "test"

rem INPUT TEST
print "INPUT TEST: ";
input x
print hex x

rem LEN TEST
print "LEN TEST: ";
$1 = "this string is 33 characters long"
len $1 x
print x

rem LISTBOX TEST
print "LISTBOX TEST: ";
listbox "first,second,third,fourth" "b" "c" x
cls
print x

rem LOAD TEST
print "LOAD TEST: ";
load "example.bas" RAMSTART
peek x RAMSTART
print chr x

rem MOVE TEST
print "MOVE TEST: "
curspos x y
move 0 0
pause 20
move x y

rem NUMBER TEST
print "NUMBER TEST: ";
$1 = "123"
number $1 x
print x ;
print " ";
x = 456
number x $1
print $1

rem PAGE TEST
print "PAGE TEST, change in 6 seconds"  rem This is on page 0, visible
pause 30
page 1 0
print "Hello world, another 3 seconds"  rem This is written to 1, invisible
pause 30
page 0 1
print "Back to original"  rem This is written to 0, while it is invisible
pause 30
page 0 0

rem POKE/INT TEST
print "POKE 40000: ";
poke 127 40001
poke 247 40000
print "ok"

rem PEEK/INT TEST
print "PEEK 40000: ";
peek a 40000
print hex a
print "PEEK 40001: ";
peek a 40001
print hex a
print "PEEKINT 40000: ";
peekint a 40000
print hex a

rem PRINT TEST
rem (Unnecessary)

rem RAND TEST
rand x 100 1000
print x

rem SAVE TEST
print "DISK TESTS: ";
pokeint 26984 40000
pokeint 30752 40002
save "test.txt" 40000 4

rem RENAME TEST
rename "test.txt" "hello.txt"

rem SIZE TEST
size "test.txt"
print s

rem STRING TEST
print "STRING TEST: ";
$1 = "Hello"
string get $1 3 b
print hex b ;
b = 121
string set $1 5 b
print " " + $1

rem WAITKEY TEST
print "WAITKEY TEST: ";
waitkey x
print x

end

func:
  print "gosub is here!"
return
