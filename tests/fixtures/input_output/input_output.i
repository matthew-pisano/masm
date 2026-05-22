.data

promptmsg:
.byte 0x45
.byte 0x6e
.byte 0x74
.byte 0x65
.byte 0x72
.byte 0x20
.byte 0x61
.byte 0x20
.byte 0x6e
.byte 0x75
.byte 0x6d
.byte 0x62
.byte 0x65
.byte 0x72
.byte 0x20
.byte 0x74
.byte 0x6f
.byte 0x20
.byte 0x63
.byte 0x61
.byte 0x6c
.byte 0x63
.byte 0x75
.byte 0x6c
.byte 0x61
.byte 0x74
.byte 0x65
.byte 0x20
.byte 0x74
.byte 0x68
.byte 0x65
.byte 0x20
.byte 0x66
.byte 0x61
.byte 0x63
.byte 0x74
.byte 0x6f
.byte 0x72
.byte 0x69
.byte 0x61
.byte 0x6c
.byte 0x3a
.byte 0x20
.byte 0x00

outmsg:
.byte 0x54
.byte 0x68
.byte 0x65
.byte 0x20
.byte 0x66
.byte 0x61
.byte 0x63
.byte 0x74
.byte 0x6f
.byte 0x72
.byte 0x69
.byte 0x61
.byte 0x6c
.byte 0x20
.byte 0x69
.byte 0x73
.byte 0x3a
.byte 0x20
.byte 0x00


.text

main:
addiu $v0, $zero, 4
lui $at, 4097
ori $a0, $at, 0
syscall
addiu $v0, $zero, 5
syscall
addu $a0, $zero, $v0
jal fact
addu $s1, $zero, $v0
addiu $v0, $zero, 4
lui $at, 4097
ori $a0, $at, 44
syscall
addiu $v0, $zero, 1
addu $a0, $zero, $s1
syscall
addiu $v0, $zero, 10
syscall

fact:
addi $sp, $sp, -8
sw $ra, $sp, 4
sw $a0, $sp, 0
slti $t0, $a0, 1
beq $t0, $zero, recurse
addi $v0, $zero, 1
addi $sp, $sp, 8
jr $ra

recurse:
addi $a0, $a0, -1
jal fact
lw $a0, $sp, 0
lw $ra, $sp, 4
addi $sp, $sp, 8
mult $a0, $v0
mflo $v0
jr $ra
