main:
	li $t1, 24
	li $t0, -1
loop:	addi $t0, $t0, 1
	bgt $t0, $t1, exit
	j loop
exit:
	li $v0, 10
	syscall