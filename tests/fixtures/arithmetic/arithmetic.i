.data
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00

small:
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00

large:
.byte 0x00
.byte 0x01
.byte 0x00
.byte 0x00


.text

main:
lui $at, 4097
ori $t0, $at, 8
lw $t1, $t0, 0
addi $t2, $t1, 1024
sub $t3, $t2, $t1
addi $a0, $t3, 0
addiu $v0, $zero, 1
syscall
addiu $v0, $zero, 10
syscall
