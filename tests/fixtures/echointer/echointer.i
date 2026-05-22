.data

lastChar:
.byte 0x00
.byte 0x00
.byte 0x00
.byte 0x00


.text

main:
lui $at, 32768
ori $t7, $at, 100
jalr $t7

loop:
lui $at, 4097
ori $t0, $at, 0
lw $t1, $t0, 0
addiu $t2, $zero, 113
bne $t1, $t2, loop

exit:
addiu $v0, $zero, 10
syscall


.ktext

interp:
lui $at, 36864
ori $k0, $at, 0
sw $t0, $k0, 0
sw $t1, $k0, 4
sw $t2, $k0, 8
sw $t3, $k0, 12
lui $t0, 65535
lw $t1, $t0, 0
andi $t1, $t1, 1
beq $t1, $0, intDone
lw $t2, $t0, 4
lui $at, 4097
ori $t3, $at, 0
sw $t2, $t3, 0
lw $t1, $t0, 8
andi $t1, $t1, 1
beq $t1, $0, intDone
sw $t2, $t0, 12

intDone:
mfc0 $t0, $13
mtc0 $0, $13
lw $t0, $k0, 0
lw $t1, $k0, 4
lw $t2, $k0, 8
lw $t3, $k0, 12
eret

enable_rxint:
mfc0 $t0, $12
ori $t0, $t0, 1
mtc0 $t0, $12
jr $ra


.kdata

ihrSaveArea:
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
