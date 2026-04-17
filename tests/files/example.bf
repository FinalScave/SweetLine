This is a Brainfuck sample program
Any characters that are not the 8 commands are treated as comments
The 8 commands are: > < + - . , [ ]

Hello World Program
++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>
.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.

Simple loop example
+++++ +++++ [>+++++ ++<-] > +++.
This prints the character 'A' (ASCII 65)

Add two numbers
,>++++++[<-------->-]<,>++++++[<-------->-]<[<+>-]<.
This reads two single-digit numbers and prints their sum

Multiply two numbers
,>++++++[<-------->-]<,>++++++[<-------->-]<[>[>+>+<<-]>>[<<+>>-]<<<-]>>.
This reads two single-digit numbers and prints their product

Clear a cell
[-]
This sets the current cell to zero

Copy a value
[->+>+<<]>>[-<<+>>]
This copies the value from the current cell to the next two cells

Conditional statement
,----------[----------------------.,----------]
This reads characters until a newline

Print numbers 0-9
++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.

Fibonacci sequence (prints first few numbers)
+++++++++++>+>>>>++++++++++++++++++++++++++++++++++++++++++++>++++++++++++++++++++++++++++++++<<<<<<[>[>>>>>>+>+<<<<<<<-]>>>>>>>[<<<<<<<+>>>>>>>-]<[>++++++++++[-<-[>>+>+<<<-]>>>[<<<+>>>-]+<[>[-]<[-]]>[<<[>>>+<<<-]>>[-]]<<]>>>[>>+>+<<<-]>>>[<<<+>>>-]+<[>[-]<[-]]>[<<+>>[-]]<<<<<<<]>>>>[-]<<<<[>>>>>>>>>+<<<<<<<<<-]>>>>>>>>>[<<<<<<<<<+>>>>>>>>>-]<<<<<<<<.>>>>>>[-]<<<<<<[>>>>>>>>+<<<<<<<<-]>>>>>>>>[<<<<<<<<+>>>>>>>>-]<<<<<<<<[-<<<<<<<.>>>>>>>]<<<<<<<]

Factorial of 5
+++++>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>++[<+>]

Nested loops example
+++[>+++[>+++<-]<-]
This multiplies 3*3*3 = 27

Print alphabet
++++++++[>++++++++<-]>+.>++++++++[<++++++++>-]<+.+++++++..+++.>++++[<+++++++>-]<++.------------.>+++++++[<++++++++>-]<+.--------.+++.------.--------.>++++++++[<++++++++>-]<+.

Memory manipulation
>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+
Move right and increment 30 cells

>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+>+
Move right and increment 30 more cells

<<[<]<<[<]<<[<]<<[<]<<[<]<<[<]<<[<]<<[<]<<[<]<<[<]<<[<]<<[<
Move back to beginning

Simple arithmetic
+++ +++ +++ +++ +++ +++ +++ +++ +++ +++ +++ +++
Add 36 to current cell

[-]
Clear the cell

[>+<-]
Move value to next cell

[>>+<<-]
Move value two cells to the right

[->+<]
Add current cell to next cell

[->[->+<]<]
Add all cells to the right

End of program
