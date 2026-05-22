.text

main:
addiu $t1, $zero, 24
addiu $t0, $zero, -1

loop:
addi $a0, $t0, 0
addiu $v0, $zero, 1
syscall
addi $a0, $zero, 32
addiu $v0, $zero, 11
syscall
addi $t0, $t0, 1
slt $at, $t1, $t0
bne $at, $zero, exit
j loop

exit:
addi $a0, $zero, 47
addiu $v0, $zero, 11
syscall
addiu $v0, $zero, 10
syscall
