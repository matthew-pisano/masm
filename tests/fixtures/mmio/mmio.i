.data

buffer:
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00


.text

main:
lui $at, 4097
ori $a0, $at, 0
addiu $a1, $zero, 4
jal getNChar
lui $at, 4097
ori $a0, $at, 0
addiu $a1, $zero, 4
jal putNChar
addiu $v0, $zero, 10
syscall

getNChar:
lui $at, 65535
ori $t0, $at, 4
lui $at, 65535
ori $t1, $at, 0
addiu $t2, $zero, 0

getLoop:
lb $t3, $t1, 3
andi $t3, $t3, 1
beq $t3, $0, getLoop
lb $t4, $t0, 3
add $t5, $a0, $t2
sb $t4, $t5, 0
addi $t2, $t2, 1
slt $t5, $t2, $a1
bne $t5, $zero, getLoop
jr $ra

putNChar:
lui $at, 65535
ori $t1, $at, 8
lui $at, 65535
ori $t5, $at, 12
addiu $t2, $zero, 0

putLoop:
lb $t3, $t1, 3
andi $t3, $t3, 1
beq $t3, $0, putLoop
add $t4, $a0, $t2
lbu $t6, $t4, 0
sb $t6, $t5, 3
addi $t2, $t2, 1
slt $t7, $t2, $a1
bne $t7, $zero, putLoop
jr $ra
