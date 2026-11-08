; Test Task 1: Comments and Task 7: Moving constants into any register
mov reg1, 10
mov reg2, 20 ; expands to "mov reg1, 20" and "mov reg2, reg1"

; Test Task 2: not instruction
not reg3, reg1

; Test Task 4: ALU instructions
or reg4, reg1, reg2
add reg5, reg1, reg2
sub reg6, reg1, reg2
nand reg3, reg1, reg2
and reg4, reg1, reg2
xor reg5, reg1, reg2
eq reg6, reg1, reg2

; Test Task 5 & 6: Writing to and reading from RAM
mov [reg2], reg1
mov reg1, [reg2]