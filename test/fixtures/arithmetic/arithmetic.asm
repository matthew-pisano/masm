.data

small: .word 0
large: .word 65536

.text

main: la $t0, large
lw $t1, ($t0)

addi $t2, $t1, 1024
sub $t3, $t2, $t1

addi $a0, $t3, 0

li $v0, 1
syscall

li $v0, 10
syscall
