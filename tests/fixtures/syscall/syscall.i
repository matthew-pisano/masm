.data

string:
.byte 0x68
.byte 0x65
.byte 0x6c
.byte 0x6c
.byte 0x6f
.byte 0x0a
.byte 0x00


.text

main:
addiu $v0, $zero, 4
lui $at, 4097
ori $a0, $at, 0
syscall
addiu $v0, $zero, 40
addiu $a0, $zero, 0
addiu $a1, $zero, 42
syscall
addiu $v0, $zero, 43
addiu $a0, $zero, 0
syscall
mov.s $f12, $f0
addiu $v0, $zero, 2
syscall
addiu $v0, $zero, 10
syscall
